/*
 * OCTACLoadDistrict.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADDISTRICT_H_
#define _OCTACLOADDISTRICT_H_

#include <stdio.h>   /* fprintf snprintf printf */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadDistrict_beforeTX(int d_id, int d_w_id, void **inout);
int OCTACLoadDistrict_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize);
void OCTACLoadDistrict_afterTX(void **inout);

#endif /* _OCTACLOADDISTRICT_H_ */
