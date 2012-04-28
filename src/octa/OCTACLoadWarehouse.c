/*
 * OCTACLoadWarehouse.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadWarehouse.h"

struct OCTACLoadWarehouseInput
{
  char w_id[40];       /* NUMBER */
  char w_name[11];     /* VARCHAR2(10) */
  char w_street_1[21]; /* VARCHAR2(20) */
  char w_street_2[21]; /* VARCHAR2(20) */
  char w_city[21];     /* VARCHAR2(20) */
  char w_state[3];     /* CHAR(2) */
  char w_zip[10];      /* CHAR(9) */
  char w_tax[7];       /* NUMBER(4, 4) */
  char w_ytd[14];      /* NUMBER(12, 2) */
};
typedef struct OCTACLoadWarehouseInput OCTACLoadWarehouseInput;

void OCTACLoadWarehouse_beforeTX(long w_id, void **inout)
{
  OCTACLoadWarehouseInput *in;

  in = malloc(sizeof(OCTACLoadWarehouseInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadWarehouseInput\n");
    exit(1);
  }

  snprintf(in->w_id, sizeof(in->w_id), "%ld", w_id);
  TARandom_getAlphaString(in->w_name, 6, 10);
  OCTACConfig_makeAddress(in->w_street_1, in->w_street_2, in->w_city,
                          in->w_state, in->w_zip);
  /* W_TAX random within [0.0000 .. 0.2000] */
  snprintf(in->w_tax, sizeof(in->w_tax), "%.4f",
           TARandom_number(0, 20) / 100.0);
  snprintf(in->w_ytd, sizeof(in->w_ytd), "300000.00"); /* W_YTD = 300,000.00 */

  *inout = in;
}

int OCTACLoadWarehouse_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize)
{
  OCTACLoadWarehouseInput *in = (OCTACLoadWarehouseInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  static char *insert_warehouse_sql =
    "INSERT INTO warehouse (w_id, w_name, w_street_1, w_street_2, w_city, "
    "w_state, w_zip, w_tax, w_ytd) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9)";
  static long num_inserts = 0;

  sql = OCSQL_initWithSQL(envhp, errhp, insert_warehouse_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->w_id,
                                     in->w_name,
                                     in->w_street_1,
                                     in->w_street_2,
                                     in->w_city,
                                     in->w_state,
                                     in->w_zip,
                                     in->w_tax,
                                     in->w_ytd));
  OCSQL_release(sql);
  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadWarehouse_afterTX(void **inout)
{
  OCTACLoadWarehouseInput *in = (OCTACLoadWarehouseInput *)*inout;

  free(in);
}
