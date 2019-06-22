/*
 * TADistribution.h
 * TA
 *
 * Created by Takashi Hashizume on 03/25/12.
 * Copyright 2012 Takashi hashizume. All rights reserved.
 */

#ifndef _TADISTRIBUTION_H_
#define _TADISTRIBUTION_H_

#include <stdlib.h>   /* malloc free strtol strtoul */
#include <string.h>   /* memset strstr strlen */
#include <stdio.h>    /* printf snprintf */
#include <errno.h>    /* errno ERANGE */
#include <sys/time.h>
#include <limits.h>   /* INT_MAX UINT_MAX */
#include <math.h>     /* log10 */
#include "TABool.h"
#include "TATime.h"

typedef struct __TADistribution *TADistribution;

TADistribution TADistribution_init();
TADistribution TADistribution_initWithJSON(const char *json);
void TADistribution_release(TADistribution self);
size_t TADistribution_sizeof();
TADistribution TADistribution_nextAddr(TADistribution self);
void TADistribution_deepCopy(TADistribution self, TADistribution dest);
void TADistribution_setElapsedTime(TADistribution self, struct timeval elaps);
struct timeval TADistribution_percentile(TADistribution self, int percent);
void TADistribution_print(TADistribution self);
TADistribution TADistribution_plus(TADistribution self, TADistribution tadist);
TADistribution TADistribution_minus(TADistribution self,
                                    TADistribution tadist);
char *TADistribution_JSON(TADistribution self, char *output, size_t outputsize);
size_t TADistribution_JSONMaxLength();

#endif /* _TADISTRIBUTION_H_ */
