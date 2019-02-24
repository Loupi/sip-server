#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <plibsys.h>
#include <uv.h>


#define default_port 8080
#define default_timeout 10000
#define default_queue_len 128
#define default_bind_address "0.0.0.0"

int port = default_port;
int timeout = default_timeout;
int queue_len = default_queue_len;
const char* bind_address;


static int verbose_flag = 1;

uv_loop_t *loop;
struct sockaddr_in addr;
PHashTable* sipTable = NULL;


typedef struct {
  uv_timer_t timer;
  uv_tcp_t *client;
} SocketData;


void log_info(char *fmt, ...);
void log_error(char *fmt, ...);
void print_usage();
unsigned int larson_hash(const char* s, int len);
int read_sip_table(const char* file);
void free_sip_table();
int start_server();
void on_sigint_received(uv_signal_t *handle, int signum);
void on_uv_walk(uv_handle_t* handle, void* arg);
void on_connect(uv_stream_t *server, int status);
void on_timeout(uv_timer_t* handle);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void socket_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void socket_write(uv_write_t *req, int status);


void log_info(char *fmt, ...) {
  if (verbose_flag) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
}


void log_error(char *fmt, ...) {
  if (verbose_flag) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  }
}


void print_usage() {
  printf("Usage: sip-server [v] [-b bind_address] [-p port] [-t timeout(ms)] [-q queuelen] -i inputfile\n");
  printf("ie: sip-server -b 0.0.0.0 -p 8080 -t 10000 -q 128 -i ../regs\n");
}


int main(int argc, char** argv) {
  char* file = NULL;
  int option = 0;
  bind_address =  default_bind_address;

  while ((option = getopt(argc, argv,"vb:p:t:q:i:")) != -1) {
    switch (option) {
      case 's' : verbose_flag = 0;
        break;
      case 'b' : bind_address = optarg;
        break;
      case 'p' : port = atoi(optarg); 
        break;
      case 't' : timeout = atoi(optarg);
        break;
      case 'q' : queue_len = atoi(optarg);
        break;
      case 'i' : file = optarg;
        break;
      default: print_usage(); 
        exit(EXIT_FAILURE);
    }
  }

  if (port == -1 || timeout ==-1 || queue_len == -1 || file == NULL) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  p_libsys_init();
  
  if (read_sip_table(file)) {
    start_server();
  }

  log_info("Shutting down...\n");

  free_sip_table();
  p_libsys_shutdown();
  return 0;
}

#ifdef WIN32
char* getline(char** dst, size_t len, FILE* fp) {
  char buffer[4096];
  memset(buffer, sizeof(buffer), 0);
  
  if (fgets(buffer, 4096, fp)) {
    size_t len = strlen(buffer);
    if (len) { 
      char* data = malloc(len + 1);
      data[len] = 0;
      memcpy(data, buffer, len);
      *dst = data;
      return data;
    }
  }
  return NULL;
}
#endif


int read_sip_table(const char* file) {
  const char* aorHeader = "{\"addressOfRecord\":\"";
  const char aorHeaderEnd = '\"';
  const size_t aorHeaderLen = strlen(aorHeader);
  
  /* Open SIP registration dump file */
  FILE* fp = fopen(file, "r+");
  if (!fp) {
    log_error("Error: Cannot open registry file.\n");
    return 0;
  }

  /* Create new hashtable */
  sipTable = p_hash_table_new();
  if (!sipTable) {
    log_error("Error: Cannot allocate hashtable.\n");
    return 0;
  }

  while (!feof(fp)) {
    char* line = NULL;
    size_t size;

    /* Read all lines and process only the ones with a matching header. */
    if (getline(&line, &size, fp)) {
      if (strlen(line) > aorHeaderLen &&
          !strncmp(aorHeader, line, aorHeaderLen)) {

        /* Determine AOR position in string */
        const char* aorStartOffset = line + aorHeaderLen;
        const char* aorEndOffSet = strchr(aorStartOffset, aorHeaderEnd);
        
        if (aorEndOffSet) {
          /* Compute hash of AOR and add record to hashtable */
          const size_t aorLength = aorEndOffSet - aorStartOffset;
          unsigned int crc = larson_hash(aorStartOffset, aorLength);
          p_hash_table_insert(sipTable, (void*)(unsigned long)crc, line);
          /* This is a valid entry, mark as null to not free it now,
           but rather when cleaning the sipTable at the end of the program. */
          line = NULL;
        }
      }

      /* Free incorrect lines */
      if (line) {
        free(line);
      }
    }
  }

  
  fclose(fp);

  return 1;
}


/* Paul Larson's Hash
 https://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map */
unsigned int larson_hash(const char* s, int len) {
  unsigned int hash = 0;
  for (int i = 0; i < len; i++) {
    hash = hash * 101  +  *(s+i);
  }
  return hash;
}


static const char *addr_and_port(uv_tcp_t *handle)
{
  struct sockaddr_in name;
  int namelen = sizeof(name);
  if (uv_tcp_getpeername(handle, (struct sockaddr*) &name, &namelen)) {
    log_error("uv_tcp_getpeername");
  }

  char addr[16];
  static char buf[32];
  uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));
  snprintf(buf, sizeof(buf), "%s:%d", addr, ntohs(name.sin_port));

  return buf;
}


