/*
 * TANet.c
 * TA
 *
 * Created by Takashi Hashizume on 07/27/19.
 * Copyright 2019 Takashi Hashizume. All rights reserved.
 */

#include "TANet.h"

struct __TANet
{
#define MAX_HOSTNAME_LENGTH 128
  char hostname[MAX_HOSTNAME_LENGTH];
#define MAX_PORT_LENGTH 6 /* 0-65535 + '\0' */
  char port[MAX_PORT_LENGTH];
  FILE *in;
  FILE *out;
};
typedef struct __TANet __TANet;

#define MAX_PATH_LENGTH 128
#define MAX_HEADER_LINE_SIZE 1024

static void TANet_parseURL(const char *url, char *hostname, char *port,
                           char *path);
static int TANet_connect(const char *hostname, const char *port);
static int TANet_parseResponse(FILE *in, char *response_body);
static int TANet_listenWithPort(const char *port);
static int TANet_service(int fd, void *front_object,
                         int (*response)(void *front_object, int method,
                                         const char *path, long content_length,
                                         const char *request_body,
                                         char *response_body));
static char *TANet_parseRequest(FILE *in, int *method, char *path,
                                long *content_length, char *request_body);
static char *TANet_methodString(int method);
static int TANet_parseMethod(const char *method);
static void TANet_response(FILE *out, int method, int status_code,
                           const char *response_body);
static char *TANet_statusDescription(int status_code);
static char *TANet_guessContentType(const char *response_body);

TANet TANet_initWithURL(const char *url)
{
  struct __TANet *self = malloc(sizeof(struct __TANet));
  char hostname[MAX_HOSTNAME_LENGTH];
  char port[MAX_PORT_LENGTH];
  char path[MAX_PATH_LENGTH];
  int sockfd;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  TANet_parseURL(url, hostname, port, path);
  strcpy(self->hostname, hostname);
  strcpy(self->port, port);
  sockfd = TANet_connect(hostname, port);
  self->in = fdopen(sockfd, "rb");
  self->out = fdopen(sockfd, "wb");
  if (self->in == NULL || self->out == NULL)
    return NULL;

  return self;
}

void TANet_release(TANet self)
{
  fclose(self->in);
  fclose(self->out);

  free(self);
}

int TANet_request(TANet self, int method, const char *path,
                  const char *request_body, char *response_body)
{
  /* request */
  /* start line */
  fprintf(self->out, "%s %s HTTP/1.1\r\n", TANet_methodString(method), path);

  /* headers */
  fprintf(self->out, "Host: %s:%s\r\n", self->hostname, self->port);
  fprintf(self->out, "Accept: */*\r\n");
  fprintf(self->out, "Content-Length: %ld\r\n", strlen(request_body));
  fprintf(self->out, "Content-Type: %s\r\n", TANet_guessContentType(request_body));
  fprintf(self->out, "\r\n");

  /* body */
  fprintf(self->out, "%s", request_body);

  fflush(self->out);

  /* response */
  return TANet_parseResponse(self->in, response_body);
}

int TANet_requestWithURL(const char *url, int method, const char *request_body,
                         char *response_body)
{
  TANet net = NULL;
  int status_code;
  char hostname[MAX_HOSTNAME_LENGTH];
  char port[MAX_PORT_LENGTH];
  char path[MAX_PATH_LENGTH];

  net = TANet_initWithURL(url);
  if (net == NULL)
    return -1;

  TANet_parseURL(url, hostname, port, path);
  status_code = TANet_request(net, method, path, request_body, response_body);
  TANet_release(net);

  return status_code;
}

int TANet_startService(void *front_object, const char *port,
                       int (*response)(void *front_object, int method,
                                       const char *path, long content_length,
                                       const char *request_body,
                                       char *response_body))
{
  int status_code = TANet_OK;
  int client_sockfd, server_sockfd;
  fd_set readfds, testfds;
  int fd, max_fd;
  struct sockaddr_storage addr;
  socklen_t addrlen = sizeof(addr);
  int nread;

  server_sockfd = TANet_listenWithPort(port);
  if (server_sockfd < 0)
    return -1;

  FD_ZERO(&readfds);
  FD_SET(server_sockfd, &readfds);
  max_fd = server_sockfd;

  while (status_code != TANet_SERVICE_UNAVAILABLE)
  {
    testfds = readfds;

    if (select(max_fd + 1, &testfds, NULL, NULL, NULL) < 0)
      return -1;

    for (fd = 0; fd <= max_fd; fd++)
    {
      if (! FD_ISSET(fd, &testfds))
        continue;

      if (fd == server_sockfd)
      {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&addr,
                               &addrlen);
        if (client_sockfd < 0)
          return -1;

        FD_SET(client_sockfd, &readfds);
        if (client_sockfd > max_fd)
          max_fd = client_sockfd;
      }
      else
      {
        ioctl(fd, FIONREAD, &nread);

        if (nread == 0)
        {
          close(fd);
          FD_CLR(fd, &readfds);
          if (fd == max_fd)
            max_fd--;
        }
        else
        {
          status_code = TANet_service(fd, front_object, response);
        }
      }
    }
  }

  return 0;
}

