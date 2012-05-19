/*
 * OCTACBench.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/05/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCH_H_
#define _OCTACBENCH_H_

#include <stdio.h>    /* printf snprintf fprintf */
#include <stdlib.h>   /* malloc exit free */
#include <string.h>   /* memset */
#include <time.h>     /* nanosleep */
#include <sys/time.h> /* gettimeofday */
#include <oci.h>      /* OCITransRollback */
#include <OC/OC.h>
#include <TA/TA.h>
#include "OCTAOption.h"
#include "OCTACConfig.h"
#include "OCTACBenchNewOrder.h"
#include "OCTACBenchPayment.h"
#include "OCTACBenchOrderStatus.h"
#include "OCTACBenchDelivery.h"
/* #include "OCTACBenchStockLevel.h" */

int OCTACBench_main(const OCTAOption *opt);

#endif /* _OCTACBENCH_H_ */
