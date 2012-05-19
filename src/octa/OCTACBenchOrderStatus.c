/*
 * OCTACBenchOrderStatus.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/12/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBenchOrderStatus.h"

void OCTACBenchOrderStatus_beforeTX(OCTACBenchOrderStatusInput *input,
                                    int session_id, long scale_factor,
                                    struct timeval keying_time)
{
  long w_id = OCTACConfig_homeWID(scale_factor, session_id);
  long y = TARandom_number(1, 100);

  snprintf(input->w_id, sizeof(input->w_id), "%ld", w_id);
  snprintf(input->d_id, sizeof(input->d_id), "%ld",
           TARandom_number(1, DIST_PER_WARE));
  if (y <= 60)
  {
    OCTACConfig_lastname(OCTACConfig_NURand(255, 0, 999), input->c_last);
    input->byname = TRUE;
  }
  else
  {
    snprintf(input->c_id, sizeof(input->c_id), "%ld",
             OCTACConfig_NURand(1023, 1, CUST_PER_DIST));
    input->byname = FALSE;
  }

  OCTACConfig_sleepKeyingTime(keying_time);
}

int OCTACBenchOrderStatus_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                   OCISvcCtx *svchp, void **inout,
                                   char *errmsg, size_t errmsgsize)
{
  OCTACBenchOrderStatusInOut *io = (OCTACBenchOrderStatusInOut *)*inout;
  OCTACBenchOrderStatusInput *in = &(io->input);
  OCTACBenchOrderStatusOutput *out = &(io->output);
  static OCSQL sql1 = NULL;
  static OCSQL sql2 = NULL;
  static OCSQL sql3 = NULL;
  static OCSQL sql4 = NULL;
  static OCSQL sql5 = NULL;
  sword errcode = 0;
  long namecnt = 0;
  long n = 0;
  int i = 0;

  memset(out, 0, sizeof(*out));

  snprintf(out->c_id, sizeof(out->c_id), "%s", in->c_id);
  if (in->byname)
  {
    if (sql1 == NULL)
      sql1 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT count(c_id) "
                               "FROM customer "
                               "WHERE c_last = :1 AND c_d_id = :2 "
                               "  AND c_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql1, errhp, svchp,
                                       in->c_last,
                                       in->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql1, errhp));
    if (errcode != 0) goto end;
    namecnt = atol(OCSQL_valueByPos(sql1, 1));

    if (sql2 == NULL)
      sql2 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT c_balance, c_first, c_middle, c_id "
                               "FROM customer "
                               "WHERE c_last = :1 AND c_d_id = :2 "
                               "  AND c_w_id = :3"
                               "ORDER BY c_first");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql2, errhp, svchp,
                                       in->c_last,
                                       in->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;

    if (namecnt % 2 == 1) namecnt++;
    for (n = 0; n < namecnt / 2; n++)
    {
      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCSQL_fetchInto(sql2, errhp,
                                           out->c_balance,
                                           out->c_first,
                                           out->c_middle,
                                           out->c_id));
      if (errcode != 0) goto end;
    }
  }
  else
  {
    if (sql3 == NULL)
      sql3 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT c_balance, c_first, c_middle, c_last "
                               "FROM customer "
                               "WHERE c_id = :1 AND c_d_id = :2 "
                               "  AND c_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql3, errhp, svchp,
                                       in->c_id,
                                       in->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql3, errhp,
                                         out->c_balance,
                                         out->c_first,
                                         out->c_middle,
                                         out->c_last));
    if (errcode != 0) goto end;
  }

  /*
   * The row in the ORDER table with matching O_W_ID (equals C_W_ID), O_D_ID
   * (equals C_D_ID), O_C_ID (equals C_ID), and with the largest existing
   * O_ID, is selected. This is the most recent order placed by that customer.
   * O_ID, O_ENTRY_D, and O_CARRIER_ID are retrieved.
   */
  if (sql4 == NULL)
    sql4 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT o_id, o_carrier_id, "
                             /* "  o_entry_d " */
                             "  to_char(o_entry_d) o_entry_d " /* @todo Avoid ORA-1406 */
                             "FROM orders "
                             "WHERE o_w_id = :1 AND o_d_id = :2 "
                             "  AND o_c_id = :3 AND "
                             "  o_id = (SELECT MAX(o_id) FROM orders "
                             "          WHERE o_w_id = :4 AND o_d_id = :5 "
                             "            AND o_c_id = :6)");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql4, errhp, svchp,
                                     in->w_id,
                                     in->d_id,
                                     out->c_id,
                                     in->w_id,
                                     in->d_id,
                                     out->c_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql4, errhp,
                                       out->o_id,
                                       out->o_carrier_id,
                                       out->o_entry_d));
  if (errcode != 0 && errcode != 1405) goto end; /* o_carrier_id can be NULL */

  if (sql5 == NULL)
    sql5 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT ol_i_id, ol_supply_w_id, ol_quantity, "
                             "  ol_amount, "
                             /* "  ol_delivery_d " */
                             " to_char(ol_delivery_d) ol_delivery_d " /* @todo Avoid ORA-1406 */
                             "FROM order_line "
                             "WHERE ol_o_id = :1 AND ol_d_id = :2 "
                             "  AND ol_w_id = :3");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql5, errhp, svchp,
                                     out->o_id,
                                     in->d_id,
                                     in->w_id));
  if (errcode != 0) goto end;
  for (i = 0; errcode != OCI_NO_DATA; i++)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql5, errhp,
                                         out->ol_i_id[i],
                                         out->ol_supply_w_id[i],
                                         out->ol_quantity[i],
                                         out->ol_amount[i],
                                         out->ol_delivery_d[i]));
    if (errcode != 0 && errcode != OCI_NO_DATA) goto end;
  }
  if (errcode == OCI_NO_DATA) errcode = 0;

  /*
   * Comment: a commit is not required as long as all ACID properties are
   * satisfied (see Clause 3).
   */
  /* OCITransCommit(svchp, errhp, (ub4) 0); */

 end:
  /* if (sql1 != NULL) */
  /* { */
  /*   OCSQL_release(sql1); */
  /*   sql1 = NULL; */
  /* } */
  /* if (sql2 != NULL) */
  /* { */
  /*   OCSQL_release(sql2); */
  /*   sql2 = NULL; */
  /* } */
  /* if (sql3 != NULL) */
  /* { */
  /*   OCSQL_release(sql3); */
  /*   sql3 = NULL; */
  /* } */
  /* if (sql4 != NULL) */
  /* { */
  /*   OCSQL_release(sql4); */
  /*   sql4 = NULL; */
  /* } */
  /* if (sql5 != NULL) */
  /* { */
  /*   OCSQL_release(sql5); */
  /*   sql5 = NULL; */
  /* } */

  return errcode;
}

void OCTACBenchOrderStatus_afterTX(OCTACBenchOrderStatusInOut *inout,
                                   struct timeval think_time)
{
  OCTACConfig_sleepThinkTime(think_time);
}