/* private */
static void TANet_parseURL(const char *url, char *hostname, char *port,
                           char *path)
{
  char *s, *e, *p;

  s = strstr(url, "://");
  if (s == NULL)
    s = url;
  else
    s = s + strlen("://");

  e = strchr(url, ':');
  p = strchr(url, '/');

  if (e == NULL && p == NULL)
  {
    strcpy(hostname, s);
    strcpy(port, "");
    strcpy(path, "");
  }
  else if (e == NULL)
  {
    snprintf(hostname, (size_t) (p - s + 1), "%s", s);
    strcpy(port, "");
    strcpy(path, p);
  }
  else if (p == NULL)
  {
    snprintf(hostname, (size_t) (e - s + 1), "%s", s);
    strcpy(port, e + 1);
    strcpy(path, "");
  }
  else
  {
    snprintf(hostname, (size_t) (e - s + 1), "%s", s);
    snprintf(port, (size_t) (p - e), "%s", e + 1);
    strcpy(path, p);
  }
}

static int TANet_connect(const char *hostname, const char *port)
{
  struct addrinfo hints;
  struct addrinfo *res, *ai;
  int err;
  int sockfd;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((err = getaddrinfo(hostname, port, &hints, &res)) != 0)
    return err;

  for (ai = res; ai; ai = ai->ai_next)
  {
    sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0)
      continue;

    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
    {
      close(sockfd);
      continue;
    }
    freeaddrinfo(res);
    return sockfd;
  }

  freeaddrinfo(res);
  return -1;
}

static int TANet_parseResponse(FILE *in, char *response_body)
{
  int status_code = -1;
  long content_length;
  char buf[MAX_HEADER_LINE_SIZE];
  char *s, *e, *end;

  /* parse status line */
  if (fgets(buf, MAX_HEADER_LINE_SIZE, in) == NULL)
    return -1;

  s = strchr(buf, ' ');
  status_code = (int) strtol(s, &end, 10);
  if (end == s || errno == ERANGE)
    return -1;

  /* parse headers */
  while (buf[0] != '\n' && strcmp(buf, "\r\n") != 0)
  {
    if (fgets(buf, MAX_HEADER_LINE_SIZE, in) == NULL)
      return -1;

    s = strstr(buf, "Content-Length:");
    if (s != NULL)
    {
      s = s + strlen("Content-Length:");
      content_length = strtol(s, &end, 10);
      if (end == s || errno == ERANGE || content_length > TANet_MAX_BODY_LENGTH)
        return -1;
    }
  }

  /* parse body */
  if (fread(response_body, 1, content_length, in) < content_length)
    return -1;

  response_body[content_length] = '\0';

  return status_code;
}

static int TANet_listenWithPort(const char *port)
{
  struct addrinfo hints;
  struct addrinfo *res, *ai;
  int err;
  int sockfd;
  int on = 1;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((err = getaddrinfo(NULL, port, &hints, &res)) != 0)
    return err;

  for (ai = res; ai; ai = ai->ai_next)
  {
    sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0)
      continue;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
    {
      close(sockfd);
      continue;
    }
#define MAX_BACKLOG 5
    if (listen(sockfd, MAX_BACKLOG) < 0)
    {
      close(sockfd);
      continue;
    }
    freeaddrinfo(res);
    return sockfd;
  }

  return -1;
}

