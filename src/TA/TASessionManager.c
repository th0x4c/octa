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
  unsigned short port;
#define MAX_REMOTE_URL_SIZE 128
  char *urls[MAX_REMOTE_URL_SIZE];
};

static volatile sig_atomic_t sigflag = 0;

static TABool TASessionManager_isAllStatus(TASessionManager self, int status);
static TABool TASessionManager_isAnyStatus(TASessionManager self, int status);
static TABool TASessionManager_isAllPeriod(TASessionManager self, int period);
static TABool TASessionManager_isAnyPeriod(TASessionManager self, int period);
static void TASessionManager_signalHandler(int sig);
static void TASessionManager_printLineMonitoredTX(TASessionManager self,
                                                  TATXStat txstat,
                                                  const char *tx_name,
                                                  struct timeval current_time,
                                                  int period,
                                                  TABool long_format);
static void TASessionManager_printTestDuration(TASessionManager self,
                                               char *tx_names[], int tx_count);
static int TASessionManager_response(void *front_object, int method,
                                     const char *path, long content_length,
                                     const char *request_body,
                                     char *response_body);

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
  self->port = 0;
  for (i = 0; i < MAX_REMOTE_URL_SIZE; i++)
  {
    self->urls[i] = NULL;
  }

  return self;
}

void TASessionManager_release(TASessionManager self)
{
  int i = 0;

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

  for (i = 0; i < MAX_REMOTE_URL_SIZE; i++)
  {
    if (self->urls[i] != NULL)
      free(self->urls[i]);
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

void TASessionManager_setPort(TASessionManager self, unsigned short port)
{
  if (port > 0 && port < 65536)
    self->port = port;
}

void TASessionManager_setURL(TASessionManager self, const char *url)
{
#define MAX_HOSTNAME_LENGTH 128
#define MAX_PORT_LENGTH 6 /* 0-65535 + '\0' */
  static int url_idx = 0;

  self->urls[url_idx] =
    malloc(sizeof(char) * (MAX_HOSTNAME_LENGTH + MAX_PORT_LENGTH));
  if (self->urls[url_idx] != NULL)
    strcpy(self->urls[url_idx], url);

  url_idx++;
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
                                       const char *tx_name, int pagesize,
                                       TABool long_format)
{
  static int monitor_count = 0;
  static TATXStat pre_summary_rampup = NULL;
  static TATXStat pre_summary_measurement = NULL;
  static TATXStat pre_summary_rampdown = NULL;
  TATXStat summary_rampup, summary_measurement, summary_rampdown;
  TATXStat diff_rampup, diff_measurement, diff_rampdown;
  struct timeval current_time;

  if (pre_summary_rampup == NULL)
    pre_summary_rampup = TATXStat_init();
  if (pre_summary_measurement == NULL)
    pre_summary_measurement = TATXStat_init();
  if (pre_summary_rampdown == NULL)
    pre_summary_rampdown = TATXStat_init();

  timerclear(&current_time);
  gettimeofday(&current_time, (struct timezone *)0);

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
    printf("\n");
    printf("Time           Period      Count    Error    AVG      TPS     ");
    if (long_format)
      printf(" 0%%tile   50%%tile  80%%tile  90%%tile  100%%tile Transaction");

    printf("\n");

    printf("-------------- ----------- -------- -------- -------- --------");
    if (long_format)
      printf(" -------- -------- -------- -------- -------- -----------");

    printf("\n");
  }
  monitor_count++;

  TASessionManager_printLineMonitoredTX(self, diff_rampup, tx_name,
                                        current_time, TASession_RAMPUP,
                                        long_format);
  TASessionManager_printLineMonitoredTX(self, diff_measurement, tx_name,
                                        current_time, TASession_MEASUREMENT,
                                        long_format);
  TASessionManager_printLineMonitoredTX(self, diff_rampdown, tx_name,
                                        current_time, TASession_RAMPDOWN,
                                        long_format);
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
  int i = 0;

  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    total_count += TATXStat_count(summary_stat);
    TATXStat_release(summary_stat);
  }

  printf("================================================================\n");
  printf("================= Numerical Quantities Summary =================\n");
  printf("================================================================\n");

  summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                   tx_names[0], TASession_MEASUREMENT, TASession_TX);
  printf("MQTh, computed Maximum Qualified Throughput %16.2f tpm\n",
         TATXStat_tps(summary_stat) * 60);
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    printf("  - %-39s %16.2f tps\n", tx_names[i], TATXStat_tps(summary_stat));
    TATXStat_release(summary_stat);
  }
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
    printf("  - %-29s %-8.6f / %-8.6f / %-8.6f\n",
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
    printf("  - %-33s %15d / %6.2f %%\n",
           tx_names[i],
           TATXStat_count(summary_stat),
           total_count == 0 ? 0.0 :
           ((double) TATXStat_count(summary_stat)) * 100 / total_count);
    TATXStat_release(summary_stat);
  }
  printf("\n");

  printf("Errors (number of errors/ percentage)\n");
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_MEASUREMENT, TASession_TX);
    printf("  - %-33s %15d / %6.2f %%\n",
           tx_names[i],
           TATXStat_errorCount(summary_stat),
           TATXStat_count(summary_stat) == 0 ? 0.0 :
           ((double) TATXStat_errorCount(summary_stat)) * 100 /
           TATXStat_count(summary_stat));
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
  TASessionManager_printTestDuration(self, tx_names, tx_count);
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
#define MAX_PORT_LENGTH 6 /* 0-65535 + '\0' */
  char port[MAX_PORT_LENGTH];

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
      if (self->urls[i] == NULL)
        TASession_main(session, inout);
      else
        TASession_mainWithURL(session, self->urls[i]);

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

  if (self->port > 0)
  {
    switch (pid = fork())
    {
    case -1:
      fprintf(stderr, "fork failed\n");
      exit(1);
    case 0:
      /* child */
      sprintf(port, "%d", self->port);
      TANet_startService(self, port, TASessionManager_response);
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
      /* ignore SIGINT */
      sigflag = 0;
      break;
    case SIGTERM:
      kill(0, SIGTERM);
      sigflag = 0;
      break;
    case SIGUSR1:
      kill(0, SIGUSR1);
      sigflag = 0;
      break;
    case SIGUSR2:
      kill(0, SIGUSR2);
      sigflag = 0;
      break;
    default:
      break;
    }
  }

  if (self->afterTeardown)
    self->afterTeardown(self, inout);

  waitpid(pid, NULL, 0);
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

