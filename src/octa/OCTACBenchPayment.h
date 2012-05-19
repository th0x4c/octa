/*
 * OCTACBenchPayment.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/12/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCHPAYMENT_H_
#define _OCTACBENCHPAYMENT_H_

#include <stdio.h>  /* snprintf */
#include <string.h> /* memset strstr strncat strlen */
#include <stdlib.h> /* atol */
#include <oci.h>    /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

struct OCTACBenchPaymentInput
{
  char w_id[40];    /* NUMBER */
  char d_id[3];     /* NUMBER(2, 0) */
  char c_d_id[3];   /* NUMBER(2, 0) */
  char c_w_id[40];  /* NUMBER */
  char c_last[17];  /* VARCHAR2(16) */
  char c_id[6];     /* NUMBER(5, 0) */
  TABool byname;
  char h_amount[8]; /* NUMBER(6, 2) */
};
typedef struct OCTACBenchPaymentInput OCTACBenchPaymentInput;

struct OCTACBenchPaymentOutput
{
  char w_street_1[21];   /* VARCHAR2(20) */
  char w_street_2[21];   /* VARCHAR2(20) */
  char w_city[21];       /* VARCHAR2(20) */
  char w_state[3];       /* CHAR(2) */
  char w_zip[10];        /* CHAR(9) */
  char w_name[11];       /* VARCHAR2(10) */
  char d_street_1[21];   /* VARCHAR2(20) */
  char d_street_2[21];   /* VARCHAR2(20) */
  char d_city[21];       /* VARCHAR2(20) */
  char d_state[3];       /* CHAR(2) */
  char d_zip[10];        /* CHAR(9) */
  char d_name[11];       /* VARCHAR2(10) */
  char c_id[6];          /* NUMBER(5, 0) */
  char c_first[17];      /* VARCHAR2(16) */
  char c_middle[3];      /* CHAR(2) */
  char c_last[17];       /* VARCHAR2(16) */
  char c_street_1[21];   /* VARCHAR2(20) */
  char c_street_2[21];   /* VARCHAR2(20) */
  char c_city[21];       /* VARCHAR2(20) */
  char c_state[3];       /* CHAR(2) */
  char c_zip[10];        /* CHAR(9) */
  char c_phone[17];      /* CHAR(16) */
  char c_credit[3];      /* CHAR(2) */
  char c_credit_lim[14]; /* NUMBER(12, 2) */
  char c_discount[7];    /* NUMBER(4, 4) */
  char c_balance[14];    /* NUMBER(12, 2) */
  char c_since[20];      /* DATE */
  char c_data[501];      /* VARCHAR2(500) */
  char c_new_data[501];  /* VARCHAR2(500) */
  char h_data[25];       /* VARCHAR2(24) */
};
typedef struct OCTACBenchPaymentOutput OCTACBenchPaymentOutput;

struct OCTACBenchPaymentInOut
{
  OCTACBenchPaymentInput input;
  OCTACBenchPaymentOutput output;
};
typedef struct OCTACBenchPaymentInOut OCTACBenchPaymentInOut;

void OCTACBenchPayment_beforeTX(OCTACBenchPaymentInput *input,
                                int session_id, long scale_factor,
                                struct timeval keying_time);
int OCTACBenchPayment_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize);
void OCTACBenchPayment_afterTX(OCTACBenchPaymentInOut *inout,
                               struct timeval think_time);

#endif /* _OCTACBENCHPAYMENT_H_ */