static int TANet_service(int fd, void *front_object,
                         int (*response)(void *front_object, int method,
                                         const char *path, long content_length,
                                         const char *request_body,
                                         char *response_body))
{
  int status_code;
  int method;
  char path[MAX_PATH_LENGTH];
  long content_length = 0;
  static char *request_body = NULL;
  static char *response_body = NULL;
  FILE *in, *out;

  if (request_body == NULL)
  {
    request_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);
    if (request_body == NULL)
      return TANet_SERVICE_UNAVAILABLE;
  }
  if (response_body == NULL)
  {
    response_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);
    if (response_body == NULL)
      return TANet_SERVICE_UNAVAILABLE;
  }

  request_body[0] = '\0';
  response_body[0] = '\0';

  in = fdopen(dup(fd), "rb");
  out = fdopen(dup(fd), "wb");

  TANet_parseRequest(in, &method, path, &content_length, request_body);
  status_code = response(front_object, method, path, content_length, request_body, response_body);
  TANet_response(out, method, status_code, response_body);

  if (status_code == TANet_SERVICE_UNAVAILABLE)
  {
    free(request_body);
    free(response_body);
    request_body = NULL;
    response_body = NULL;
  }

  fclose(in);
  fclose(out);
  return status_code;
}

static char *TANet_parseRequest(FILE *in, int *method, char *path,
                                long *content_length, char *request_body)
{
  char buf[MAX_HEADER_LINE_SIZE];
  char *s, *e, *end;

  /* parse start line */
  if (fgets(buf, MAX_HEADER_LINE_SIZE, in) == NULL)
    return NULL;

  s = buf;
  e = strchr(s, ' ');
  *e = '\0';
  *method = TANet_parseMethod(s);

  s = e + 1;
  e = strchr(s, ' ');
  *e = '\0';
  strcpy(path, s);

  /* parse headers */
  while (buf[0] != '\n' && strcmp(buf, "\r\n") != 0)
  {
    if (fgets(buf, MAX_HEADER_LINE_SIZE, in) == NULL)
      return NULL;

    s = strstr(buf, "Content-Length:");
    if (s != NULL)
    {
      s = s + strlen("Content-Length:");
      *content_length = strtol(s, &end, 10);
      if (end == s || errno == ERANGE || *content_length > TANet_MAX_BODY_LENGTH)
        return NULL;
    }
  }

  /* parse body */
  if (fread(request_body, 1, *content_length, in) < *content_length)
    return NULL;

  request_body[*content_length] = '\0';

  return request_body;
}

static char *TANet_methodString(int method)
{
  switch (method)
  {
  case TANet_GET:
    return "GET";
  case TANet_HEAD:
    return "HEAD";
  case TANet_POST:
    return "POST";
  case TANet_PUT:
    return "PUT";
  case TANet_PATCH:
    return "PATCH";
  default:
    return "";
  }
}

static int TANet_parseMethod(const char *method)
{
  if (strcmp(method, "GET") == 0)
    return TANet_GET;
  else if (strcmp(method, "HEAD") == 0)
    return TANet_HEAD;
  else if (strcmp(method, "POST") == 0)
    return TANet_POST;
  else if (strcmp(method, "PUT") == 0)
    return TANet_PUT;
  else if (strcmp(method, "PATCH") == 0)
    return TANet_PATCH;
  else
    return -1;
}

static void TANet_response(FILE *out, int method, int status_code,
                           const char *response_body)
{
  /* status line */
  fprintf(out, "HTTP/1.1 %d %s\r\n", status_code,
          TANet_statusDescription(status_code));

  /* headers */
  fprintf(out, "Content-Length: %ld\r\n", strlen(response_body));
  fprintf(out, "Content-Type: %s\r\n", TANet_guessContentType(response_body));
  fprintf(out, "\r\n");

  /* body */
  if (method != TANet_HEAD)
    fprintf(out, "%s", response_body);

  fflush(out);
}

static char *TANet_statusDescription(int status_code)
{
  switch (status_code)
  {
  case TANet_OK:
    return "OK";
  case TANet_BAD_REQUEST:
    return "Bad Request";
  case TANet_NOT_FOUND:
    return "Not Found";
  case TANet_METHOD_NOT_ALLOWED:
    return "Method Not Allowed";
  case TANet_NOT_IMPLEMENTED:
    return "Not Implemented";
  case TANet_SERVICE_UNAVAILABLE:
    return "Service Unavailable";
  default:
    return "";
  }
}

static char *TANet_guessContentType(const char *response_body)
{
  if (response_body[0] == '<')
    return "text/html";
  else if (response_body[0] == '{')
    return "application/json";
  else
    return "text/plain";
}
