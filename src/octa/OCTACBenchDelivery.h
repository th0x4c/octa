/*
 * OCTACBenchDelivery.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCHDELIVERY_H_
#define _OCTACBENCHDELIVERY_H_

#include <stdio.h>    /* snprintf */
#include <string.h>   /* memset */
#include <sys/time.h> /* gettimeofday */
#include <oci.h>      /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

struct OCTACBenchDeliveryInput
{
  char w_id[40];        /* NUMBER */
  char o_carrier_id[3]; /* NUMBER(2, 0) */
  TALog log;
  TABool select_only;
};
typedef struct OCTACBenchDeliveryInput OCTACBenchDeliveryInput;

struct OCTACBenchDeliveryOutput
{
  char d_id[3];      /* NUMBER(2, 0) */
  char no_o_id[9];   /* NUMBER(8, 0) */
  char c_id[6];      /* NUMBER(5, 0) */
  char ol_total[10]; /* NUMBER(6, 2) * [5 .. 15] */
};
typedef struct OCTACBenchDeliveryOutput OCTACBenchDeliveryOutput;

struct OCTACBenchDeliveryInOut
{
  OCTACBenchDeliveryInput input;
  OCTACBenchDeliveryOutput output;
};
typedef struct OCTACBenchDeliveryInOut OCTACBenchDeliveryInOut;

void OCTACBenchDelivery_beforeTX(OCTACBenchDeliveryInput *input,
                                 int session_id, long scale_factor,
                                 struct timeval keying_time);
int OCTACBenchDelivery_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout,
                                char *errmsg, size_t errmsgsize);
void OCTACBenchDelivery_afterTX(OCTACBenchDeliveryInOut *inout,
                                struct timeval think_time);

#endif /* _OCTACBENCHDELIVERY_H_ */
