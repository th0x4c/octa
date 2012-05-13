/*
 * TASessionManager.c
 * TA
 *
 * Created by Takashi Hashizume on 03/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "TASessionManager.h"

struct __TASessionManager
{
  int num_sessions;
  int sessions_shmid;
  void (*beforeSetup)(TASessionManager self, void **inout);
  void (*afterTeardown)(TASessionManager self, void **inout);
  void (*monitor)(TASessionManager self);
  TASession *sessions;
};

static volatile sig_atomic_t sigflag = 0;

static TABool TASessionManager_isAllStatus(TASessionManager self, int status);
static void TASessionManager_setAllStatus(TASessionManager self, int status);
static void TASessionManager_toggleAllStatus(TASessionManager self);
static void TASessionManager_moveAllPeriod(TASessionManager self);
static void TASessionManager_signalHandler(int sig);

TASessionManager TASessionManager_initWithSessionPrototype(TASession prototype,
                                                           int num_sessions)
{
  struct __TASessionManager *self = malloc(sizeof(struct __TASessionManager));
  TASession session = NULL;
  int i = 0;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->num_sessions = num_sessions;
  self->sessions_shmid = shmget(IPC_PRIVATE,
                                sizeof(TASession) * num_sessions +
                                TASession_sizeof() * num_sessions,
                                0666 | IPC_CREAT);
  if (self->sessions_shmid < 0)
  {
    fprintf(stderr, "shmget failed [%s]\n", strerror(errno));
    exit(1);
  }
  self->beforeSetup = NULL;
  self->afterTeardown = NULL;
  self->monitor = NULL;
  self->sessions = (TASession *) shmat(self->sessions_shmid, 0, 0);
  if (self->sessions == (void *)-1)
  {
    fprintf(stderr, "shmat failed [%s]\n", strerror(errno));
    exit(1);
  }
  session = (TASession) (self->sessions + num_sessions);
  for (i = 0; i < num_sessions; i++)
  {
    if (i != 0)
      session = TASession_nextAddr(session);

    self->sessions[i] = session;
    TASession_deepCopy(prototype, session);
  }

  return self;
}

void TASessionManager_release(TASessionManager self)
{
  if (shmdt(self->sessions) == -1)
  {
    fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
    exit(1);
  }
  if (shmctl(self->sessions_shmid, IPC_RMID, 0) == -1)
  {
    fprintf(stderr, "shmctl failed [%s]\n", strerror(errno));
    exit(1);
  }
  free(self);
}

int TASessionManager_numberOfSessions(TASessionManager self)
{
  return self->num_sessions;
}

TASession *TASessionManager_sessions(TASessionManager self)
{
  return self->sessions;
}

void TASessionManager_setBeforeSetup(TASessionManager self,
                                     void (*beforeSetup)(TASessionManager self,
                                                         void **inout))
{
  self->beforeSetup = beforeSetup;
}

void TASessionManager_setAfterTeardown(TASessionManager self,
                                       void (*afterTeardown)(
                                                TASessionManager self,
                                                void **inout))
{
  self->afterTeardown = afterTeardown;
}

void TASessionManager_setMonitor(TASessionManager self,
                                 void (*monitor)(TASessionManager self))
{
  self->monitor = monitor;
}

TATXStat TASessionManager_summaryStatByNameInPeriodInPhase(
           TASessionManager self,
           const char *tx_name,
           int period, int phase)
{
  TATXStat summary_stat = TATXStat_init();
  TATXStat tmp_stat = NULL;
  TATXStat ses_stat = NULL;
  int i = 0;

  for (i = 0; i < self->num_sessions; i++)
  {
    tmp_stat = summary_stat;
    ses_stat = TASession_statByNameInPeriodInPhase(self->sessions[i], tx_name,
                                                   period, phase);
    summary_stat = TATXStat_plus(summary_stat, ses_stat);
    TATXStat_release(tmp_stat);
  }
  TATXStat_setName(summary_stat, tx_name);

  /* returned TATXStat must be released by caller */
  return summary_stat;
}

