/*
 * OCTABSetup.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABSETUP_H_
#define _OCTABSETUP_H_

#include <stdlib.h> /* malloc exit free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memset strncpy */
#include <OC/OC.h>
#include "OCTAOption.h"
#include "OCTADDL.h"
#include "OCTABCreateTable.h"
#include "OCTABCreateIndex.h"
#include "OCTABAddConstraint.h"
#include "OCTABAnalyze.h"
#include "OCTABLoad.h"

int OCTABSetup_main(const OCTAOption *option);

#endif /* _OCTABSETUP_H_ */
