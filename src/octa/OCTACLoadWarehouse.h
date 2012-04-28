/*
 * OCTACLoadWarehouse.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADWAREHOUSE_H_
#define _OCTACLOADWAREHOUSE_H_

#include <stdio.h>   /* fprintf snprintf */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadWarehouse_beforeTX(long w_id, void **inout);
int OCTACLoadWarehouse_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize);
void OCTACLoadWarehouse_afterTX(void **inout);

#endif /* _OCTACLOADWAREHOUSE_H_ */
