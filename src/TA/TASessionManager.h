/*
 * TASessionManager.h
 * TA
 *
 * Created by Takashi Hashizume on 03/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _TASESSIONMANAGER_H_
#define _TASESSIONMANAGER_H_

#include <stdlib.h>    /* malloc free fprintf stderr strerror */
#include <string.h>    /* memset */
#include <stdio.h>     /* printf fflush */
#include <sys/shm.h>   /* shmget shmat shmdt shmctl */
#include <stdlib.h>    /* exit */
#include <errno.h>     /* errno */
#include <unistd.h>    /* fork */
#include <time.h>      /* nanosleep */
#include <sys/types.h> /* wait */
#include <sys/wait.h>  /* wait */
#include <signal.h>    /* sigemptyset sigaction */
#include "TABool.h"
#include "TATime.h"
#include "TATXStat.h"
#include "TASession.h"

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
TATXStat TASessionManager_summaryStatByNameInPeriodInPhase(
           TASessionManager self,
           const char *tx_name,
           int period, int phase);
void TASessionManager_printNumericalQuantitiesSummary(TASessionManager self,
                                                      char *tx_names[],
                                                      int tx_count);
int TASessionManager_main(TASessionManager self, void **inout);

#endif /* _TASESSIONMANAGER_H_ */
