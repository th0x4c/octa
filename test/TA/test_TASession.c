/*
 * test_TASession.c
 * TA
 *
 * Created by Takashi Hashizume on 03/04/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <TA/TASession.h>
#include "../munit.h"
#include <stdio.h>  /* snprintf */
#include <stdlib.h> /* malloc exit free */
#include <string.h> /* strcpy */
#include <time.h>   /* nanosleep */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

#define TX_COUNT 5
#define MSGSIZE 64

static int myTX(TASession self, void **inout)
{
  int ret = 0;
  char *msg = (char *)*inout;
  TALog log = TASession_log(self);
  char logmsg[MSGSIZE] = "";
  TATXStat stat =  TASession_currentStatByName(self, "my TX");
  struct timespec sleeptp;

  snprintf(logmsg, MSGSIZE, "myTX input: %s", msg);
  TALog_info(log, logmsg);
  strcpy(msg, "from myTX");

  sleeptp.tv_sec = 0;
  sleeptp.tv_nsec = 10000000; /* 10ms */
  nanosleep(&sleeptp, NULL);

  if (TATXStat_count(stat) == 2)
    ret = 999;

  return ret;
}

static void myafterTX(TASession self, void **inout)
{
  char *msg = (char *)*inout;
  TALog log = TASession_log(self);
  char logmsg[MSGSIZE] = "";
  TATXStat stat =  TASession_currentStatByName(self, "my TX");

  snprintf(logmsg, MSGSIZE, "myafterTX input: %s", msg);
  TALog_info(log, logmsg);
  strcpy(msg, "from myafterTX");

  if (TATXStat_count(stat) == TX_COUNT)
    TASession_setStatus(self, TASession_STOP);
}

static void mywhen_errorTX(TASession self, void **inout, int error_code,
                           char *error_message, size_t error_message_size)
{
  char *msg = (char *)*inout;
  TALog log = TASession_log(self);
  char logmsg[MSGSIZE] = "";

  snprintf(logmsg, MSGSIZE, "mywhen_errorTX input: %s", msg);
  TALog_info(log, logmsg);
  strcpy(msg, "from mywhen_errorTX");

  snprintf(error_message, error_message_size, "ERROR-%d Occurred", error_code);
}

static void mysetup(TASession self, void **inout)
{
  char *msg = (char *)*inout;
  TALog log = TASession_log(self);
  char logmsg[MSGSIZE] = "";

  snprintf(logmsg, MSGSIZE, "mysetup input: %s", msg);
  TALog_info(log, logmsg);
  strcpy(msg, "from mysetup");

  TASession_setTX(self, myTX, "my TX");
  TASession_setAfterTX(self, myafterTX, "my TX");
  TASession_setWhenErrorTX(self, mywhen_errorTX, "my TX");
  TASession_setStatus(self, TASession_RUNNING);
}

static char *myselectTX(TASession self)
{
#define MAX_NAME_SIZE 64
  static char mytx_name[MAX_NAME_SIZE] = "my TX";

  return mytx_name;
}

static void myteardown(TASession self, void **inout)
{
  char *msg = (char *)*inout;
  TALog log = TASession_log(self);
  char logmsg[MSGSIZE] = "";
  TATXStat stat = TASession_statByNameInPeriodInPhase(self, "my TX",
                                                      TASession_MEASUREMENT,
                                                      TASession_TX);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  snprintf(logmsg, MSGSIZE, "myteardown input: %s", msg);
  TALog_info(log, logmsg);
  strcpy(msg, "from myteardown");

  TALog_info(log, TATXStat_description(stat, desc, DESC_SIZE));
}

static void test_TASession_main()
{
  TASession tasession = TASession_init();
  char *msg = "";

  msg = malloc(sizeof(char) * MSGSIZE);
  if (msg == NULL)
    exit(1);

  strcpy(msg, "from main");

  TASession_setID(tasession, 1);
  TASession_setSetup(tasession, mysetup);
  TASession_setSelectTX(tasession, myselectTX);
  TASession_setTeardown(tasession, myteardown);

  mu_assert(TASession_main(tasession, (void **)&msg) == 0);
  TASession_release(tasession);
  free(msg);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TASession_main);
  mu_show_failures();
  return mu_nfail != 0;
}
