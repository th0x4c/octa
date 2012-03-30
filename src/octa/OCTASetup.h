/*
 * OCTASetup.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/30/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTASETUP_H_
#define _OCTASETUP_H_

#include <stdlib.h> /* malloc exit free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memset strncpy */
#include <OC/OC.h>
#include "OCTAOption.h"
#include "OCTADDL.h"

int OCTASetup_main(const OCTAOption *option, OCTADDLInput create_table,
                   int (*load)(const OCTAOption *opt),
                   OCTADDLInput create_index, OCTADDLInput add_constraint,
                   OCTADDLInput analyze);

#endif /* _OCTASETUP_H_ */