static TABool TASessionManager_isAnyStatus(TASessionManager self, int status)
{
  int i = 0;

  for (i = 0; i < self->num_sessions; i++)
  {
    if (TASession_status(self->sessions[i]) == status)
      return TRUE;
  }

  return FALSE;
}

static TABool TASessionManager_isAllPeriod(TASessionManager self, int period)
{
  int i = 0;

  for (i = 0; i < self->num_sessions; i++)
  {
    if (TASession_period(self->sessions[i]) != period)
      return FALSE;
  }

  return TRUE;
}

static TABool TASessionManager_isAnyPeriod(TASessionManager self, int period)
{
  int i = 0;

  for (i = 0; i < self->num_sessions; i++)
  {
    if (TASession_period(self->sessions[i]) == period)
      return TRUE;
  }

  return FALSE;
}

static void TASessionManager_signalHandler(int sig)
{
  sigflag = sig;
}

static void TASessionManager_printLineMonitoredTX(TASessionManager self,
                                                  TATXStat txstat,
                                                  const char *tx_name,
                                                  struct timeval current_time,
                                                  int period,
                                                  TABool long_format)
{
  char current_time_str[24] = "0000-00-00 00:00:00.000";
  char short_time_str[15];
  struct timeval avg;
#define NUM_PERIOD 3
  char *period_strs[NUM_PERIOD] = { "Ramp-up", "Measurement", "Ramp-down" };
  TADistribution distribution = NULL;

  timerclear(&avg);
  timeval2str(current_time_str, current_time);
  sprintf(short_time_str, "%.*s", 14, current_time_str + 5);

  if (TATXStat_count(txstat) > 0)
  {
    avg = TATXStat_avgElapsedTime(txstat);
    printf("%s %-11s %8d %8d %8.6f %8.3f", short_time_str, period_strs[period],
           TATXStat_count(txstat),
           TATXStat_errorCount(txstat),
           timeval2sec(avg), TATXStat_tps(txstat));
    if (long_format)
    {
      distribution = TATXStat_distribution(txstat);
      printf(" %8.6f %8.6f %8.6f %8.6f %8.6f %s",
             timeval2sec(TADistribution_percentile(distribution, 0)),
             timeval2sec(TADistribution_percentile(distribution, 50)),
             timeval2sec(TADistribution_percentile(distribution, 80)),
             timeval2sec(TADistribution_percentile(distribution, 90)),
             timeval2sec(TADistribution_percentile(distribution, 100)),
             tx_name);
    }
    printf("\n");
  }
  else if (TATXStat_count(txstat) == 0 &&
           TASessionManager_isAnyPeriod(self, period) &&
           ! TASessionManager_isAnyStatus(self, TASession_TERM))
  {
    printf("%s %-11s %8d %8d %8s %8.3f", short_time_str, period_strs[period],
           TATXStat_count(txstat),
           TATXStat_errorCount(txstat),
           "", TATXStat_tps(txstat));
    if (long_format)
      printf(" %8s %8s %8s %8s %8s %s", "", "", "", "", "", tx_name);

    printf("\n");
  }
}

