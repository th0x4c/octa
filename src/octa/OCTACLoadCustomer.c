/*
 * OCTACLoadCustomer.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/08/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadCustomer.h"

struct OCTACLoadCustomerInput
{
  char c_id[6];          /* NUMBER(5, 0) */
  char c_d_id[3];        /* NUMBER(2, 0) */
  char c_w_id[40];       /* NUMBER */
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
  char c_data[501];      /* VARCHAR2(500) */
  char h_amount[8];      /* NUMBER(6, 2) */
  char h_data[25];       /* VARCHAR2(24) */
};
typedef struct OCTACLoadCustomerInput OCTACLoadCustomerInput;

void OCTACLoadCustomer_beforeTX(long c_id, long c_d_id, long c_w_id,
                                void **inout)
{
  OCTACLoadCustomerInput *in;

  in = malloc(sizeof(OCTACLoadCustomerInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadCustomerInput\n");
    exit(1);
  }

  snprintf(in->c_id, sizeof(in->c_id), "%ld", c_id);
  snprintf(in->c_d_id, sizeof(in->c_d_id), "%ld", c_d_id);
  snprintf(in->c_w_id, sizeof(in->c_w_id), "%ld", c_w_id);
  TARandom_getAlphaString(in->c_first, 8, 16);
  snprintf(in->c_middle, sizeof(in->c_middle), "%s", "OE");
  if (c_id <= 1000)
    OCTACConfig_lastname(c_id - 1, in->c_last);
  else
    OCTACConfig_lastname(OCTACConfig_NURand(255, 0, 999), in->c_last);

  OCTACConfig_makeAddress(in->c_street_1, in->c_street_2, in->c_city,
                          in->c_state, in->c_zip);
  TARandom_getNumberString(in->c_phone, 16, 16);
  /*
   * C_CREDIT = "GC". For 10% of the rows, selected at random , C_CREDIT = "BC"
   */
  snprintf(in->c_credit, sizeof(in->c_credit), "%s", "GC");
  if (TARandom_rand() % 10 == 0)
    in->c_credit[0] = 'B';

  snprintf(in->c_credit_lim, sizeof(in->c_credit_lim), "%.2f", 50000.00);
  snprintf(in->c_discount, sizeof(in->c_discount), "%.2f",
           TARandom_number(0, 50) / 100.0);
  snprintf(in->c_balance, sizeof(in->c_balance), "%.2f", -10.0);
  TARandom_getAlphaString(in->c_data, 300, 500);
  snprintf(in->h_amount, sizeof(in->h_amount), "%.2f", 10.0);
  TARandom_getAlphaString(in->h_data, 12, 24);

  *inout = in;
}

int OCTACLoadCustomer_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize)
{
  OCTACLoadCustomerInput *in = (OCTACLoadCustomerInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  static char *insert_customer_sql =
    "INSERT INTO customer (c_id, c_d_id, c_w_id, c_first, c_middle, c_last, "
    "c_street_1, c_street_2, c_city, c_state, c_zip, c_phone, c_since, "
    "c_credit, c_credit_lim, c_discount, c_balance, c_data, c_ytd_payment, "
    "c_payment_cnt, c_delivery_cnt) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9, :10, :11, :12, SYSDATE, :13, "
    ":14, :15, :16, :17, 10.0, 1, 0)";
  static char *insert_history_sql =
    "INSERT INTO history (h_c_id, h_c_d_id, h_c_w_id, h_w_id, h_d_id, h_date, "
    "h_amount, h_data) "
    "VALUES (:1, :2, :3, :4, :5, SYSDATE, :6, :7)";
  static long num_inserts = 0;

  /* customer */
  sql = OCSQL_initWithSQL(envhp, errhp, insert_customer_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->c_id,
                                     in->c_d_id,
                                     in->c_w_id,
                                     in->c_first,
                                     in->c_middle,
                                     in->c_last,
                                     in->c_street_1,
                                     in->c_street_2,
                                     in->c_city,
                                     in->c_state,
                                     in->c_zip,
                                     in->c_phone,
                                     in->c_credit,
                                     in->c_credit_lim,
                                     in->c_discount,
                                     in->c_balance,
                                     in->c_data));
  OCSQL_release(sql);
  if (errcode != 0)
    return errcode;

  /* history */
  sql = OCSQL_initWithSQL(envhp, errhp, insert_history_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->c_id,
                                     in->c_d_id,
                                     in->c_w_id,
                                     in->c_w_id,
                                     in->c_d_id,
                                     in->h_amount,
                                     in->h_data));
  OCSQL_release(sql);
  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadCustomer_afterTX(void **inout)
{
  OCTACLoadCustomerInput *in = (OCTACLoadCustomerInput *)*inout;
  long c_id = atol(in->c_id);

  if (c_id % 1000 == 0)
    printf("%s\n", in->c_id);

  free(in);
}
