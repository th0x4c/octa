/*
 * TADistribution.h
 * TA
 *
 * Created by Takashi Hashizume on 03/25/12.
 * Copyright 2012 Takashi hashizume. All rights reserved.
 */

#ifndef _TADISTRIBUTION_H_
#define _TADISTRIBUTION_H_

#include <stdlib.h>   /* malloc free */
#include <string.h>   /* memset */
#include <stdio.h>    /* printf snprintf */
#include <sys/time.h>
#include "TABool.h"
#include "TATime.h"

typedef struct __TADistribution *TADistribution;

TADistribution TADistribution_init();
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

#endif /* _TADISTRIBUTION_H_ */