static void TASessionManager_printTestDuration(TASessionManager self,
                                               char *tx_names[], int tx_count)
{
  TATXStat summary_stat = NULL;
  struct timeval rampup_first_time, first_time, end_time, rampdown_end_time;
  struct timeval rampup_interval, measurement_interval, rampdown_interval;
  struct timeval tmp_time;
  char from_time_str[24] = "0000-00-00 00:00:00.000";
  char to_time_str[24] = "0000-00-00 00:00:00.000";
  int i = 0;

  timerclear(&rampup_first_time);
  timerclear(&first_time);
  timerclear(&end_time);
  timerclear(&rampdown_end_time);
  timerclear(&rampup_interval);
  timerclear(&measurement_interval);
  timerclear(&rampdown_interval);
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
  for (i = 0; i < tx_count; i++)
  {
    summary_stat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     tx_names[i], TASession_RAMPDOWN, TASession_AFTER);
    tmp_time = TATXStat_endTime(summary_stat);
    if (i == 0)
      rampdown_end_time = tmp_time;
    else
    {
      if (timercmp(&rampdown_end_time, &tmp_time, <))
        rampdown_end_time = tmp_time;
    }
    TATXStat_release(summary_stat);
  }
  if (timerisset(&rampup_first_time) && timerisset(&first_time))
    timersub(&first_time, &rampup_first_time, &rampup_interval);
  if (timerisset(&first_time) && timerisset(&end_time))
    timersub(&end_time, &first_time, &measurement_interval);
  if (timerisset(&end_time) && timerisset(&rampdown_end_time))
    timersub(&rampdown_end_time, &end_time, &rampdown_interval);

  printf("  - Ramp-up time %39.3f seconds\n", timeval2sec(rampup_interval));
  if (timerisset(&rampup_interval))
  {
    timeval2str(from_time_str, rampup_first_time);
    timeval2str(to_time_str, first_time);
    printf("             (%23s / %23s)\n", from_time_str, to_time_str);
  }
  printf("  - Measurement interval %31.3f seconds\n",
         timeval2sec(measurement_interval));
  timeval2str(from_time_str, first_time);
  timeval2str(to_time_str, end_time);
  printf("             (%23s / %23s)\n", from_time_str, to_time_str);
  printf("  - Ramp-down time %37.3f seconds\n", timeval2sec(rampdown_interval));
  if (timerisset(&rampdown_interval))
  {
    timeval2str(from_time_str, end_time);
    timeval2str(to_time_str, rampdown_end_time);
    printf("             (%23s / %23s)\n", from_time_str, to_time_str);
  }
  fflush(stdout);
}

