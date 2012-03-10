/*
 * TALog.h
 * TA
 *
 * Created by Takashi Hashizume on 08/04/11.
 * Copyright 2011 Takashi Hashizume. All rights reserved.
 */

#ifndef _TALOG_H_
#define _TALOG_H_

#include <stdlib.h>   /* malloc free */
#include <string.h>   /* memset strcpy */
#include <stdio.h>    /* FILENAME_MAX stdout fopen fclose fprintf fflush */
#include <sys/time.h> /* gettimeofday */
#include "TATime.h"

typedef struct __TALog *TALog;

#define TALog_OFF   0
#define TALog_FATAL 1
#define TALog_ERROR 2
#define TALog_WARN  3
#define TALog_INFO  4
#define TALog_DEBUG 5
#define TALog_ALL   10

TALog TALog_initWithFilename(const char *filename);
void TALog_release(TALog self);
size_t TALog_sizeof();
TALog TALog_nextAddr(TALog self);
void TALog_deepCopy(TALog self, TALog dest);
int TALog_level(TALog self);
void TALog_setLevel(TALog self, int level);
void TALog_fatal(TALog self, const char *str);
void TALog_error(TALog self, const char *str);
void TALog_warn(TALog self, const char *str);
void TALog_info(TALog self, const char *str);
void TALog_debug(TALog self, const char *str);

#endif /* _TALOG_H_ */