void TASessionManager_printMonitoredTX(TASessionManager self,
                                       const char *tx_name, int pagesize)
{
  static int monitor_count = 0;
  static TATXStat pre_summary_rampup = NULL;
  static TATXStat pre_summary_measurement = NULL;
  static TATXStat pre_summary_rampdown = NULL;
  TATXStat summary_rampup, summary_measurement, summary_rampdown;
  TATXStat diff_rampup, diff_measurement, diff_rampdown;
  struct timeval current_time;
  char current_time_str[24] = "0000-00-00 00:00:00.000";
  char short_time_str[15];
  struct timeval avg;

  if (pre_summary_rampup == NULL)
    pre_summary_rampup = TATXStat_init();
  if (pre_summary_measurement == NULL)
    pre_summary_measurement = TATXStat_init();
  if (pre_summary_rampdown == NULL)
    pre_summary_rampdown = TATXStat_init();

  timerclear(&avg);
  timerclear(&current_time);
  gettimeofday(&current_time, (struct timezone *)0);
  timeval2str(current_time_str, current_time);
  sprintf(short_time_str, "%.*s", 14, current_time_str + 5);

  summary_rampup = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_name, TASession_RAMPUP, TASession_TX);
  summary_measurement = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                          tx_name, TASession_MEASUREMENT, TASession_TX);
  summary_rampdown = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                       tx_name, TASession_RAMPDOWN, TASession_TX);

  diff_rampup = TATXStat_minus(summary_rampup, pre_summary_rampup);
  diff_measurement = TATXStat_minus(summary_measurement,
                                    pre_summary_measurement);
  diff_rampdown = TATXStat_minus(summary_rampdown, pre_summary_rampdown);

  if (pagesize > 0 && (monitor_count % pagesize) == 0)
  {
    printf("Time           Period      Count    Error    AVG      TPS\n");
    printf("-------------- ----------- -------- -------- -------- --------\n");
  }
  monitor_count++;

  if (TATXStat_count(diff_rampup) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_rampup);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Ramp-up",
           TATXStat_count(diff_rampup),
           TATXStat_errorCount(diff_rampup),
           timeval2sec(avg), TATXStat_tps(diff_rampup));
  }
  if (TATXStat_count(diff_measurement) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_measurement);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Measurement",
           TATXStat_count(diff_measurement),
           TATXStat_errorCount(diff_measurement),
           timeval2sec(avg), TATXStat_tps(diff_measurement));
  }
  if (TATXStat_count(diff_rampdown) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_rampdown);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Ramp-down",
           TATXStat_count(diff_rampdown),
           TATXStat_errorCount(diff_rampdown),
           timeval2sec(avg), TATXStat_tps(diff_rampdown));
  }
  fflush(stdout);

  TATXStat_release(pre_summary_rampup);
  TATXStat_release(pre_summary_measurement);
  TATXStat_release(pre_summary_rampdown);
  TATXStat_release(diff_rampup);
  TATXStat_release(diff_measurement);
  TATXStat_release(diff_rampdown);

  pre_summary_rampup = summary_rampup;
  pre_summary_measurement = summary_measurement;
  pre_summary_rampdown = summary_rampdown;
}

