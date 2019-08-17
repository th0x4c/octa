/*
 * TASessionManager.h
 * TA
 *
 * Created by Takashi Hashizume on 03/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _TASESSIONMANAGER_H_
#define _TASESSIONMANAGER_H_

#include <stdlib.h>    /* malloc free fprintf stderr strerror strtol */
#include <string.h>    /* memset strstr strchr strcmp strncmp strcpy */
#include <stdio.h>     /* sprintf printf fflush */
#include <sys/shm.h>   /* shmget shmat shmdt shmctl */
#include <stdlib.h>    /* exit */
#include <errno.h>     /* errno */
#include <unistd.h>    /* fork */
#include <time.h>      /* nanosleep */
#include <sys/time.h>  /* gettimeofday */
#include <sys/types.h> /* waitpid */
#include <sys/wait.h>  /* waitpid */
#include <signal.h>    /* sigemptyset sigaction kill */
#include "TABool.h"
#include "TATime.h"
#include "TATXStat.h"
#include "TASession.h"
#include "TANet.h"

typedef struct __TASessionManager *TASessionManager;

TASessionManager TASessionManager_initWithSessionPrototype(TASession prototype,
                                                           int num_sessions);
void TASessionManager_release(TASessionManager self);
int TASessionManager_numberOfSessions(TASessionManager self);
TASession *TASessionManager_sessions(TASessionManager self);
void TASessionManager_setBeforeSetup(TASessionManager self,
                                     void (*beforeSetup)(TASessionManager self,
                                                         void **inout));
void TASessionManager_setAfterTeardown(TASessionManager self,
                                       void (*afterTeardown)(
                                                TASessionManager self,
                                                void **inout));
void TASessionManager_setMonitor(TASessionManager self,
                                 void (*monitor)(TASessionManager self));
void TASessionManager_setPort(TASessionManager self, unsigned short port);
void TASessionManager_setURL(TASessionManager self, const char *url);
TATXStat TASessionManager_summaryStatByNameInPeriodInPhase(
           TASessionManager self,
           const char *tx_name,
           int period, int phase);
void TASessionManager_printMonitoredTX(TASessionManager self,
                                       const char *tx_name, int pagesize,
                                       TABool long_format);
void TASessionManager_printNumericalQuantitiesSummary(TASessionManager self,
                                                      char *tx_names[],
                                                      int tx_count);
int TASessionManager_main(TASessionManager self, void **inout);

#endif /* _TASESSIONMANAGER_H_ */
