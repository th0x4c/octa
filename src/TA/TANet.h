/*
 * TANet.h
 * TA
 *
 * Created by Takashi Hashizume on 07/27/19.
 * Copyright 2019 Takashi Hashizume. All rights reserved.
 */

#ifndef _TANET_H_
#define _TANET_H_

#include <stdlib.h>     /* malloc free strtol */
#include <string.h>     /* memset strchr strcpy strcmp strstr strlen */
#include <stdio.h>      /* fdopen fclose snprintf fgets fread fprintf fflush */
#include <sys/types.h>  /* select FD_ZERO FD_SET FD_ISSET FD_CLR getaddrinfo  connect socket listen freeaddrinfo */
#include <sys/time.h>   /* select FD_ZERO FD_SET FD_ISSET FD_CLR */
#include <unistd.h>     /* select FD_ZERO FD_SET FD_ISSET FD_CLR close dup */
#include <sys/socket.h> /* accept getaddrinfo connect socket bind listen freeaddrinfo */
#include <sys/ioctl.h>  /* ioctl */
#include <netdb.h>      /* getaddrinfo freeaddrinfo */
#include <errno.h>      /* errno */
#include "TABool.h"

typedef struct __TANet *TANet;

/* method */
#define TANet_GET   1
#define TANet_HEAD  2
#define TANet_POST  3
#define TANet_PUT   4
#define TANet_PATCH 5

/* status code */
#define TANet_OK                  200
#define TANet_BAD_REQUEST         400
#define TANet_NOT_FOUND           404
#define TANet_METHOD_NOT_ALLOWED  405
#define TANet_NOT_IMPLEMENTED     501
#define TANet_SERVICE_UNAVAILABLE 503

#define TANet_MAX_BODY_LENGTH 10485760 /* 10M */

TANet TANet_initWithURL(const char *url);
void TANet_release(TANet self);
int TANet_request(TANet self, int method, const char *path,
                  const char *request_body, char *response_body);
int TANet_requestWithURL(const char *url, int method, const char *request_body,
                         char *response_body);
int TANet_startService(void *front_object, const char *port,
                       int (*response)(void *front_object, int method,
                                       const char *path, long content_length,
                                       const char *request_body,
                                       char *response_body));

#endif /* _TANET_H_ */
