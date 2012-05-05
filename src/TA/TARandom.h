/*
 * TARandom.h
 * TA
 *
 * Created by Takashi Hashizume on 03/31/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _TARANDOM_H_
#define _TARANDOM_H_

#include <stdlib.h> /* srand48 lrand48 drand48 */
#include <time.h>   /* time */
#include <unistd.h> /* getpid */
#include <string.h> /* strlen */
#include "TABool.h"

long TARandom_rand();
double TARandom_drand();
long TARandom_number(long min, long max);
int TARandom_getString(char *str, int min, int max, const char *charset);
int TARandom_getAlphaString(char *str, int min, int max);
int TARandom_getNumberString(char *str, int min, int max);
int TARandom_indexInRatio(const int ratio[], int indexsize);

#endif /* _TARANDOM_H_ */
