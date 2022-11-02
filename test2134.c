#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
  #define THE_OS_WINDOWS
  #define THE_EOL "\r\n"
  #define THE_PATH_SEP "\\"
#else
  #if defined(__APPLE__)
    #define THE_OS_MACOS
  #elif defined(__linux__)
    #define THE_OS_LINUX
  #endif
  #define THE_EOL "\n"
  #define THE_PATH_SEP "/"
#endif

#include <ctype.h>
#include <openssl/ssl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef THE_OS_WINDOWS
  #include <winsock2.h>
  #include <windows.h>
  #include <ws2tcpip.h>
#endif
#ifndef THE_OS_WINDOWS
  #include <netdb.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
#endif

struct buffer {
  unsigned char *d;
  size_t l;
};
struct request {
  #ifdef THE_OS_WINDOWS
    SOCKET fd;
  #else
    int fd;
  #endif
  SSL_CTX *ctx;
  SSL *ssl;
};
struct str {
  char *d;
  size_t l;
};

struct request_Header;
struct request_Request;
struct __THE_1_array_request_Header;
struct url_URL;

struct request_Header {
  const struct str __THE_0_name;
  const struct str __THE_0_value;
};
struct request_Request {
};
struct __THE_1_array_request_Header {
  struct request_Header **d;
  size_t l;
};
struct url_URL {
  const struct str __THE_0_origin;
  const struct str __THE_0_protocol;
  const struct str __THE_0_host;
  const struct str __THE_0_hostname;
  const struct str __THE_0_port;
  const struct str __THE_0_path;
  const struct str __THE_0_pathname;
  const struct str __THE_0_search;
  const struct str __THE_0_hash;
};

void *alloc (size_t);
void *re_alloc (void *, size_t);
void buffer_free (struct buffer);
struct str str_alloc (const char *);
char *str_cstr (struct str);
void str_free (struct str);
void request_close (struct request_Request **);
struct request_Request *request_open (struct str, struct str, struct buffer, struct __THE_1_array_request_Header);
char *request_stringifyHeaders (struct __THE_1_array_request_Header, struct url_URL *, struct buffer);
struct url_URL *url_parse (struct str);
void request_Header_free (struct request_Header *);
void request_Request_free (struct request_Request *);
struct request_Request *request_Request_realloc (struct request_Request *, struct request_Request *);
void __THE_1_array_request_Header_free (struct __THE_1_array_request_Header);
struct url_URL *url_URL_alloc (struct str, struct str, struct str, struct str, struct str, struct str, struct str, struct str, struct str);
void url_URL_free (struct url_URL *);

