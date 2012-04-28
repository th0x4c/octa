/*
 * OCTACLoadStock.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadStock.h"

struct OCTACLoadStockInput
{
  char s_i_id[7];       /* NUMBER(6, 0) */
  char s_w_id[40];      /* NUMBER */
  char s_quantity[5];   /* NUMBER(4, 0) */
  char s_dist_01[25];   /* CHAR(24) */
  char s_dist_02[25];   /* CHAR(24) */
  char s_dist_03[25];   /* CHAR(24) */
  char s_dist_04[25];   /* CHAR(24) */
  char s_dist_05[25];   /* CHAR(24) */
  char s_dist_06[25];   /* CHAR(24) */
  char s_dist_07[25];   /* CHAR(24) */
  char s_dist_08[25];   /* CHAR(24) */
  char s_dist_09[25];   /* CHAR(24) */
  char s_dist_10[25];   /* CHAR(24) */
  char s_data[51];      /* VARCHAR2(50) */
};
typedef struct OCTACLoadStockInput OCTACLoadStockInput;

void OCTACLoadStock_beforeTX(int s_i_id, int s_w_id, void **inout)
{
  OCTACLoadStockInput *in;

  in = malloc(sizeof(OCTACLoadStockInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadStockInput\n");
    exit(1);
  }

  snprintf(in->s_i_id, sizeof(in->s_i_id), "%d", s_i_id);
  snprintf(in->s_w_id, sizeof(in->s_w_id), "%d", s_w_id);
  snprintf(in->s_quantity, sizeof(in->s_quantity), "%ld",
           TARandom_number(10, 100));
  TARandom_getAlphaString(in->s_dist_01, 24, 24);
  TARandom_getAlphaString(in->s_dist_02, 24, 24);
  TARandom_getAlphaString(in->s_dist_03, 24, 24);
  TARandom_getAlphaString(in->s_dist_04, 24, 24);
  TARandom_getAlphaString(in->s_dist_05, 24, 24);
  TARandom_getAlphaString(in->s_dist_06, 24, 24);
  TARandom_getAlphaString(in->s_dist_07, 24, 24);
  TARandom_getAlphaString(in->s_dist_08, 24, 24);
  TARandom_getAlphaString(in->s_dist_09, 24, 24);
  TARandom_getAlphaString(in->s_dist_10, 24, 24);
  TARandom_getAlphaString(in->s_data, 26, 50);
  if (TARandom_rand() % 10 == 0)
    OCTACConfig_insertOriginal(in->s_data);

  *inout = in;
}

int OCTACLoadStock_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                            void **inout, char *errmsg, size_t errmsgsize)
{
  OCTACLoadStockInput *in = (OCTACLoadStockInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  static char *insert_stock_sql =
    "INSERT INTO stock (s_i_id, s_w_id, s_quantity, s_dist_01, s_dist_02, "
    "s_dist_03, s_dist_04, s_dist_05, s_dist_06, s_dist_07, s_dist_08, "
    "s_dist_09, s_dist_10, s_data, s_ytd, s_order_cnt, s_remote_cnt) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9, :10, :11, :12, :13, :14, "
    "0, 0, 0)";
  static int num_inserts = 0;

  sql = OCSQL_initWithSQL(envhp, errhp, insert_stock_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->s_i_id,
                                     in->s_w_id,
                                     in->s_quantity,
                                     in->s_dist_01,
                                     in->s_dist_02,
                                     in->s_dist_03,
                                     in->s_dist_04,
                                     in->s_dist_05,
                                     in->s_dist_06,
                                     in->s_dist_07,
                                     in->s_dist_08,
                                     in->s_dist_09,
                                     in->s_dist_10,
                                     in->s_data));
  OCSQL_release(sql);
  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadStock_afterTX(void **inout)
{
  OCTACLoadStockInput *in = (OCTACLoadStockInput *)*inout;
  long s_i_id = atol(in->s_i_id);

  if (s_i_id % 5000 == 0)
    printf("%s\n", in->s_i_id);

  free(in);
}
