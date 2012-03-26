/*
 * TASession.c
 * TA
 *
 * Created by Takashi Hashizume on 03/03/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "TASession.h"

struct __TASession
{
  int deep_copied;
  int id;
  TALog log;
  int status;
  int period;
  int phase;
  void (*setup)(TASession self, void **inout);
  char *(*selectTX)(TASession self);
  void (*teardown)(TASession self, void **inout);
#define NUM_PERIOD 4
#define NUM_PHASE 4
#define MAX_TXS 16
  TATXStat tx_stats[MAX_TXS][NUM_PERIOD][NUM_PHASE];
  void (*beforeTXs[MAX_TXS])(TASession self, void **inout);
  int (*TXs[MAX_TXS])(TASession self, void **inout);
  void (*afterTXs[MAX_TXS])(TASession self, void **inout);
  void (*when_errorTXs[MAX_TXS])(TASession self, void **inout, int errcode,
                                 char *errmessage, size_t errmessagesize);
  int tx_count;
};

#define INVALID_INDEX -1

static volatile sig_atomic_t sigflag = 0;

static int TASession_TXIndexByName(TASession self, const char *tx_name);
static void TASession_signalHandler(int sig);

TASession TASession_init()
{
  struct __TASession *self = malloc(sizeof(struct __TASession));
  int i = 0;
  int j = 0;
  int k = 0;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->deep_copied = FALSE;
  self->id = 0;
  self->log = TALog_initWithFilename(NULL);

  self->status = TASession_INIT;
  self->period = TASession_MEASUREMENT;
  self->phase = TASession_BEFORE;
  self->setup = NULL;
  self->selectTX = NULL;
  self->teardown = NULL;
  for (i = 0; i < MAX_TXS; i++)
  {
    for (j = 0; j < NUM_PERIOD; j++)
    {
      for (k = 0; k < NUM_PHASE; k++)
      {
        self->tx_stats[i][j][k] = TATXStat_init();        
      }
    }
    self->beforeTXs[i] = NULL;
    self->TXs[i] = NULL;
    self->afterTXs[i] = NULL;
    self->when_errorTXs[i] = NULL;
  }
  self->tx_count = 0;

  return self;
}

void TASession_release(TASession self)
{
  int i = 0;
  int j = 0;
  int k = 0;

  TALog_release(self->log);
  for (i = 0; i < MAX_TXS; i++)
  {
    for (j = 0; j < NUM_PERIOD; j++)
    {
      for (k = 0; k < NUM_PHASE; k++)
      {
        TATXStat_release(self->tx_stats[i][j][k]);
      }
    }
  }
  if (self->deep_copied == FALSE)
    free(self);
}

size_t TASession_sizeof()
{
  size_t ret = 0;

  ret = sizeof(struct __TASession);
  ret += TALog_sizeof();
  ret += TATXStat_sizeof() * MAX_TXS * NUM_PERIOD * NUM_PHASE;

  return ret;
}

TASession TASession_nextAddr(TASession self)
{
  TASession ret = NULL;
  int i = 0;

  ret = self + 1;
  ret = (TASession) TALog_nextAddr((TALog)ret);
  for (i = 0; i < MAX_TXS * NUM_PERIOD * NUM_PHASE; i++)
  {
    ret = (TASession) TATXStat_nextAddr((TATXStat)ret);
  }

  return ret;
}

void TASession_deepCopy(TASession self, TASession dest)
{
  TATXStat txstat = NULL;
  int i = 0;
  int j = 0;
  int k = 0;

  dest->deep_copied = TRUE;
  dest->id = self->id;
  dest->log = (TALog) (dest + 1);
  TALog_deepCopy(self->log, dest->log);
  dest->status = self->status;
  dest->period = self->period;
  dest->phase = self->phase;
  dest->setup = self->setup;
  dest->selectTX = self->selectTX;
  dest->teardown = self->teardown;
  txstat = (TATXStat) TALog_nextAddr(dest->log);
  for (i = 0; i < MAX_TXS; i++)
  {
    for (j = 0; j < NUM_PERIOD; j++)
    {
      for (k = 0; k < NUM_PHASE; k++)
      {
        if (i != 0 || j != 0 || k != 0)
          txstat = (TATXStat) TATXStat_nextAddr(txstat);

        dest->tx_stats[i][j][k] = txstat;
        TATXStat_deepCopy(self->tx_stats[i][j][k], dest->tx_stats[i][j][k]);
      }
    }
    dest->beforeTXs[i] = self->beforeTXs[i];
    dest->TXs[i] = self->TXs[i];
    dest->afterTXs[i] = self->afterTXs[i];
    dest->when_errorTXs[i] = self->when_errorTXs[i];
  }
  dest->tx_count = self->tx_count;
}

void TASession_setID(TASession self, int id)
{
  self->id = id;
}

int TASession_ID(TASession self)
{
  return self->id;
}

void TASession_setLog(TASession self, TALog log)
{
  /* comment out next line to avoid "pointer being freed was not allocated" */
  /* TALog_release(self->log); */

  self->log = log;
}

