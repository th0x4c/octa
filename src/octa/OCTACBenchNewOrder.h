/*
 * OCTACBenchNewOrder.h
 * OCTA
 *
 * Created by Takashi Hashizume on 05/05/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACBENCHNEWORDER_H_
#define _OCTACBENCHNEWORDER_H_

#include <stdio.h>  /* snprintf */
#include <time.h>   /* nanosleep */
#include <string.h> /* memset strlen strcat strstr */
#include <stdlib.h> /* atol atof */
#include <math.h>   /* log */
#include <oci.h>    /* OCITransCommit */
#include <TA/TA.h>
#include <OC/OC.h>
#include "OCTACConfig.h"

struct OCTACBenchNewOrderInput
{
  char w_id[40];        /* NUMBER */
  char d_id[3];         /* NUMBER(2, 0) */
  char c_id[6];         /* NUMBER(5, 0) */
  char o_ol_cnt[3];     /* NUMBER(2, 0) */
  char o_all_local[2];  /* NUMBER(1, 0) */
  char supware[15][40]; /* NUMBER * [5 .. 15] */
  char itemid[15][7];   /* NUMBER(6, 0) * [5 .. 15] */
  char qty[15][3];      /* NUMBER(2, 0) * [5 .. 15] */
};
typedef struct OCTACBenchNewOrderInput OCTACBenchNewOrderInput;

struct OCTACBenchNewOrderOutput
{
  char c_discount[7];     /* NUMBER(4, 4) */
  char c_last[17];        /* VARCHAR2(16) */
  char c_credit[3];       /* CHAR(2) */
  char w_tax[7];          /* NUMBER(4, 4) */
  char d_next_o_id[9];    /* NUMBER(8, 0) */
  char d_tax[7];          /* NUMBER(4, 4) */
  char price[15][7];      /* NUMBER(5, 2) * [5 .. 15] */
  char iname[15][25];     /* VARCHAR2(24) * [5 .. 15] */
  char i_data[51];        /* VARCHAR2(50) */
  char s_quantity[5];     /* NUMBER(4, 0) */
  char s_data[51];        /* VARCHAR2(50) */
  char s_dist_xx[10][25]; /* CHAR(24) * 10 */
  char ol_dist_info[25];  /* CHAR(24) */
  char stock[15][5];      /* NUMBER(4, 0) * [5 .. 15] */
  char bg[15];
  char ol_amount[8];      /* NUMBER(6, 2) */
  char amt[15][8];        /* NUMBER(6, 2) * [5 .. 15] */
  char total[10];         /* NUMBER(6, 2) * [5 .. 15] */
};
typedef struct OCTACBenchNewOrderOutput OCTACBenchNewOrderOutput;

void OCTACBenchNewOrder_beforeTX(OCTACBenchNewOrderInput *input,
                                 int session_id, long scale_factor,
                                 struct timeval keying_time);
int OCTACBenchNewOrder_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize);
void OCTACBenchNewOrder_afterTX(struct timeval think_time);

#endif /* _OCTACBENCHNEWORDER_H_ */
