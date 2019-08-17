/*
 * OCTACDist.h
 * OCTA
 *
 * Created by Takashi Hashizume on 08/17/19.
 * Copyright 2019 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACDIST_H_
#define _OCTACDIST_H_

#include <stdio.h>    /* printf snprintf fprintf */
#include <stdlib.h>   /* malloc exit free */
#include <string.h>   /* memset */
#include <time.h>     /* nanosleep */
#include <sys/time.h> /* gettimeofday */
#include <TA/TA.h>
#include "OCTAOption.h"

int OCTACDist_main(const OCTAOption *opt);

#endif /* _OCTACDIST_H_ */