TALog TASession_log(TASession self)
{
  return self->log;
}

void TASession_setStatus(TASession self, int status)
{
#define NUM_STATUS 6
  char *status_strs[NUM_STATUS] = { "", "INIT", "STANDBY", "RUNNING",
                                    "STOP", "TERM" };
#define MSG_SIZE 64
  char msg[MSG_SIZE] = "";

  if (self->status == TASession_TERM)
    return;

  if (status == TASession_STANDBY ||
      status == TASession_RUNNING ||
      status == TASession_STOP)
  {
    snprintf(msg, MSG_SIZE, "sid: %d, msg: \"status %s -> %s\"",
             self->id, status_strs[self->status], status_strs[status]);
    TALog_info(self->log, msg);
    self->status = status;
  }
}

int TASession_status(TASession self)
{
  return self->status;
}

void TASession_toggleStatus(TASession self)
{
  switch (self->status)
  {
  case TASession_STANDBY:
    TASession_setStatus(self, TASession_RUNNING);
    break;
  case TASession_RUNNING:
    TASession_setStatus(self, TASession_STANDBY);
    break;
  default:
    break;
  }
}

void TASession_setPeriod(TASession self, int period)
{
  char *period_strs[NUM_PERIOD] = { "", "RAMPUP", "MEASUREMENT", "RAMPDOWN" };
#define MSG_SIZE 64
  char msg[MSG_SIZE] = "";

  if (period == TASession_RAMPUP ||
      period == TASession_MEASUREMENT ||
      period == TASession_RAMPDOWN)
  {
    snprintf(msg, MSG_SIZE, "sid: %d, msg: \"period %s -> %s\"",
             self->id, period_strs[self->period], period_strs[period]);
    TALog_info(self->log, msg);
    self->period = period;
  }
}

int TASession_period(TASession self)
{
  return self->period;
}

void TASession_movePeriod(TASession self)
{
  switch (self->period)
  {
  case TASession_RAMPUP:
    TASession_setPeriod(self, TASession_MEASUREMENT);
    break;
  case TASession_MEASUREMENT:
    TASession_setPeriod(self, TASession_RAMPDOWN);
    break;
  case TASession_RAMPDOWN:
    TASession_setStatus(self, TASession_STOP);
    break;
  default:
    break;
  }
}

void TASession_setSetup(TASession self,
                        void (*setup)(TASession self, void **inout))
{
  self->setup = setup;
}

void TASession_setSelectTX(TASession self, char *(*selectTX)(TASession self))
{
  self->selectTX = selectTX;
}

void TASession_setTeardown(TASession self,
                           void (*teardown)(TASession self, void **inout))
{
  self->teardown = teardown;
}

void TASession_setTX(TASession self, int (*TX)(TASession self, void **inout),
                     const char *tx_name)
{
  int i = 0;
  int j = 0;

  for (i = 0; i < NUM_PERIOD; i++)
  {
    for (j = 0; j < NUM_PHASE; j++)
    {
      TATXStat_setName(self->tx_stats[self->tx_count][i][j], tx_name);
    }
  }
  self->TXs[self->tx_count] = TX;
  self->tx_count += 1;
}

void TASession_setBeforeTX(TASession self,
                           void (*beforeTX)(TASession self, void **inout),
                           const char *tx_name)
{
  int tx_idx = TASession_TXIndexByName(self, tx_name);
  self->beforeTXs[tx_idx] = beforeTX;
}

void TASession_setAfterTX(TASession self,
                          void (*afterTX)(TASession self, void **inout),
                          const char *tx_name)
{
  int tx_idx = TASession_TXIndexByName(self, tx_name);
  self->afterTXs[tx_idx] = afterTX;
}

void TASession_setWhenErrorTX(TASession self,
                              void (*when_errorTX)(TASession self,
                                                   void **inout,
                                                   int errcode,
                                                   char *errmessage,
                                                   size_t errmessagesize),
                              const char *tx_name)
{
  int tx_idx = TASession_TXIndexByName(self, tx_name);
  self->when_errorTXs[tx_idx] = when_errorTX;
}

