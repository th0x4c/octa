/*
 * TATXStat.h
 * TA
 *
 * Created by Takashi Hashizume on 08/05/11.
 * Copyright 2011 Takashi Hashizume. All rights reserved.
 */

#ifndef _TATXSTAT_H_
#define _TATXSTAT_H_

#include <stdlib.h>   /* malloc free */
#include <string.h>   /* memset strcpy strncpy strlen */
#include <sys/time.h> /* gettimeofday */
#include <stdio.h>    /* snprintf */
#include "TABool.h"
#include "TATime.h"
#include "TADistribution.h"

typedef struct __TATXStat *TATXStat;

TATXStat TATXStat_init();
void TATXStat_release(TATXStat self);
size_t TATXStat_sizeof();
TATXStat TATXStat_nextAddr(TATXStat self);
void TATXStat_deepCopy(TATXStat self, TATXStat dest);
void TATXStat_setName(TATXStat self, const char *name);
char *TATXStat_name(TATXStat self);
unsigned int TATXStat_count(TATXStat self);
struct timeval TATXStat_firstTime(TATXStat self);
struct timeval TATXStat_startTime(TATXStat self);
struct timeval TATXStat_endTime(TATXStat self);
struct timeval TATXStat_elapsedTime(TATXStat self);
struct timeval TATXStat_totalElapsedTime(TATXStat self);
struct timeval TATXStat_maxElapsedTime(TATXStat self);
struct timeval TATXStat_minElapsedTime(TATXStat self);
unsigned int TATXStat_errorCount(TATXStat self);
void TATXStat_setError(TATXStat self, int errcode, const char *errmessage);
int TATXStat_errorCode(TATXStat self);
char *TATXStat_errorMessage(TATXStat self);
TADistribution TATXStat_distribution(TATXStat self);
void TATXStat_start(TATXStat self);
void TATXStat_end(TATXStat self);
struct timeval TATXStat_avgElapsedTime(TATXStat self);
double TATXStat_tps(TATXStat self);
char *TATXStat_description(TATXStat self, char *output, size_t outputsize);
TATXStat TATXStat_plus(TATXStat self, TATXStat txstat);
TATXStat TATXStat_minus(TATXStat self, TATXStat txstat);

#endif /* _TATXSTAT_H_ */
