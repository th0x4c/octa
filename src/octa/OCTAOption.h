/*
 * OCTAOption.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTAOPTION_H_
#define _OCTAOPTION_H_

#include <unistd.h>   /* getopt */
#include <stdio.h>    /* fprintf printf */
#include <string.h>   /* strtok strncpy strlen strcmp */
#include <stdlib.h>   /* atoi exit */
#include <sys/time.h> /* timeval */
#include <TA/TA.h>
#include "OCTACConfig.h"

struct OCTAOption
{
#define OCTA_TPCB 1
#define OCTA_TPCC 2
  int mode;
#define MAX_NAME_SIZE 30
  char username[MAX_NAME_SIZE];
  char password[MAX_NAME_SIZE];
  char tnsname[MAX_NAME_SIZE];
  char table_tablespace[MAX_NAME_SIZE];
  char index_tablespace[MAX_NAME_SIZE];
  int num_sessions;
  int scale_factor;
#define OCTA_SETUP    1
#define OCTA_LOAD     2
#define OCTA_BENCH    3
#define OCTA_TEARDOWN 4
#define OCTA_DIST     5
  int command;
  struct timeval measurement_interval;
  struct timeval rampup_time;
  struct timeval rampdown_time;
#define TXS 5
  struct timeval keying_time[TXS];
  struct timeval think_time[TXS];
  int tx_percentage[TXS];
  unsigned short port;
  TABool long_format;
  TABool select_only;
#define MAX_REMOTE_URL_SIZE 128
  char urls[MAX_REMOTE_URL_SIZE][MAX_NAME_SIZE];
};
typedef struct OCTAOption OCTAOption;

void OCTAOption_getOption(int argc, char * const argv[], OCTAOption *option);
void OCTAOption_usage();
void OCTAOption_print(OCTAOption option);

#endif /* _OCTAOPTION_H_ */