void *alloc (size_t l) {
  void *r = malloc(l);
  if (r == NULL) {
    fprintf(stderr, "Error: failed to allocate %zu bytes" THE_EOL, l);
    exit(EXIT_FAILURE);
  }
  return r;
}
void *re_alloc (void *d, size_t l) {
  void *r = realloc(d, l);
  if (r == NULL) {
    fprintf(stderr, "Error: failed to reallocate %zu bytes" THE_EOL, l);
    exit(EXIT_FAILURE);
  }
  return r;
}
void buffer_free (struct buffer o) {
  free(o.d);
}
struct str str_alloc (const char *r) {
  size_t l = strlen(r);
  char *d = alloc(l);
  memcpy(d, r, l);
  return (struct str) {d, l};
}
char *str_cstr (const struct str s) {
  char *d = alloc(s.l + 1);
  memcpy(d, s.d, s.l);
  d[s.l] = '\0';
  return d;
}
void str_free (struct str s) {
  free(s.d);
}
void request_close (struct request_Request **r) {
  struct request *req = (void *) *r;
  if (req->ssl != NULL) {
    SSL_CTX_free(req->ctx);
    SSL_free(req->ssl);
  } else if (req->fd != 0) {
    #ifdef THE_OS_WINDOWS
      closesocket(req->fd);
    #else
      close(req->fd);
    #endif
  }
  req->fd = 0;
  req->ctx = NULL;
  req->ssl = NULL;
}
struct request_Request *request_open (struct str method, struct str u, struct buffer data, struct __THE_1_array_request_Header headers) {
  struct url_URL *url = url_parse(u);
  if (
    !(url->__THE_0_protocol.l == 5 && memcmp(url->__THE_0_protocol.d, "http:", 5) == 0) &&
    !(url->__THE_0_protocol.l == 6 && memcmp(url->__THE_0_protocol.d, "https:", 6) == 0)
  ) {
    char *protocol = str_cstr(url->__THE_0_protocol);
    fprintf(stderr, "Error: can't perform request with protocol `%s`" THE_EOL, protocol);
    exit(EXIT_FAILURE);
  } else if (url->__THE_0_port.l > 6) {
    char *port = str_cstr(url->__THE_0_port);
    fprintf(stderr, "Error: invalid port `%s`" THE_EOL, port);
    exit(EXIT_FAILURE);
  }
  char port[10];
  strcpy(port, url->__THE_0_protocol.l == 6 ? "443" : "80");
  if (url->__THE_0_port.l != 0) {
    memcpy(port, url->__THE_0_port.d, url->__THE_0_port.l);
    unsigned long p = strtoul(port, NULL, 10);
    if (p > 65535) {
      fprintf(stderr, "Error: invalid port `%s`" THE_EOL, port);
      exit(EXIT_FAILURE);
    }
  }
  char *hostname = str_cstr(url->__THE_0_hostname);
  struct addrinfo *addr = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  if (getaddrinfo(hostname, port, &hints, &addr) != 0) {
    fprintf(stderr, "Error: failed to resolve hostname address" THE_EOL);
    exit(EXIT_FAILURE);
  }
  free(hostname);
  struct request *req = alloc(sizeof(struct request));
  req->fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  req->ctx = NULL;
  req->ssl = NULL;
  #ifdef THE_OS_WINDOWS
    bool socket_res = req->fd != INVALID_SOCKET;
  #else
    bool socket_res = req->fd != -1;
  #endif
  if (!socket_res) {
    fprintf(stderr, "Error: failed to create socket" THE_EOL);
    exit(EXIT_FAILURE);
  }
  #ifdef THE_OS_WINDOWS
    bool connect_res = connect(req->fd, addr->ai_addr, (int) addr->ai_addrlen) != SOCKET_ERROR;
  #else
    bool connect_res = connect(req->fd, addr->ai_addr, addr->ai_addrlen) != -1;
  #endif
  if (!connect_res) {
    char *origin = str_cstr(url->__THE_0_origin);
    fprintf(stderr, "Error: failed to connect to `%s`" THE_EOL, origin);
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(addr);
  if (strcmp(port, "443") == 0) {
    SSL_library_init();
    req->ctx = SSL_CTX_new(TLS_client_method());
    if (req->ctx == NULL) {
      fprintf(stderr, "Error: failed to create SSL context" THE_EOL);
      exit(EXIT_FAILURE);
    }
    req->ssl = SSL_new(req->ctx);
    SSL_set_fd(req->ssl, req->fd);
    if (SSL_connect(req->ssl) != 1) {
      fprintf(stderr, "Error: failed to connect to socket with SSL" THE_EOL);
      exit(EXIT_FAILURE);
    }
  }
  char *req_headers = request_stringifyHeaders(headers, url, data);
  __THE_1_array_request_Header_free(headers);
  char *req_method = str_cstr(method);
  str_free(method);
  char *req_path = str_cstr(url->__THE_0_path);
  char *fmt = "%s %s HTTP/1.1\r\n%s\r\n";
  size_t req_len = snprintf(NULL, 0, fmt, req_method, req_path, req_headers);
  char *request = alloc(req_len + (data.l == 0 ? 0 : data.l + 2) + 1);
  sprintf(request, fmt, req_method, req_path, req_headers);
  free(req_path);
  free(req_method);
  free(req_headers);
  if (data.l != 0) {
    memcpy(&request[req_len], data.d, data.l);
    req_len += data.l;
    memcpy(&request[req_len], "\r\n", 3);
    req_len += 2;
  }
  buffer_free(data);
  #ifdef THE_OS_WINDOWS
    SSIZE_T y = 0, z = 0;
  #else
    ssize_t y = 0, z = 0;
  #endif
  while (y < req_len) {
    z = req->ssl == NULL
      ? send(req->fd, &request[y], req_len - y, 0)
      : SSL_write(req->ssl, &request[y], (int) (req_len - y));
    if (z == -1) {
      fprintf(stderr, "Error: failed to write to socket" THE_EOL);
      exit(EXIT_FAILURE);
    }
    y += z;
  }
  free(request);
  if (req->ssl == NULL) {
    #ifdef THE_OS_WINDOWS
      shutdown(req->fd, SD_SEND);
    #else
      shutdown(req->fd, 1);
    #endif
  } else {
    SSL_shutdown(req->ssl);
  }
  url_URL_free(url);
  return (struct request_Request *) req;
}
char *request_stringifyHeaders (struct __THE_1_array_request_Header headers, struct url_URL *url, struct buffer data) {
  bool has_content_length = false;
  bool has_host = false;
  char *d = NULL;
  size_t l = 0;
  for (size_t i = 0; i < headers.l; i++) {
    struct request_Header *h = headers.d[i];
    char *name = str_cstr(h->__THE_0_name);
    for (size_t j = 0; j < h->__THE_0_name.l; j++) name[j] = (char) tolower(name[j]);
    if (strcmp(name, "content-length") == 0) has_content_length = true;
    else if (strcmp(name, "host") == 0) has_host = true;
    free(name);
    d = re_alloc(d, l + h->__THE_0_name.l + 2 + h->__THE_0_value.l + 3);
    memcpy(&d[l], h->__THE_0_name.d, h->__THE_0_name.l);
    memcpy(&d[l + h->__THE_0_name.l], ": ", 2);
    memcpy(&d[l + h->__THE_0_name.l + 2], h->__THE_0_value.d, h->__THE_0_value.l);
    memcpy(&d[l + h->__THE_0_name.l + 2 + h->__THE_0_value.l], "\r\n", 3);
    l += h->__THE_0_name.l + 2 + h->__THE_0_value.l + 2;
  }
  if (!has_host) {
    char *h = str_cstr(url->__THE_0_hostname);
    size_t z = snprintf(NULL, 0, "Host: %s\r\n", h);
    d = re_alloc(d, l + z + 1);
    sprintf(&d[l], "Host: %s\r\n", h);
    l += z;
    free(h);
  }
  if (!has_content_length) {
    size_t z = snprintf(NULL, 0, "Content-Length: %zu\r\n", data.l);
    d = re_alloc(d, l + z + 1);
    sprintf(&d[l], "Content-Length: %zu\r\n", data.l);
    l += z;
  }
  return d;
}
struct url_URL *url_parse (struct str s) {
  if (s.l == 0) {
    fprintf(stderr, "Error: invalid URL" THE_EOL);
    exit(EXIT_FAILURE);
  }
  size_t i = 0;
  for (;; i++) {
    char ch = s.d[i];
    if (ch == ':' && i != 0) {
      i++;
      break;
    } else if (!isalnum(ch) && ch != '.' && ch != '-' && ch != '+') {
      fprintf(stderr, "Error: invalid URL protocol" THE_EOL);
      exit(EXIT_FAILURE);
    } else if (i == s.l - 1) {
      fprintf(stderr, "Error: invalid URL" THE_EOL);
      exit(EXIT_FAILURE);
    }
  }
  struct str protocol;
  protocol.l = i;
  protocol.d = alloc(protocol.l);
  memcpy(protocol.d, s.d, protocol.l);
  while (i < s.l && s.d[i] == '/') i++;
  if (i == s.l) {
    str_free(s);
    return url_URL_alloc(str_alloc(""), protocol, str_alloc(""), str_alloc(""), str_alloc(""), str_alloc(""), str_alloc(""), str_alloc(""), str_alloc(""));
  }
  size_t protocol_end = i;
  if ((protocol_end - protocol.l) < 2) {
    i = protocol.l;
    protocol_end = i;
  }
  size_t hostname_start = protocol.l == protocol_end ? 0 : i;
  size_t port_start = 0;
  size_t pathname_start = protocol.l == protocol_end ? i : 0;
  size_t search_start = 0;
  size_t hash_start = 0;
  for (;; i++) {
    char ch = s.d[i];
    if (ch == '@' && hostname_start != 0 && pathname_start == 0) {
      fprintf(stderr, "Error: URL auth is not supported" THE_EOL);
      exit(EXIT_FAILURE);
    } else if (ch == ':' && port_start != 0 && (pathname_start == 0 || search_start == 0 || hash_start == 0)) {
      fprintf(stderr, "Error: invalid URL port" THE_EOL);
      exit(EXIT_FAILURE);
    }
    if (ch == ':' && hostname_start != 0 && pathname_start == 0) port_start = i;
    else if (ch == '/' && pathname_start == 0) pathname_start = i;
    else if (ch == '?' && search_start == 0) search_start = i;
    else if (ch == '#' && hash_start == 0) hash_start = i;
    else if (i == s.l - 1) break;
  }
  struct str hostname = str_alloc("");
  size_t hostname_end = port_start != 0 ? port_start : pathname_start != 0 ? pathname_start : search_start != 0 ? search_start : hash_start != 0 ? hash_start : s.l;
  if (hostname_start != 0 && hostname_start == hostname_end) {
    fprintf(stderr, "Error: invalid URL hostname" THE_EOL);
    exit(EXIT_FAILURE);
  } else if (hostname_start != 0 && hostname_start != hostname_end) {
    hostname.l = hostname_end - hostname_start;
    hostname.d = re_alloc(hostname.d, hostname.l);
    memcpy(hostname.d, &s.d[hostname_start], hostname.l);
  }
  struct str port = str_alloc("");
  size_t port_end = pathname_start != 0 ? pathname_start : search_start != 0 ? search_start : hash_start != 0 ? hash_start : s.l;
  if (port_start != 0 && port_start + 1 != port_end) {
    port.l = port_end - port_start - 1;
    port.d = re_alloc(port.d, port.l);
    memcpy(port.d, &s.d[port_start + 1], port.l);
  }
  struct str host = str_alloc("");
  if (hostname.l != 0) {
    host.l = hostname.l + (port.l == 0 ? 0 : port.l + 1);
    host.d = re_alloc(host.d, host.l);
    memcpy(host.d, hostname.d, hostname.l);
    if (port.l != 0) {
      memcpy(&host.d[hostname.l], ":", 1);
      memcpy(&host.d[hostname.l + 1], port.d, port.l);
    }
  }
  struct str origin = str_alloc("");
  if (memcmp(protocol.d, "ftp:", 4) == 0 || memcmp(protocol.d, "http:", 5) == 0 || memcmp(protocol.d, "https:", 6) == 0 || memcmp(protocol.d, "ws:", 3) == 0 || memcmp(protocol.d, "wss:", 4) == 0) {
    if (host.l == 0) {
      fprintf(stderr, "Error: URL origin is not present" THE_EOL);
      exit(EXIT_FAILURE);
    }
    origin.l = protocol.l + 2 + host.l;
    origin.d = re_alloc(origin.d, origin.l);
    memcpy(origin.d, protocol.d, protocol.l);
    memcpy(&origin.d[protocol.l], "//", 2);
    memcpy(&origin.d[protocol.l + 2], host.d, host.l);
  }
  struct str pathname = str_alloc("");
  size_t pathname_end = search_start != 0 ? search_start : hash_start != 0 ? hash_start : s.l;
  if (pathname_start != 0 && pathname_start != pathname_end) {
    pathname.l = pathname_end - pathname_start;
    pathname.d = re_alloc(pathname.d, pathname.l);
    memcpy(pathname.d, &s.d[pathname_start], pathname.l);
  } else if (memcmp(protocol.d, "ftp:", 4) == 0 || memcmp(protocol.d, "http:", 5) == 0 || memcmp(protocol.d, "https:", 6) == 0 || memcmp(protocol.d, "ws:", 3) == 0 || memcmp(protocol.d, "wss:", 4) == 0) {
    pathname.l = 1;
    pathname.d = re_alloc(pathname.d, pathname.l);
    memcpy(pathname.d, "/", pathname.l);
  }
  struct str search = str_alloc("");
  size_t search_end = hash_start != 0 ? hash_start : s.l;
  if (search_start != 0 && search_start != search_end) {
    search.l = search_end - search_start;
    search.d = re_alloc(search.d, search.l);
    memcpy(search.d, &s.d[search_start], search.l);
  }
  struct str path = str_alloc("");
  if (pathname.l != 0 || search.l != 0) {
    path.l = pathname.l + search.l;
    path.d = re_alloc(path.d, path.l);
    if (pathname.l != 0) {
      memcpy(path.d, pathname.d, pathname.l);
      if (search.l != 0) memcpy(&path.d[pathname.l], search.d, search.l);
    } else if (search.l != 0) {
      memcpy(path.d, search.d, search.l);
    }
  }
  struct str hash = str_alloc("");
  if (hash_start != 0) {
    hash.l = s.l - hash_start;
    hash.d = re_alloc(hash.d, hash.l);
    memcpy(hash.d, &s.d[hash_start], hash.l);
  }
  str_free(s);
  return url_URL_alloc(origin, protocol, host, hostname, port, path, pathname, search, hash);
}
void request_Header_free (struct request_Header *o) {
  str_free((struct str) o->__THE_0_name);
  str_free((struct str) o->__THE_0_value);
  free(o);
}
void request_Request_free (struct request_Request *o) {
  request_close(&o);
  free(o);
}
struct request_Request *request_Request_realloc (struct request_Request *o1, struct request_Request *o2) {
  request_Request_free((struct request_Request *) o1);
  return o2;
}
void __THE_1_array_request_Header_free (struct __THE_1_array_request_Header n) {
  for (size_t i = 0; i < n.l; i++) request_Header_free((struct request_Header *) n.d[i]);
  free(n.d);
}
struct url_URL *url_URL_alloc (struct str __THE_0_origin, struct str __THE_0_protocol, struct str __THE_0_host, struct str __THE_0_hostname, struct str __THE_0_port, struct str __THE_0_path, struct str __THE_0_pathname, struct str __THE_0_search, struct str __THE_0_hash) {
  struct url_URL *r = alloc(sizeof(struct url_URL));
  struct url_URL s = {__THE_0_origin, __THE_0_protocol, __THE_0_host, __THE_0_hostname, __THE_0_port, __THE_0_path, __THE_0_pathname, __THE_0_search, __THE_0_hash};
  memcpy(r, &s, sizeof(struct url_URL));
  return r;
}
void url_URL_free (struct url_URL *o) {
  str_free((struct str) o->__THE_0_origin);
  str_free((struct str) o->__THE_0_protocol);
  str_free((struct str) o->__THE_0_host);
  str_free((struct str) o->__THE_0_hostname);
  str_free((struct str) o->__THE_0_port);
  str_free((struct str) o->__THE_0_path);
  str_free((struct str) o->__THE_0_pathname);
  str_free((struct str) o->__THE_0_search);
  str_free((struct str) o->__THE_0_hash);
  free(o);
}

int main () {
  struct request_Request *__THE_0_req_0 = request_open(str_alloc("GET"), str_alloc("http://example.com"), (struct buffer) {NULL, 0}, (struct __THE_1_array_request_Header) {NULL, 0});
  request_close(&__THE_0_req_0);
  __THE_0_req_0 = request_Request_realloc(__THE_0_req_0, request_open(str_alloc("GET"), str_alloc("https://example.com"), (struct buffer) {NULL, 0}, (struct __THE_1_array_request_Header) {NULL, 0}));
  request_close(&__THE_0_req_0);
  request_Request_free((struct request_Request *) __THE_0_req_0);
}
