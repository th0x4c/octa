/*
 * OCTACBenchStockLevel.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCHSTOCKLEVEL_H_
#define _OCTACBENCHSTOCKLEVEL_H_

#include <stdio.h>  /* snprintf */
#include <string.h> /* memset */
#include <oci.h>    /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

struct OCTACBenchStockLevelInput
{
  char w_id[40];     /* NUMBER */
  char d_id[3];      /* NUMBER(2, 0) */
  char threshold[3];
};
typedef struct OCTACBenchStockLevelInput OCTACBenchStockLevelInput;

struct OCTACBenchStockLevelOutput
{
  char o_id[9];        /* NUMBER(8, 0) */
  char stock_count[7]; /* NUMBER(6, 0) */
};
typedef struct OCTACBenchStockLevelOutput OCTACBenchStockLevelOutput;

void OCTACBenchStockLevel_beforeTX(OCTACBenchStockLevelInput *input,
                                   int session_id, long scale_factor,
                                   struct timeval keying_time);
int OCTACBenchStockLevel_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                  OCISvcCtx *svchp, void **inout,
                                  char *errmsg, size_t errmsgsize);
void OCTACBenchStockLevel_afterTX(struct timeval think_time);

#endif /* _OCTACBENCHSTOCKLEVEL_H_ */
