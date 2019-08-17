/*
 * OCTACDist.c
 * OCTA
 *
 * Created by Takashi Hashizume on 08/17/19.
 * Copyright 2019 Takashi Hashizume. All rights reserved.
 */

#include "OCTACDist.h"

static TABool long_format;

static void OCTACDist_beforeSetup(TASessionManager self, void **inout)
{
  OCTAOption *opt = (OCTAOption *)*inout;
#define MAX_HOSTNAME_LENGTH 128
#define MAX_PATH_LENGTH 128
  char url[MAX_HOSTNAME_LENGTH + MAX_PATH_LENGTH];
  char *request_body;
  char *response_body;
  struct timeval start_time;
  char start_time_str[24] = "0000-00-00 00:00:00.000";
  int i = 0;

  long_format = opt->long_format;

  OCTAOption_print(*opt);

  timerclear(&start_time);
  gettimeofday(&start_time, (struct timezone *)0);
  timeval2str(start_time_str, start_time);

  request_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);
  response_body = malloc(sizeof(char) * TANet_MAX_BODY_LENGTH);

  sprintf(request_body,
          "{rampup_interval:%d,measurement_interval:%d,rampdown_interval:%d}",
          (int) timeval2sec(opt->rampup_time),
          (int) timeval2sec(opt->measurement_interval),
          (int) timeval2sec(opt->rampdown_time));

  for (i = 0; i < TASessionManager_numberOfSessions(self); i++)
  {
    sprintf(url, "%s/period-interval", opt->urls[i]);
    TANet_requestWithURL(url, TANet_POST, request_body, response_body);
  }

  printf("----------------------------------------------------------------\n");
  printf("              Start Time : %s\n", start_time_str);
  printf("----------------------------------------------------------------\n");

  free(request_body);
  free(response_body);
}

static void OCTACDist_monitor(TASessionManager self)
{
#define MONITOR_INTERVAL 4
#define PAGESIZE 20
  struct timespec monitor_interval;

  monitor_interval.tv_sec = MONITOR_INTERVAL;
  monitor_interval.tv_nsec = 0;
  nanosleep(&monitor_interval, NULL);

  TASessionManager_printMonitoredTX(self, "New-Order", PAGESIZE, long_format);
}

static void OCTACDist_afterTeardown(TASessionManager self, void **inout)
{
  TATXStat summary_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "New-Order", TASession_MEASUREMENT, TASession_TX);
  struct timeval end_timeval;
  char end_time_str[24] = "0000-00-00 00:00:00.000";
  static char *tx_names[TXS] =
                {"New-Order", "Payment", "Order-Status", "Delivery",
                 "Stock-Level"};

  TASessionManager_printMonitoredTX(self, "New-Order", PAGESIZE, long_format);
  TADistribution_print(TATXStat_distribution(summary_stat));
  TASessionManager_printNumericalQuantitiesSummary(self, tx_names, TXS);

  timerclear(&end_timeval);
  gettimeofday(&end_timeval, (struct timezone *)0);
  timeval2str(end_time_str, end_timeval);
  printf("----------------------------------------------------------------\n");
  printf("               End Time : %s\n", end_time_str);
  printf("----------------------------------------------------------------\n");
}

int OCTACDist_main(const OCTAOption *opt)
{
  TASession session_prototype = TASession_init();
  TASessionManager session_manager = NULL;
  int ret = 0;
  int i = 0;

  TASession_setTX(session_prototype, NULL, "New-Order");
  TASession_setTX(session_prototype, NULL, "Payment");
  TASession_setTX(session_prototype, NULL, "Order-Status");
  TASession_setTX(session_prototype, NULL, "Delivery");
  TASession_setTX(session_prototype, NULL, "Stock-Level");

  session_manager =
    TASessionManager_initWithSessionPrototype(session_prototype,
                                              opt->num_sessions);
  for (i = 0; i < opt->num_sessions; i++)
  {
    TASessionManager_setURL(session_manager, opt->urls[i]);
  }
  TASessionManager_setBeforeSetup(session_manager, OCTACDist_beforeSetup);
  TASessionManager_setAfterTeardown(session_manager, OCTACDist_afterTeardown);
  TASessionManager_setMonitor(session_manager, OCTACDist_monitor);
  if (opt->port > 0)
    TASessionManager_setPort(session_manager, opt->port);

  ret = TASessionManager_main(session_manager, (void **)&opt);
  return ret;
}