void TASessionManager_printNumericalQuantitiesSummary(TASessionManager self,
                                                      char *tx_names[],
                                                      int tx_count)
{
  TATXStat summary_stat = NULL;
  TATXStat before_stat = NULL;
  TATXStat after_stat = NULL;
  long total_count = 0;
  struct timeval rampup_first_time, first_time, end_time;
  struct timeval rampup_interval, measurement_interval;
  struct timeval tmp_time;
  int i = 0;

  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    total_count += TATXStat_count(summary_stat);
    TATXStat_release(summary_stat);
  }

  timerclear(&rampup_first_time);
  timerclear(&first_time);
  timerclear(&end_time);
  timerclear(&rampup_interval);
  timerclear(&measurement_interval);
  timerclear(&tmp_time);
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_RAMPUP, TASession_BEFORE);
    tmp_time = TATXStat_firstTime(summary_stat);
    if (i == 0)
      rampup_first_time = tmp_time;
    else
    {
      if (timercmp(&tmp_time, &rampup_first_time, <))
        rampup_first_time = tmp_time;
    }
    TATXStat_release(summary_stat);
  }
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_BEFORE);
    tmp_time = TATXStat_firstTime(summary_stat);
    if (i == 0)
      first_time = tmp_time;
    else
    {
      if (timercmp(&tmp_time, &first_time, <))
        first_time = tmp_time;
    }
    TATXStat_release(summary_stat);
  }
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_AFTER);
    tmp_time = TATXStat_endTime(summary_stat);
    if (i == 0)
      end_time = tmp_time;
    else
    {
      if (timercmp(&end_time, &tmp_time, <))
        end_time = tmp_time;
    }
    TATXStat_release(summary_stat);
  }
  if (timerisset(&rampup_first_time) && timerisset(&first_time))
    timersub(&first_time, &rampup_first_time, &rampup_interval);
  if (timerisset(&first_time) && timerisset(&end_time))
    timersub(&end_time, &first_time, &measurement_interval);

  printf("================================================================\n");
  printf("================= Numerical Quantities Summary =================\n");
  printf("================================================================\n");

  summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                   tx_names[0], TASession_MEASUREMENT, TASession_TX);
  printf("MQTh, computed Maximum Qualified Throughput %16.2f tpm\n",
         TATXStat_tps(summary_stat) * 60);
  printf("  - %-39s %16.2f tps\n", TATXStat_name(summary_stat),
         TATXStat_tps(summary_stat));
  TATXStat_release(summary_stat);
  printf("\n");

  printf("Response Times (minimum/ Average/ maximum) in seconds\n");
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    printf("  - %-29s %8.6f / %8.6f / %8.6f\n",
           tx_names[i],
           timeval2sec(TATXStat_minElapsedTime(summary_stat)),
           timeval2sec(TATXStat_avgElapsedTime(summary_stat)),
           timeval2sec(TATXStat_maxElapsedTime(summary_stat)));
    TATXStat_release(summary_stat);
  }
  printf("\n");

  printf("Response Times (50th/ 80th/ 90th percentile) in seconds\n");
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    printf("  - %-29s %-8.3f / %-8.3f / %-8.3f\n",
           tx_names[i],
           timeval2sec(TADistribution_percentile(
                         TATXStat_distribution(summary_stat), 50)),
           timeval2sec(TADistribution_percentile(
                         TATXStat_distribution(summary_stat), 80)),
           timeval2sec(TADistribution_percentile(
                         TATXStat_distribution(summary_stat), 90)));
    TATXStat_release(summary_stat);
  }
  printf("\n");

  printf("Transaction Mix, in percent of total transactions\n");
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    printf("  - %-51s %6.2f %%\n",
           tx_names[i],
           total_count == 0 ? 0.0 :
           ((double) TATXStat_count(summary_stat)) * 100 / total_count);
    TATXStat_release(summary_stat);
  }
  printf("\n");

  printf("Keying/Think Times (in seconds),\n");
  printf("                         Min.          Average           Max.\n");
  for (i = 0; i < tx_count; i++)
  {
    before_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                    tx_names[i], TASession_MEASUREMENT, TASession_BEFORE);
    after_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                   tx_names[i], TASession_MEASUREMENT, TASession_AFTER);
    printf("  - %-12s %7.3f/%7.3f %7.3f/%7.3f %7.3f/%7.3f\n",
           tx_names[i],
           timeval2sec(TATXStat_minElapsedTime(before_stat)),
           timeval2sec(TATXStat_minElapsedTime(after_stat)),
           timeval2sec(TATXStat_avgElapsedTime(before_stat)),
           timeval2sec(TATXStat_avgElapsedTime(after_stat)),
           timeval2sec(TATXStat_maxElapsedTime(before_stat)),
           timeval2sec(TATXStat_maxElapsedTime(after_stat)));
    TATXStat_release(before_stat);
    TATXStat_release(after_stat);
  }
  printf("\n");

  printf("Test Duration\n");
  printf("  - Ramp-up time %39.3f seconds\n", timeval2sec(rampup_interval));
  printf("  - Measurement interval %31.3f seconds\n",
         timeval2sec(measurement_interval));
  printf("  - Number of transactions (all types)\n");
  printf("    completed in measurement interval %26ld\n", total_count);
  printf("================================================================\n");
  fflush(stdout);
}

