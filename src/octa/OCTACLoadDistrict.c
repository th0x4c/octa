/*
 * OCTACLoadDistrict.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadDistrict.h"

struct OCTACLoadDistrictInput
{
  char d_id[3];        /* NUMBER(2, 0) */
  char d_w_id[40];     /* NUMBER */
  char d_name[11];     /* VARCHAR2(10) */
  char d_street_1[21]; /* VARCHAR2(20) */
  char d_street_2[21]; /* VARCHAR2(20) */
  char d_city[21];     /* VARCHAR2(20) */
  char d_state[3];     /* CHAR(2) */
  char d_zip[10];      /* CHAR(9) */
  char d_tax[7];       /* NUMBER(4, 4) */
  char d_ytd[14];      /* NUMBER(12, 2) */
  char d_next_o_id[9]; /* NUMBER(8, 0) */
};
typedef struct OCTACLoadDistrictInput OCTACLoadDistrictInput;

void OCTACLoadDistrict_beforeTX(long d_id, long d_w_id, void **inout)
{
  OCTACLoadDistrictInput *in;

  in = malloc(sizeof(OCTACLoadDistrictInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadDistrictInput\n");
    exit(1);
  }

  snprintf(in->d_id, sizeof(in->d_id), "%ld", d_id);
  snprintf(in->d_w_id, sizeof(in->d_w_id), "%ld", d_w_id);
  TARandom_getAlphaString(in->d_name, 6, 10);
  OCTACConfig_makeAddress(in->d_street_1, in->d_street_2, in->d_city,
                          in->d_state, in->d_zip);
  /* D_TAX random within [0.0000 .. 0.2000] */
  snprintf(in->d_tax, sizeof(in->d_tax), "%.4f",
           TARandom_number(0, 20) / 100.0);
  snprintf(in->d_ytd, sizeof(in->d_ytd), "300000.00");
  snprintf(in->d_next_o_id, sizeof(in->d_next_o_id), "%ld", 3001L);

  *inout = in;
}

int OCTACLoadDistrict_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize)
{
  OCTACLoadDistrictInput *in = (OCTACLoadDistrictInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  static char *insert_district_sql =
    "INSERT INTO district (d_id, d_w_id, d_name, d_street_1, d_street_2, "
    "d_city, d_state, d_zip, d_tax, d_ytd, d_next_o_id) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9, :10, :11)";
  static long num_inserts = 0;

  sql = OCSQL_initWithSQL(envhp, errhp, insert_district_sql);
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql, errhp, svchp,
                                     in->d_id,
                                     in->d_w_id,
                                     in->d_name,
                                     in->d_street_1,
                                     in->d_street_2,
                                     in->d_city,
                                     in->d_state,
                                     in->d_zip,
                                     in->d_tax,
                                     in->d_ytd,
                                     in->d_next_o_id));
  OCSQL_release(sql);
  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadDistrict_afterTX(void **inout)
{
  OCTACLoadDistrictInput *in = (OCTACLoadDistrictInput *)*inout;

  free(in);
}
