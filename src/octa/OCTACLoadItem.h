/*
 * OCTACLoadItem.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/31/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADITEM_H_
#define _OCTACLOADITEM_H_

#include <stdio.h>   /* fprintf snprintf printf */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadItem_beforeTX(int i_id, void **inout);
int OCTACLoadItem_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                           void **inout, char *errmsg, size_t errmsgsize);
void OCTACLoadItem_afterTX(void **inout);

#endif /* _OCTACLOADITEM_H_ */
