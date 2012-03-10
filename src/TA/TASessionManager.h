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
#include <sys/shm.h>   /* shmget shmat shmdt shmctl */
#include <stdlib.h>    /* exit */
#include <errno.h>     /* errno */
#include <unistd.h>    /* fork */
#include <time.h>      /* nanosleep */
#include <sys/types.h> /* wait */
#include <sys/wait.h>  /* wait */
#include <signal.h>    /* sigemptyset sigaction */
#include "TABool.h"
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
int TASessionManager_main(TASessionManager self, void **inout);

#endif /* _TASESSIONMANAGER_H_ */
