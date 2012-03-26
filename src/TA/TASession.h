/*
 * TASession.h
 * TA
 *
 * Created by Takashi Hashizume on 03/03/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _TASESSION_H_
#define _TASESSION_H_

#include <stdlib.h> /* malloc free */
#include <string.h> /* memset strcpy strcmp */
#include <stdio.h>  /* snprintf fprintf */
#include <time.h>   /* nanosleep */
#include <signal.h> /* sigemptyset sigaction */
#include "TABool.h"
#include "TALog.h"
#include "TATXStat.h"

typedef struct __TASession *TASession;

/* status */
#define TASession_INIT    1
#define TASession_STANDBY 2
#define TASession_RUNNING 3
#define TASession_STOP    4
#define TASession_TERM    5

/* period */
#define TASession_RAMPUP      1
#define TASession_MEASUREMENT 2
#define TASession_RAMPDOWN    3

/* phase */
#define TASession_BEFORE 1
#define TASession_TX     2
#define TASession_AFTER  3

TASession TASession_init();
void TASession_release(TASession self);
size_t TASession_sizeof();
TASession TASession_nextAddr(TASession self);
void TASession_deepCopy(TASession self, TASession dest);
void TASession_setID(TASession self, int id);
int TASession_ID(TASession self);
void TASession_setLog(TASession self, TALog log);
TALog TASession_log(TASession self);
void TASession_setStatus(TASession self, int status);
int TASession_status(TASession self);
void TASession_toggleStatus(TASession self);
void TASession_setPeriod(TASession self, int period);
int TASession_period(TASession self);
void TASession_movePeriod(TASession self);
void TASession_setSetup(TASession self,
                        void (*setup)(TASession self, void **inout));
void TASession_setSelectTX(TASession self, char *(*selectTX)(TASession self));
void TASession_setTeardown(TASession self,
                           void (*teardown)(TASession self, void **inout));
void TASession_setTX(TASession self, int (*TX)(TASession self, void **inout),
                     const char *tx_name);
void TASession_setBeforeTX(TASession self,
                           void (*beforeTX)(TASession self, void **inout),
                           const char *tx_name);
void TASession_setAfterTX(TASession self,
                          void (*afterTX)(TASession self, void **inout),
                          const char *tx_name);

void TASession_setWhenErrorTX(TASession self,
                              void (*when_errorTX)(TASession self,
                                                   void **inout,
                                                   int errcode,
                                                   char *errmessage,
                                                   size_t errmessagesize),
                              const char *tx_name);
TATXStat TASession_statByNameInPeriodInPhase(TASession self,
                                             const char *tx_name,
                                             int period, int phase);
TATXStat TASession_currentStatByName(TASession self, const char *tx_name);
int TASession_main(TASession self, void **inout);

#endif /* _TASESSION_H_ */