int TASessionManager_main(TASessionManager self, void **inout)
{
  TASession session = NULL;
  pid_t pid;
  int i = 0;
  struct timespec sleeptp;
  struct sigaction act, oldact;

  sleeptp.tv_sec = 1;  /* 1s */
  sleeptp.tv_nsec = 0;

  if (self->beforeSetup)
    self->beforeSetup(self, inout);

  for (i = 0; i < self->num_sessions; i++)
  {
    session = self->sessions[i];
    TASession_setID(session, i + 1);

    switch (pid = fork())
    {
    case -1:
      fprintf(stderr, "fork failed\n");
      exit(1);
    case 0:
      /* child */
      TASession_main(session, inout);
      if (shmdt(self->sessions) == -1)
      {
        fprintf(stderr, "shmdt failed\n");
        exit(1);
      }
      exit(0);
      break;
    default:
      /* parent */
      break;
    }
  }

  /* signal handler */
  act.sa_handler = TASessionManager_signalHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, NULL, &oldact);
  if (oldact.sa_handler != SIG_IGN)
    sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, NULL, &oldact);
  if (oldact.sa_handler != SIG_IGN)
    sigaction(SIGTERM, &act, 0);
  sigaction(SIGUSR1, NULL, &oldact);
  if (oldact.sa_handler != SIG_IGN)
    sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, NULL, &oldact);
  if (oldact.sa_handler != SIG_IGN)
    sigaction(SIGUSR2, &act, 0);

  while (! TASessionManager_isAllStatus(self, TASession_TERM))
  {
    if (self->monitor)
      self->monitor(self);
    else
      nanosleep(&sleeptp, NULL);

    switch (sigflag)
    {
    case SIGINT:
      TASessionManager_setAllStatus(self, TASession_STOP);
      sigflag = 0;
      break;
    case SIGTERM:
      TASessionManager_setAllStatus(self, TASession_STOP);
      sigflag = 0;
      break;
    case SIGUSR1:
      TASessionManager_toggleAllStatus(self);
      sigflag = 0;
      break;
    case SIGUSR2:
      TASessionManager_moveAllPeriod(self);
      sigflag = 0;
      break;
    default:
      break;
    }
  }

  if (self->afterTeardown)
    self->afterTeardown(self, inout);

  wait(NULL);
  TASessionManager_release(self);

  return 0;
}

/* private */
static TABool TASessionManager_isAllStatus(TASessionManager self, int status)
{
  int i = 0;
  
  for (i = 0; i < self->num_sessions; i++)
  {
    if (TASession_status(self->sessions[i]) != status)
      return FALSE;
  }

  return TRUE;
}

static void TASessionManager_setAllStatus(TASessionManager self, int status)
{
  int i = 0;
  
  for (i = 0; i < self->num_sessions; i++)
  {
    TASession_setStatus(self->sessions[i], status);
  }
}

static void TASessionManager_toggleAllStatus(TASessionManager self)
{
  int i = 0;
  
  for (i = 0; i < self->num_sessions; i++)
  {
    TASession_toggleStatus(self->sessions[i]);
  }
}

static void TASessionManager_moveAllPeriod(TASessionManager self)
{
  int i = 0;
  
  for (i = 0; i < self->num_sessions; i++)
  {
    TASession_movePeriod(self->sessions[i]);
  }
}

static void TASessionManager_signalHandler(int sig)
{
  sigflag = sig;
}
