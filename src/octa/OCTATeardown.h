/*
 * OCTATeardown.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/30/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTATEARDOWN_H_
#define _OCTATEARDOWN_H_

#include <stdlib.h> /* malloc exit free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memset strncpy */
#include <OC/OC.h>
#include "OCTAOption.h"
#include "OCTADDL.h"

int OCTATeardown_main(const OCTAOption *option, OCTADDLInput drop_table);

#endif /* _OCTATEARDOWN_H_ */