TATXStat TASession_statByNameInPeriodInPhase(TASession self,
                                             const char *tx_name,
                                             int period, int phase)
{
  int tx_idx = TASession_TXIndexByName(self, tx_name);
  TATXStat ret = NULL;

  if ((period == TASession_RAMPUP ||
       period == TASession_MEASUREMENT ||
       period == TASession_RAMPDOWN) &&
      (phase == TASession_BEFORE ||
       phase == TASession_TX ||
       phase == TASession_AFTER))
  {
    ret = self->tx_stats[tx_idx][period][phase]; 
  }
  return ret;
}

TATXStat TASession_currentStatByName(TASession self, const char *tx_name)
{
  return TASession_statByNameInPeriodInPhase(self, tx_name, self->period,
                                             TASession_TX);
}

int TASession_main(TASession self, void **inout)
{
  TATXStat txstat = NULL;
  int tx_idx = INVALID_INDEX;
  int error_code = 0;
#define MAX_MSG_SIZE 256
  char error_message[MAX_MSG_SIZE] = "";
  struct timespec sleeptp;
  struct sigaction act, oldact;

  sleeptp.tv_sec = 0;
  sleeptp.tv_nsec = 10000000; /* 10ms */

  /* signal handler */
  act.sa_handler = TASession_signalHandler;
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

  if (self->setup)
    self->setup(self, inout);

  if (self->status == TASession_INIT)
    self->status = TASession_STANDBY;

  while (self->status == TASession_STANDBY ||
         self->status == TASession_RUNNING)
  {
    switch (sigflag)
    {
    case SIGINT:
      /* ignore SIGINT */
      sigflag = 0;
      break;
    case SIGTERM:
      TASession_setStatus(self, TASession_STOP);
      sigflag = 0;
      break;
    case SIGUSR1:
      TASession_toggleStatus(self);
      sigflag = 0;
      break;
    case SIGUSR2:
      TASession_movePeriod(self);
      sigflag = 0;
      break;
    default:
      break;
    }

    switch (self->status)
    {
    case TASession_STANDBY:
      nanosleep(&sleeptp, NULL);
      break;
    case TASession_RUNNING:
      tx_idx = TASession_TXIndexByName(self, self->selectTX(self));
      if (tx_idx == INVALID_INDEX)
        self->status = TASession_STOP;

      if (self->status != TASession_RUNNING)
        break;

      self->phase = TASession_BEFORE;
      if (self->beforeTXs[tx_idx])
      {
        txstat = self->tx_stats[tx_idx][self->period][self->phase];
        TATXStat_start(txstat);
        self->beforeTXs[tx_idx](self, inout);
        TATXStat_end(txstat);
      }

      self->phase = TASession_TX;
      if (self->TXs[tx_idx])
      {
        txstat = self->tx_stats[tx_idx][self->period][self->phase];
        TATXStat_start(txstat);
        error_code = self->TXs[tx_idx](self, inout);
        TATXStat_end(txstat);
      }
      if (error_code)
      {
        strcpy(error_message, "");
        if (self->when_errorTXs[tx_idx])
        {
          self->when_errorTXs[tx_idx](self, inout, error_code, error_message,
                                      MAX_MSG_SIZE);
          error_message[MAX_MSG_SIZE - 1] = '\0';
        }
        TATXStat_setError(txstat, error_code, error_message);
        if (strcmp(error_message, "") != 0)
          TALog_error(self->log, error_message);
      }

      self->phase = TASession_AFTER;
      if (self->afterTXs[tx_idx])
      {
        txstat = self->tx_stats[tx_idx][self->period][self->phase];
        TATXStat_start(txstat);
        self->afterTXs[tx_idx](self, inout);
        TATXStat_end(txstat);
      }
      break;
    default:
      break;
    }
  }

  if (self->teardown)
    self->teardown(self, inout);

  self->status = TASession_TERM;

  return 0;
}

/* private */
static int TASession_TXIndexByName(TASession self, const char *tx_name)
{
  int ret = INVALID_INDEX;
  int i = 0;

  for (i = 0; i < MAX_TXS; i++)
  {
    if (! strcmp(TATXStat_name(
                   self->tx_stats[i][TASession_MEASUREMENT][TASession_TX]),
                 tx_name))
    {
      ret = i;
      break;
    }
  }

  if (ret == INVALID_INDEX)
    fprintf(stderr, "TX name: %s is invalid.", tx_name);

  return ret;
}

static void TASession_signalHandler(int sig)
{
  sigflag = sig;
}
