/*
 * OCTACBenchPayment.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/12/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBenchPayment.h"

void OCTACBenchPayment_beforeTX(OCTACBenchPaymentInput *input,
                                int session_id, long scale_factor,
                                struct timeval keying_time)
{
  long w_id = OCTACConfig_homeWID(scale_factor, session_id);
  long d_id = TARandom_number(1, DIST_PER_WARE);
  long c_d_id;
  long c_w_id;
  long x = TARandom_number(1, 100);
  long y = TARandom_number(1, 100);

  snprintf(input->w_id, sizeof(input->w_id), "%ld", w_id);
  snprintf(input->d_id, sizeof(input->d_id), "%ld", d_id);
  if (x <= 85)
  {
    c_d_id = d_id;
    c_w_id = w_id;
  }
  else
  {
    c_d_id = TARandom_number(1, DIST_PER_WARE);
    if (scale_factor > 1)
    {
      do
      {
        c_w_id = TARandom_number(1, scale_factor);
      } while (c_w_id == w_id);
    }
    else
      c_w_id = w_id;
  }
  snprintf(input->c_d_id, sizeof(input->c_d_id), "%ld", c_d_id);
  snprintf(input->c_w_id, sizeof(input->c_w_id), "%ld", c_w_id);
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
  snprintf(input->h_amount, sizeof(input->h_amount), "%f",
           TARandom_number(100, 500000) / 100.0);

  OCTACConfig_sleepKeyingTime(keying_time);
}

int OCTACBenchPayment_oracleTX(OCIEnv *envhp, OCIError *errhp,
                               OCISvcCtx *svchp, void **inout, char *errmsg,
                               size_t errmsgsize)
{
  OCTACBenchPaymentInput *in = (OCTACBenchPaymentInput *)*inout;
  OCTACBenchPaymentOutput out;
  static OCSQL sql1 = NULL;
  static OCSQL sql2 = NULL;
  static OCSQL sql3 = NULL;
  static OCSQL sql4 = NULL;
  static OCSQL sql5 = NULL;
  static OCSQL sql6 = NULL;
  static OCSQL sql7 = NULL;
  static OCSQL sql8 = NULL;
  static OCSQL sql9 = NULL;
  static OCSQL sql10 = NULL;
  static OCSQL sql11 = NULL;
  sword errcode = 0;
  long namecnt = 0;
  long n = 0;

  memset(&out, 0, sizeof(out));

  if (sql1 == NULL)
    sql1 = OCSQL_initWithSQL(envhp, errhp,
                             "UPDATE warehouse SET w_ytd = w_ytd + :1 "
                             "WHERE w_id = :2");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql1, errhp, svchp,
                                     in->h_amount,
                                     in->w_id));
  if (errcode != 0) goto end;

  if (sql2 == NULL)
    sql2 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT w_street_1, w_street_2, w_city, w_state, "
                             "w_zip, w_name "
                             "FROM warehouse "
                             "WHERE w_id = :1");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql2, errhp, svchp,
                                     in->w_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql2, errhp,
                                       out.w_street_1,
                                       out.w_street_2,
                                       out.w_city,
                                       out.w_state,
                                       out.w_zip,
                                       out.w_name));
  if (errcode != 0) goto end;

  if (sql3 == NULL)
    sql3 = OCSQL_initWithSQL(envhp, errhp,
                             "UPDATE district SET d_ytd = d_ytd + :1 "
                             "WHERE d_w_id = :2 AND d_id = :3");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql3, errhp, svchp,
                                     in->h_amount,
                                     in->w_id,
                                     in->d_id));
  if (errcode != 0) goto end;

  if (sql4 == NULL)
    sql4 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT d_street_1, d_street_2, d_city, d_state, "
                             "d_zip, d_name "
                             "FROM district "
                             "WHERE d_w_id = :1 AND d_id = :2");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql4, errhp, svchp,
                                     in->w_id,
                                     in->d_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql4, errhp,
                                       out.d_street_1,
                                       out.d_street_2,
                                       out.d_city,
                                       out.d_state,
                                       out.d_zip,
                                       out.d_name));
  if (errcode != 0) goto end;

  snprintf(out.c_id, sizeof(out.c_id), "%s", in->c_id);
  if (in->byname)
  {
    if (sql5 == NULL)
      sql5 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT count(c_id) "
                               "FROM customer "
                               "WHERE c_last = :1 AND c_d_id = :2 "
                               "  AND c_w_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql5, errhp, svchp,
                                       in->c_last,
                                       in->c_d_id,
                                       in->c_w_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql5, errhp));
    if (errcode != 0) goto end;
    namecnt = atol(OCSQL_valueByPos(sql5, 1));

    if (sql6 == NULL)
      sql6 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT c_first, c_middle, c_id, "
                               "  c_street_1, c_street_2, c_city, c_state, "
                               "  c_zip, c_phone, c_credit, c_credit_lim, "
                               "  c_discount, c_balance, "
                               /* "  c_since " */
                               " to_char(c_since) c_since " /* @todo Avoid ORA-1406 */
                               "FROM customer "
                               "WHERE c_w_id = :1 AND c_d_id = :2 "
                               "  AND c_last = :3 "
                               "ORDER BY c_first");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql6, errhp, svchp,
                                       in->c_w_id,
                                       in->c_d_id,
                                       in->c_last));
    if (errcode != 0) goto end;

    if (namecnt % 2 == 1) namecnt++;
    for (n = 0; n < namecnt / 2; n++)
    {
      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCSQL_fetchInto(sql6, errhp,
                                           out.c_first,
                                           out.c_middle,
                                           out.c_id,
                                           out.c_street_1,
                                           out.c_street_2,
                                           out.c_city,
                                           out.c_state,
                                           out.c_zip,
                                           out.c_phone,
                                           out.c_credit,
                                           out.c_credit_lim,
                                           out.c_discount,
                                           out.c_balance,
                                           out.c_since));
      if (errcode != 0) goto end;
    }
  }
  else
  {
    if (sql7 == NULL)
      sql7 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT c_first, c_middle, c_last, "
                               "  c_street_1, c_street_2, c_city, c_state, "
                               "  c_zip, c_phone, c_credit, c_credit_lim, "
                               "  c_discount, c_balance, "
                               /* "  c_since " */
                               " to_char(c_since) c_since " /* @todo Avoid ORA-1406 */
                               "FROM customer "
                               "WHERE c_w_id = :1 AND c_d_id = :2 "
                               "  AND c_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql7, errhp, svchp,
                                       in->c_w_id,
                                       in->c_d_id,
                                       in->c_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql7, errhp,
                                         out.c_first,
                                         out.c_middle,
                                         out.c_last,
                                         out.c_street_1,
                                         out.c_street_2,
                                         out.c_city,
                                         out.c_state,
                                         out.c_zip,
                                         out.c_phone,
                                         out.c_credit,
                                         out.c_credit_lim,
                                         out.c_discount,
                                         out.c_balance,
                                         out.c_since));
    if (errcode != 0) goto end;
  }

  if (strstr(out.c_credit, "BC") != NULL)
  {
    if (sql8 == NULL)
      sql8 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT c_data "
                               "FROM customer "
                               "WHERE c_w_id = :1 AND c_d_id = :2 "
                               "  AND c_id = :3");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql8, errhp, svchp,
                                       in->c_w_id,
                                       in->c_d_id,
                                       out.c_id));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql8, errhp,
                                         out.c_data));
    if (errcode != 0) goto end;

    /*
     * If the value of C_CREDIT is equal to "BC", then C_DATA is also
     * retrieved from the selected customer and the following history
     * information: C_ID, C_D_ID, C_W_ID, D_ID, W_ID, and H_AMOUNT, are
     * inserted at the left of the C_DATA field by shifting the existing
     * content of C_DATA to the right by an equal number of bytes and by
     * discarding the bytes that are shifted out of the right side of the
     * C_DATA field. The content of the C_DATA field never exceeds 500
     * characters. The selected customer is updated with the new C_DATA field.
     * If C_DATA is implemented as two fields (see Clause 1.4.9), they must
     * be treated and operated on as one single field.
     */
    snprintf(out.c_new_data, sizeof(out.c_new_data), "| %s %s %s %s %s %s",
             out.c_id, in->c_d_id, in->c_w_id, in->d_id, in->w_id,
             in->h_amount);
    strncat(out.c_new_data, out.c_data, 500 - strlen(out.c_new_data));

    /*
     * C_BALANCE is decreased by H_AMOUNT. C_YTD_PAYMENT is increased by
     * H_AMOUNT. C_PAYMENT_CNT is incremented by 1.
     */
    if (sql9 == NULL)
      sql9 = OCSQL_initWithSQL(envhp, errhp,
                               "UPDATE customer "
                               "SET c_balance = c_balance - :1, c_data = :2, "
                               "  c_ytd_payment = c_ytd_payment + :3, "
                               "  c_payment_cnt = c_payment_cnt + 1 "
                               "WHERE c_w_id = :4 AND c_d_id = :5 "
                               "  AND c_id = :6");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql9, errhp, svchp,
                                       in->h_amount,
                                       out.c_new_data,
                                       in->h_amount,
                                       in->c_w_id,
                                       in->c_d_id,
                                       out.c_id));
    if (errcode != 0) goto end;
  }
  else
  {
    /*
     * C_BALANCE is decreased by H_AMOUNT. C_YTD_PAYMENT is increased by
     * H_AMOUNT. C_PAYMENT_CNT is incremented by 1.
     */
    if (sql10 == NULL)
      sql10 = OCSQL_initWithSQL(envhp, errhp,
                                "UPDATE customer "
                                "SET c_balance = c_balance - :1, "
                                "  c_ytd_payment = c_ytd_payment + :2, "
                                "  c_payment_cnt = c_payment_cnt + 1 "
                                "WHERE c_w_id = :3 AND c_d_id = :4 "
                                "  AND c_id = :5");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql10, errhp, svchp,
                                       in->h_amount,
                                       in->h_amount,
                                       in->c_w_id,
                                       in->c_d_id,
                                       out.c_id));
    if (errcode != 0) goto end;
  }

  /*
   * H_DATA is built by concatenating W_NAME and D_NAME separated by 4 spaces. 
   */
  snprintf(out.h_data, sizeof(out.h_data), "%s    %s", out.w_name, out.d_name);

  if (sql11 == NULL)
    sql11 = OCSQL_initWithSQL(envhp, errhp,
                              "INSERT INTO history (h_c_d_id, h_c_w_id, "
                              "  h_c_id, h_d_id, h_w_id, h_date, h_amount, "
                              "h_data) "
                              "VALUES (:1, :2, :3, :4, :5, SYSDATE, :6, :7)");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql11, errhp, svchp,
                                     in->c_d_id,
                                     in->c_w_id,
                                     out.c_id,
                                     in->d_id,
                                     in->w_id,
                                     in->h_amount,
                                     out.h_data));
  if (errcode != 0) goto end;

  OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  /* OCSQL_release(sql1); */
  /* OCSQL_release(sql2); */
  /* OCSQL_release(sql3); */
  /* OCSQL_release(sql4); */
  /* OCSQL_release(sql5); */
  /* OCSQL_release(sql6); */
  /* OCSQL_release(sql7); */
  /* OCSQL_release(sql8); */
  /* OCSQL_release(sql9); */
  /* OCSQL_release(sql10); */
  /* OCSQL_release(sql11); */

  return errcode;
}

void OCTACBenchPayment_afterTX(struct timeval think_time)
{
  OCTACConfig_sleepThinkTime(think_time);
}
