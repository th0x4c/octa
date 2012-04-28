/*
 * OCTACLoadCustomer.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/08/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOADCUSTOMER_H_
#define _OCTACLOADCUSTOMER_H_

#include <stdio.h>   /* fprintf snprintf printf */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

void OCTACLoadCustomer_beforeTX(int c_id, int c_d_id, int c_w_id,
                                void **inout);
int OCTACLoadCustomer_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize);
void OCTACLoadCustomer_afterTX(void **inout);

#endif /* _OCTACLOADCUSTOMER_H_ */
