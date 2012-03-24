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
