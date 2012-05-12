/*
 * OCTACBenchOrderStatus.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/12/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCHORDERSTATUS_H_
#define _OCTACBENCHORDERSTATUS_H_

#include <stdio.h>  /* snprintf */
#include <string.h> /* memset */
#include <stdlib.h> /* atol */
#include <oci.h>    /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

struct OCTACBenchOrderStatusInput
{
  char w_id[40];    /* NUMBER */
  char d_id[3];     /* NUMBER(2, 0) */
  char c_last[17];  /* VARCHAR2(16) */
  char c_id[6];     /* NUMBER(5, 0) */
  TABool byname;
};
typedef struct OCTACBenchOrderStatusInput OCTACBenchOrderStatusInput;

struct OCTACBenchOrderStatusOutput
{
  char c_id[6];                /* NUMBER(5, 0) */
  char c_balance[14];          /* NUMBER(12, 2) */
  char c_first[17];            /* VARCHAR2(16) */
  char c_middle[3];            /* CHAR(2) */
  char c_last[17];             /* VARCHAR2(16) */
  char o_id[9];                /* NUMBER(8, 0) */
  char o_carrier_id[3];        /* NUMBER(2, 0) */
  char o_entry_d[20];          /* DATE */
  char ol_i_id[15][7];         /* NUMBER(6, 0) * [5 .. 15] */
  char ol_supply_w_id[15][40]; /* NUMBER * [5 .. 15] */
  char ol_quantity[15][3];     /* NUMBER(2, 0) * [5 .. 15] */
  char ol_amount[15][8];       /* NUMBER(6, 2) * [5 .. 15] */
  char ol_delivery_d[15][20];  /* DATE * [5 .. 15] */
};
typedef struct OCTACBenchOrderStatusOutput OCTACBenchOrderStatusOutput;

void OCTACBenchOrderStatus_beforeTX(OCTACBenchOrderStatusInput *input,
                                    int session_id, long scale_factor,
                                    struct timeval keying_time);
int OCTACBenchOrderStatus_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                   OCISvcCtx *svchp, void **inout,
                                   char *errmsg, size_t errmsgsize);
void OCTACBenchOrderStatus_afterTX(struct timeval think_time);

#endif /* _OCTACBENCHORDERSTATUS_H_ */
