/*
 * OCTACLoadStock.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADSTOCK_H_
#define _OCTACLOADSTOCK_H_

#include <stdio.h>   /* fprintf snprintf printf */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadStock_beforeTX(long s_i_id, long s_w_id, void **inout);
int OCTACLoadStock_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                            void **inout, char *errmsg, size_t errmsgsize);
void OCTACLoadStock_afterTX(void **inout);

#endif /* _OCTACLOADSTOCK_H_ */
