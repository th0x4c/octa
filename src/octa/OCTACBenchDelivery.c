/*
 * OCTACBenchDelivery.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBenchDelivery.h"

void OCTACBenchDelivery_beforeTX(OCTACBenchDeliveryInput *input,
                                 int session_id, long scale_factor,
                                 struct timeval keying_time)
{
  long w_id = OCTACConfig_homeWID(scale_factor, session_id);

  snprintf(input->w_id, sizeof(input->w_id), "%ld", w_id);
  snprintf(input->o_carrier_id, sizeof(input->o_carrier_id), "%ld",
           TARandom_number(1, DIST_PER_WARE));

  OCTACConfig_sleepKeyingTime(keying_time);
}

int OCTACBenchDelivery_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout,
                                char *errmsg, size_t errmsgsize)
{
  OCTACBenchDeliveryInOut *io = (OCTACBenchDeliveryInOut *)*inout;
  OCTACBenchDeliveryInput *in = &(io->input);
  OCTACBenchDeliveryOutput *out = &(io->output);
  static OCSQL sql1 = NULL;
  static OCSQL sql2 = NULL;
  static OCSQL sql3 = NULL;
  static OCSQL sql4 = NULL;
  static OCSQL sql5 = NULL;
  static OCSQL sql6 = NULL;
  static OCSQL sql7 = NULL;
  sword errcode = 0;
  TALog log = in->log;
  struct timeval datetime;
  char datetime_str[24] = "0000-00-00 00:00:00.000";
  char logstr[128];
  long d_id;

  memset(out, 0, sizeof(*out));

  timerclear(&datetime);
  gettimeofday(&datetime, (struct timezone *)0);
  timeval2str(datetime_str, datetime);

  /*
   * Upon completion of the business transaction, the following information
   * must have been recorded into a result file:
   *   o The time at which the business transaction was queued.
   *   o The warehouse number (W_ID) and the carried number (O_CARRIER_ID)
   *     associated with the business transaction.
   *   o The district number (D_ID) and the order number (O_ID) of each order
   *     delivered by the business transaction.
   *   o The time at which the business transaction completed.
   */
  snprintf(logstr, sizeof(logstr), "W: %s, C: %s", in->w_id, in->o_carrier_id);
  TALog_info(log, logstr);

  for (d_id = 1; d_id <= DIST_PER_WARE; d_id++)
  {
    snprintf(out->d_id, sizeof(out->d_id), "%ld", d_id);

    /*
     * The row in the NEW-ORDER table with matching NO_W_ID (equals W_ID) and
     * NO_D_ID (equals D_ID) and with the lowest NO_O_ID value is selected.
     * This is the oldest undelivered order of that district. NO_O_ID, the
     * order number, is retrieved. If no matching row is found, then the
     * delivery of an order for this district is skipped.
     */
    if (sql1 == NULL)
      sql1 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT MIN(no_o_id) "
                               "FROM new_order "
                               "WHERE no_d_id = :1 AND no_w_id = :2");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql1, errhp, svchp,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql1, errhp,
                                         out->no_o_id));
    if (errcode != 0)
    {
      if (errcode == OCI_NO_DATA || errcode == 1405) /* NULL value returned */
        continue;
      else
        goto end;
    }

    if (sql2 == NULL)
      sql2 = OCSQL_initWithSQL(envhp, errhp,
                               "DELETE FROM new_order "
                               "WHERE no_w_id = :1 AND no_d_id = :2 "
                               "  AND no_o_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql2, errhp, svchp,
                                       in->w_id,
                                       out->d_id,
                                       out->no_o_id));
    if (errcode != 0) goto end;

    if (sql3 == NULL)
      sql3 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT o_c_id "
                               "FROM orders "
                               "WHERE o_id = :1 AND o_d_id = :2 "
                               "  AND  o_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql3, errhp, svchp,
                                       out->no_o_id,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql3, errhp,
                                         out->c_id));
    if (errcode != 0) goto end;

    if (sql4 == NULL)
      sql4 = OCSQL_initWithSQL(envhp, errhp,
                               "UPDATE orders SET o_carrier_id = :1 "
                               "WHERE o_id = :2 AND o_d_id = :3 "
                               "  AND o_w_id = :4");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql4, errhp, svchp,
                                       in->o_carrier_id,
                                       out->no_o_id,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;

    if (sql5 == NULL)
      sql5 = OCSQL_initWithSQL(envhp, errhp,
                               "UPDATE order_line SET ol_delivery_d = SYSDATE "
                               "WHERE ol_o_id = :1 AND ol_d_id = :2 "
                               "  AND ol_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql5, errhp, svchp,
                                       out->no_o_id,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;

    if (sql6 == NULL)
      sql6 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT SUM(ol_amount) "
                               "FROM order_line "
                               "WHERE ol_o_id = :1 AND ol_d_id = :2 "
                               "  AND ol_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql6, errhp, svchp,
                                       out->no_o_id,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql6, errhp,
                                         out->ol_total));
    if (errcode != 0) goto end;

    /*
     * The row in the CUSTOMER table with matching C_W_ID (equals W_ID),
     * C_D_ID (equals D_ID), and C_ID (equals O_C_ID) is selected and
     * C_BALANCE is increased by the sum of all order-line amounts (OL_AMOUNT)
     * previously retrieved. C_DELIVERY_CNT is incremented by 1.
     */
    if (sql7 == NULL)
      sql7 = OCSQL_initWithSQL(envhp, errhp,
                               "UPDATE customer "
                               "SET c_balance = c_balance + :1, "
                               "  c_delivery_cnt = c_delivery_cnt + 1 "
                               "WHERE c_id = :2 AND c_d_id = :3 "
                               "  AND c_w_id = :4");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql7, errhp, svchp,
                                       out->ol_total,
                                       out->c_id,
                                       out->d_id,
                                       in->w_id));
    if (errcode != 0) goto end;

    OCITransCommit(svchp, errhp, (ub4) 0);

    snprintf(logstr, sizeof(logstr), "D: %s, O: %s, time: %s",
             out->d_id, out->no_o_id, datetime_str);
    TALog_info(log, logstr);
  }

  OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  /* OCSQL_release(sql1); */
  /* OCSQL_release(sql2); */
  /* OCSQL_release(sql3); */
  /* OCSQL_release(sql4); */
  /* OCSQL_release(sql5); */
  /* OCSQL_release(sql6); */
  /* OCSQL_release(sql7); */

  return errcode;
}

void OCTACBenchDelivery_afterTX(OCTACBenchDeliveryInOut *inout,
                                struct timeval think_time)
{
  OCTACConfig_sleepThinkTime(think_time);
}
