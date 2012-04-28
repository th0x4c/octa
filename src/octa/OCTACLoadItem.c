/*
 * OCTACLoadItem.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/31/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadItem.h"

struct OCTACLoadItemInput
{
  char i_id[7];    /* NUMBER(6, 0) */
  char i_im_id[7]; /* NUMBER(6, 0) */
  char i_name[25]; /* VARCHAR2(24) */
  char i_price[7]; /* NUMBER(5, 2) */
  char i_data[51]; /* VARCHAR2(50) */
};
typedef struct OCTACLoadItemInput OCTACLoadItemInput;

void OCTACLoadItem_beforeTX(long i_id, void **inout)
{
  OCTACLoadItemInput *in;

  in = malloc(sizeof(OCTACLoadItemInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadItemInput\n");
    exit(1);
  }

  snprintf(in->i_id, sizeof(in->i_id), "%ld", i_id);
  /* I_IM_ID random within [1 .. 10,000] */
  snprintf(in->i_im_id, sizeof(in->i_im_id), "%ld", TARandom_number(1, 10000));
  TARandom_getAlphaString(in->i_name, 14, 24);
  snprintf(in->i_price, sizeof(in->i_price), "%.2f",
           TARandom_number(100, 10000) / 100.0);
  TARandom_getAlphaString(in->i_data, 26, 50);
  if (TARandom_rand() % 10 == 0)
    OCTACConfig_insertOriginal(in->i_data);

  *inout = in;
}

int OCTACLoadItem_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                           void **inout, char *errmsg, size_t errmsgsize)
{
  OCTACLoadItemInput *in = (OCTACLoadItemInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  static char *insert_item_sql =
    "INSERT INTO item (i_id, i_im_id, i_name, i_price, i_data) "
    "VALUES (:1, :2, :3, :4, :5)";
  static long num_inserts = 0;

  sql = OCSQL_initWithSQL(envhp, errhp, insert_item_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->i_id,
                                     in->i_im_id,
                                     in->i_name,
                                     in->i_price,
                                     in->i_data));
  OCSQL_release(sql);
  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadItem_afterTX(void **inout)
{
  OCTACLoadItemInput *in = (OCTACLoadItemInput *)*inout;
  long i_id = atol(in->i_id);

  if (i_id % 5000 == 0)
    printf("%s\n", in->i_id);

  free(in);
}
