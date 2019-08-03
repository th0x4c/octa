/*
 * test_TANet.c
 * TA
 *
 * Created by Takashi Hashizume on 07/27/19.
 * Copyright 2019 Takashi Hashizume. All rights reserved.
 */

#include <TA/TANet.h>
#include "../munit.h"
#include <string.h>   /* strcmp strcpy */
#include <unistd.h>   /* fork */
#include <stdio.h>    /* fprintf */
#include <stdlib.h>   /* malloc exit free */
#include <sys/wait.h> /* wait */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

#define myhtml                        \
  "<!DOCTYPE html>\n"                 \
  "<html>\n"                          \
  "  <head>\n"                        \
  "    <meta charset=\"utf-8\">\n"    \
  "    <title>hello, world</title>\n" \
  "  </head>\n"                       \
  "  <body>\n"                        \
  "    <p>hello, world</p>\n"         \
  "  </body>\n"                       \
  "</html>\n"

#define mynotfoundhtml                \
  "<!DOCTYPE html>\n"                 \
  "<html>\n"                          \
  "  <head>\n"                        \
  "    <meta charset=\"utf-8\">\n"    \
  "    <title>Not Found</title>\n"    \
  "  </head>\n"                       \
  "  <body>\n"                        \
  "    <p>File Not Found</p>\n"       \
  "  </body>\n"                       \
  "</html>\n"

static int myresponse(void *front_object, int method, const char *path,
                      long content_length, const char *request_body,
                      char *response_body)
{
  char *myresponsebody = myhtml;
  char *myerrorrbody = mynotfoundhtml;

  switch (method)
  {
  case TANet_GET:
  case TANet_HEAD:
    if (strcmp(path, "/") == 0)
    {
      strcpy(response_body, myresponsebody);
      return TANet_OK;
    }
    else
    {
      strcpy(response_body, myerrorrbody);
      return TANet_NOT_FOUND;
    }
    break;
  case TANet_POST:
    if (strcmp(path, "/hello") == 0)
    {
      if (strcmp(request_body, "world") == 0)
      {
        strcpy(response_body, "hello, world");
        return TANet_OK;
      }
    }
    else if (strcmp(path, "/stop") == 0)
    {
      strcpy(response_body, "");
      return TANet_SERVICE_UNAVAILABLE;
    }
    else
    {
      strcpy(response_body, myerrorrbody);
      return TANet_NOT_FOUND;
    }
    break;
  case TANet_PUT:
  case TANet_PATCH:
    return TANet_METHOD_NOT_ALLOWED;
    break;
  default:
    return TANet_NOT_IMPLEMENTED;
    break;
  }
}

static void test_TANet_startService()
{
  pid_t pid;
  int status_code;
  char *response_body;

  response_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);
  if (response_body == NULL)
    exit(1);

  switch (pid = fork())
  {
  case -1:
    fprintf(stderr, "fork failed\n");
    exit(1);
  case 0:
    /* child */
    TANet_startService(NULL, "9000", myresponse);
    exit(0);
    break;
  default:
    /* parent */
    break;
  }

  sleep(1); /* wait for service to start */

  status_code = TANet_requestWithURL("localhost:9000/", TANet_GET, "",
                                     response_body);
  mu_assert(status_code == TANet_OK);
  printf("%s\n", response_body);
  mu_assert(strcmp(response_body, myhtml) == 0);

  status_code = TANet_requestWithURL("localhost:9000/stop", TANet_POST, "",
                                     response_body);
  mu_assert(status_code == TANet_SERVICE_UNAVAILABLE);

  wait(NULL);

  free(response_body);
}

static void test_TANet_request()
{
  TANet tanet = NULL;
  pid_t pid;
  int status_code;
  char *response_body;

  response_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);
  if (response_body == NULL)
    exit(1);

  switch (pid = fork())
  {
  case -1:
    fprintf(stderr, "fork failed\n");
    exit(1);
  case 0:
    /* child */
    TANet_startService(NULL, "9001", myresponse);
    exit(0);
    break;
  default:
    /* parent */
    break;
  }

  sleep(1); /* wait for service to start */

  tanet = TANet_initWithURL("localhost:9001");
  if (tanet == NULL)
    exit(1);

  status_code = TANet_request(tanet, TANet_GET, "/", "", response_body);
  mu_assert(status_code == TANet_OK);
  printf("%s\n", response_body);
  mu_assert(strcmp(response_body, myhtml) == 0);

  status_code = TANet_request(tanet, TANet_GET, "/notfound", "", response_body);
  mu_assert(status_code == TANet_NOT_FOUND);
  printf("%s\n", response_body);
  mu_assert(strcmp(response_body, mynotfoundhtml) == 0);

  status_code = TANet_request(tanet, TANet_POST, "/hello", "world", response_body);
  mu_assert(status_code == TANet_OK);
  printf("%s\n", response_body);
  mu_assert(strcmp(response_body, "hello, world") == 0);

  status_code = TANet_request(tanet, TANet_POST, "/stop", "", response_body);
  mu_assert(status_code == TANet_SERVICE_UNAVAILABLE);

  wait(NULL);

  TANet_release(tanet);
  free(response_body);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TANet_startService);
  mu_run_test(test_TANet_request);
  mu_show_failures();
  return mu_nfail != 0;
}
