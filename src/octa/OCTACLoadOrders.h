/*
 * OCTACLoadOrders.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADORDERS_H_
#define _OCTACLOADORDERS_H_

#include <stdio.h>   /* fprintf snprintf */
#include <stdlib.h>  /* exit malloc free atol */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadOrders_beforeTX(long o_id, long o_d_id, long o_w_id, long o_c_id,
                              void **inout);
int OCTACLoadOrders_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                             void **inout, char *errmsg, size_t errmsgsize);
void OCTACLoadOrders_afterTX(void **inout);

#endif /* _OCTACLOADORDERS_H_ */
