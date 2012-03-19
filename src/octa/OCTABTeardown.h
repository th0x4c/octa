/*
 * OCTABTeardown.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABTEARDOWN_H_
#define _OCTABTEARDOWN_H_

#include <stdlib.h> /* malloc exit free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memset strncpy */
#include <OC/OC.h>
#include "OCTAOption.h"
#include "OCTADDL.h"
#include "OCTABDropTable.h"

int OCTABTeardown_main(const OCTAOption *option);

#endif /* _OCTABTEARDOWN_H_ */
