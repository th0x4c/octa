/*
 * OCTAOption.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTAOPTION_H_
#define _OCTAOPTION_H_

#include <unistd.h> /* getopt */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strtok strncpy strlen strcmp */
#include <stdlib.h> /* atoi exit */
#include <sys/time.h> /* timeval */

struct OCTAOption
{
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
  int command;
  struct timeval measurement_interval;
  struct timeval rampup_time;
  struct timeval rampdown_time;
  struct timeval think_time;
};
typedef struct OCTAOption OCTAOption;

void OCTAOption_getOption(int argc, char * const argv[], OCTAOption *option);
void OCTAOption_usage();

#endif /* _OCTAOPTION_H_ */