static int TASessionManager_response(void *front_object, int method,
                                     const char *path, long content_length,
                                     const char *request_body,
                                     char *response_body)
{
  TASessionManager self = (TASessionManager)front_object;
#define MAX_NAME_SIZE 64
  char tx_name[MAX_NAME_SIZE];
  char period_str[MAX_NAME_SIZE];
  char phase_str[MAX_NAME_SIZE];
  int period = -1;
  int phase = -1;
  TATXStat tatxstat = NULL;
  char status_str[MAX_NAME_SIZE];
  struct timeval start_time;
  int rampup_interval = 0;
  int measurement_interval = 0;
  int rampdown_interval = 0;
#define notfoundhtml                  \
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
  char *errorbody = notfoundhtml;
  char *s, *e, *end;
  int i = 0;

  switch (method)
  {
  case TANet_GET:
  case TANet_HEAD:
    if (strncmp(path, "/stat/:tx_name/:period/:phase", 6) == 0)
    {
      s = strstr(path, "/stat/");
      if (s != NULL)
      {
        s = s + strlen("/stat/");
        e = strchr(s, '/');
        snprintf(tx_name, (size_t) (e - s + 1), "%s", s);
      }
      s = e + 1;
      e = strchr(s, '/');
      if (e != NULL)
      {
        snprintf(period_str, (size_t) (e - s + 1), "%s", s);
        if (strcmp(period_str, "rampup") == 0)
          period = TASession_RAMPUP;
        else if (strcmp(period_str, "measurement") == 0)
          period = TASession_MEASUREMENT;
        else if (strcmp(period_str, "rampdown") == 0)
          period = TASession_RAMPDOWN;
      }
      s = e + 1;
      if (s[0] != '\0')
      {
        strcpy(phase_str, s);
        if (strcmp(phase_str, "before") == 0)
          phase = TASession_BEFORE;
        else if (strcmp(phase_str, "tx") == 0)
          phase = TASession_TX;
        else if (strcmp(phase_str, "after") == 0)
          phase = TASession_AFTER;
      }

      if (period == -1 || phase == -1)
      {
        strcpy(response_body, errorbody);
        return TANet_NOT_FOUND;
      }

      tatxstat = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                   tx_name, period, phase);
      TATXStat_JSON(tatxstat, response_body, TANet_MAX_BODY_LENGTH);
      TATXStat_release(tatxstat);
      return TANet_OK;
    }
    else if (strcmp(path, "/status") == 0)
    {
      if (TASessionManager_isAllStatus(self, TASession_INIT))
        strcpy(status_str, "init");
      else if (TASessionManager_isAnyStatus(self, TASession_STANDBY))
        strcpy(status_str, "standby");

      if (TASessionManager_isAnyStatus(self, TASession_RUNNING))
        strcpy(status_str, "running");
      else if (TASessionManager_isAnyStatus(self, TASession_STOP))
        strcpy(status_str, "stop");

      if (TASessionManager_isAllStatus(self, TASession_TERM))
        strcpy(status_str, "term");
      else if (TASessionManager_isAnyStatus(self, TASession_INIT))
        strcpy(status_str, "init");

      sprintf(response_body, "{status:%s}", status_str);
      return TANet_OK;
    }
    else if (strcmp(path, "/period") == 0)
    {
      if (TASessionManager_isAllPeriod(self, TASession_RAMPUP))
        strcpy(period_str, "rampup");
      else if (TASessionManager_isAnyPeriod(self, TASession_MEASUREMENT))
        strcpy(period_str, "measurement");
      else if (TASessionManager_isAllPeriod(self, TASession_RAMPDOWN))
        strcpy(period_str, "rampdown");

      sprintf(response_body, "{period:%s}", period_str);
      return TANet_OK;
    }
    else
    {
      strcpy(response_body, errorbody);
      return TANet_NOT_FOUND;
    }
    break;
  case TANet_POST:
    if (strcmp(path, "/period-interval") == 0)
    {
      timerclear(&start_time);
      gettimeofday(&start_time, (struct timezone *)0);

      strcpy(response_body, "");

      s = strstr(request_body, "{rampup_interval:");
      if (s == NULL)
        return TANet_BAD_REQUEST;

      s = s + strlen("{rampup_interval:");
      rampup_interval = (int) strtol(s, &end, 10);
      if (end == s || errno == ERANGE)
        return TANet_BAD_REQUEST;

      e = strstr(s, ",measurement_interval:");
      if (e == NULL)
        return TANet_BAD_REQUEST;

      s = e + strlen(",measurement_interval:");
      measurement_interval = (int) strtol(s, &end, 10);
      if (end == s || errno == ERANGE)
        return TANet_BAD_REQUEST;

      e = strstr(s, ",rampdown_interval:");
      if (e == NULL)
        return TANet_BAD_REQUEST;

      s = e + strlen(",rampdown_interval:");
      rampdown_interval = (int) strtol(s, &end, 10);
      if (end == s || errno == ERANGE)
        return TANet_BAD_REQUEST;

      for (i = 0; i < TASessionManager_numberOfSessions(self); i++)
      {
        TASession_setPeriodInterval(TASessionManager_sessions(self)[i], start_time,
                                    rampup_interval, measurement_interval,
                                    rampdown_interval);
        TASession_setStatus(TASessionManager_sessions(self)[i], TASession_RUNNING);
      }

      return TANet_OK;
    }
    else if (strcmp(path, "/stop") == 0)
    {
      strcpy(response_body, "");
      return TANet_SERVICE_UNAVAILABLE;
    }
    else
    {
      strcpy(response_body, errorbody);
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