int start_server() {
  /* Setup the libuv loop */
  loop = uv_default_loop();

  /* Handle SIGIN to perform gracefull shutdown and free resources */
  uv_signal_t sigint;
  uv_signal_init(uv_default_loop(), &sigint);
  uv_signal_start(&sigint, on_sigint_received, SIGINT);

  /* Inititializeserver socket */
  uv_tcp_t server;
  uv_tcp_init(loop, &server);
  uv_ip4_addr(bind_address, port, &addr);
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);

  /* Listen for incoming connections */
  int r = uv_listen((uv_stream_t*)&server, queue_len, on_connect);
  if (r) {
    log_error("Listen error %s\n", uv_strerror(r));
    return 1;
  }

  log_info("Server started %s:%d\n", bind_address, port);
  uv_run(loop, UV_RUN_DEFAULT);
  return uv_loop_close(uv_default_loop());
}


void on_sigint_received(uv_signal_t *handle, int signum) {
  int result = uv_loop_close(handle->loop);
  if (result == UV_EBUSY) {
      uv_walk(handle->loop, on_uv_walk, NULL);
  }
}


void on_uv_walk(uv_handle_t* handle, void* arg) {
  uv_close(handle, NULL);
}


void on_connect(uv_stream_t *server, int status) {

  if (status < 0) {
    log_error("Connection error %s\n", uv_strerror(status));
    return;
  }

  /* Allocate and accept client socket */
  uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);

  if (uv_accept(server, (uv_stream_t*) client) == 0) {
    log_info("%s connected.\n", addr_and_port(client));

    /* Initialise custom data to manage timeouts for the new client socket */
    SocketData* data = malloc(sizeof(SocketData));
    memset(data, 0, sizeof(SocketData));
    data->client = client;
    ((uv_handle_t*) client)->data = data;
    ((uv_handle_t*) &(data->timer))->data = data;

    /* Setup read timeout and wait for data. */
    uv_timer_init(loop, &(data->timer));
    uv_timer_start(&(data->timer), on_timeout, timeout, 0);
    uv_read_start((uv_stream_t*)client, alloc_buffer, socket_read);
  } else {
    log_error("Connection failed.\n");
    uv_close((uv_handle_t*) client, NULL);
  }
}


void on_timeout(uv_timer_t* handle) {
  SocketData* data = ((uv_handle_t*)handle)->data;
  log_info("%s timeout.\n", addr_and_port(data->client));

  /* Close socket and free custom data. */
  
  uv_close((uv_handle_t*) data->client, NULL);
  free(data);
}


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}


void socket_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  const char* separator = "\n";

  /* Get custom data pointer and stop read timeout while processing. */
  SocketData* data = ((uv_handle_t*) client)->data; 
  uv_timer_stop(&(data->timer));

  if (nread == UV_EOF) {
    /* Socket is closed on EOF, free custom data. */
    log_info("%s closed.\n", addr_and_port((uv_tcp_t *)client));
    free(data);

  } else if (nread < 0) {
    /* Socket in error, close it and free custom data. */
    log_error("%s Read error %s.\n", addr_and_port((uv_tcp_t *)client), uv_err_name(nread));
    uv_close((uv_handle_t*) client, NULL);
    free(data);

  } else if (nread > 0) {
    /* Got some data, tokenize and process it. */
    buf->base[nread] = 0;
    char* token = strtok(buf->base, separator);
    int nb = 0;
    char* base = buf->base;
    
    while (token != NULL) {
      char* toSend;
      int datalen;

      /* Lookup key in sip table. */
      unsigned int crc = larson_hash(token, strlen(token));
      char* value = p_hash_table_lookup(sipTable, (void*)(unsigned long)crc);
      
      if (value == (char*)-1) {
        log_info("%s requested %s Not found!\n", addr_and_port((uv_tcp_t *)client), token);
        toSend = (char*)separator;
        datalen = 1;
      } else {
        log_info("%s requested %s Found!\n", addr_and_port((uv_tcp_t *)client), token);
        toSend = value;
        datalen = strlen(value);
      }
      
      /* Write response */
      uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
      uv_buf_t wrbuf = uv_buf_init(toSend, datalen);
      uv_write(req, client, &wrbuf, 1, socket_write);

      token = strtok(NULL, separator);
    }

    /* Restart read timeout and wait for data. */
    uv_timer_start(&(data->timer), on_timeout, timeout, 0);
    uv_read_start((uv_stream_t*)client, alloc_buffer, socket_read);
  }

  if (buf->base) {
    free(buf->base);
  }
}


void socket_write(uv_write_t *req, int status) {
  if (status) {
    log_error("Write error %s\n", uv_strerror(status));
  }
  free(req);
}


void free_sip_table() {
  if (!sipTable)
    return;

  /* Free the values of the hashtable.
   They were allocated by getline in read_sip_table. */
  PList* values = p_hash_table_values(sipTable);
  if (values) {
    p_list_foreach (values, (PFunc) free, NULL);
    p_list_free(values);
  }

  p_hash_table_free(sipTable);
}