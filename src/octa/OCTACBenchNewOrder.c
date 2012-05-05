/*
 * OCTACBenchNewOrder.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/05/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBenchNewOrder.h"

void OCTACBenchNewOrder_beforeTX(OCTACBenchNewOrderInput *input,
                                 int session_id, long scale_factor,
                                 struct timeval keying_time)
{
  long w_id = OCTACConfig_homeWID(scale_factor, session_id);
  long ol_cnt = TARandom_number(5, 15);
  long rbk = TARandom_number(1, 100);
  TABool home = TRUE;
  long ol_i_id;
  long ol_supply_w_id;
  int i = 0;
  struct timespec keying_timespec;

  snprintf(input->w_id, sizeof(input->w_id), "%ld", w_id);
  snprintf(input->d_id, sizeof(input->d_id), "%ld",
           TARandom_number(1, DIST_PER_WARE));
  snprintf(input->c_id, sizeof(input->c_id), "%ld",
           OCTACConfig_NURand(1023, 1, CUST_PER_DIST));
  snprintf(input->o_ol_cnt, sizeof(input->o_ol_cnt), "%ld", ol_cnt);

  for (i = 0; i < ol_cnt; i++)
  {
    if (i == ol_cnt -1 && rbk == 1)
      ol_i_id = UNUSED_I_ID;
    else
      ol_i_id = OCTACConfig_NURand(8191, 1, MAXITEMS);

    ol_supply_w_id = w_id;
    if (scale_factor > 1)
    {
      if (TARandom_number(1, 100) == 1)
      {
        do
        {
          ol_supply_w_id = TARandom_number(1, scale_factor);
        } while (ol_supply_w_id == w_id);
        home = FALSE;
      }
    }

    snprintf(input->supware[i], sizeof(input->supware[i]), "%ld",
             ol_supply_w_id);
    snprintf(input->itemid[i], sizeof(input->itemid[i]), "%ld", ol_i_id);
    snprintf(input->qty[i], sizeof(input->qty[i]), "%ld",
             TARandom_number(1, 10));
  }
  snprintf(input->o_all_local, sizeof(input->o_all_local), "%d\n",
           home ? 1 : 0);

  keying_timespec.tv_sec = keying_time.tv_sec;
  keying_timespec.tv_nsec = keying_time.tv_usec * 1000;
  nanosleep(&keying_timespec, NULL);
}

int OCTACBenchNewOrder_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize)
{
  OCTACBenchNewOrderInput *in = (OCTACBenchNewOrderInput *)*inout;
  OCTACBenchNewOrderOutput out;
  static OCSQL sql1 = NULL;
  static OCSQL sql2 = NULL;
  static OCSQL sql3 = NULL;
  static OCSQL sql4 = NULL;
  static OCSQL sql5 = NULL;
  static OCSQL sql6 = NULL;
  static OCSQL sql7 = NULL;
  static OCSQL sql8 = NULL;
  static OCSQL sql9 = NULL;
  sword errcode = 0;
  long ol_number;
  char ol_number_str[3];
  double total = 0;

  memset(&out, 0, sizeof(out));

  if (sql1 == NULL)
    sql1 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT c_discount, c_last, c_credit, w_tax "
                             "FROM customer, warehouse "
                             "WHERE w_id = :1 AND c_w_id = w_id "
                             "  AND c_d_id = :2 AND c_id = :3");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql1, errhp, svchp,
                                     in->w_id,
                                     in->d_id,
                                     in->c_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql1, errhp));
  if (errcode != 0) goto end;
  while (errcode == 0)
  {
    snprintf(out.c_discount, sizeof(out.c_discount), "%s",
             OCSQL_valueByPos(sql1, 1));
    snprintf(out.c_last, sizeof(out.c_last), "%s",
             OCSQL_valueByPos(sql1, 2));
    snprintf(out.c_credit, sizeof(out.c_credit), "%s",
             OCSQL_valueByPos(sql1, 3));
    snprintf(out.w_tax, sizeof(out.w_tax), "%s",
             OCSQL_valueByPos(sql1, 4));
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql1, errhp));
  }
  if (errcode != 0 && errcode != OCI_NO_DATA) goto end;

  if (sql2 == NULL)
    sql2 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT d_next_o_id, d_tax "
                             "FROM district "
                             "WHERE d_id = :1 AND d_w_id = :2"
                             " FOR UPDATE"); /* avoid unique constraint    *
                                              * (TPCC.ORDERS_PK) violation */
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql2, errhp, svchp,
                                     in->d_id,
                                     in->w_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql2, errhp));
  if (errcode != 0) goto end;
  while (errcode == 0)
  {
    snprintf(out.d_next_o_id, sizeof(out.d_next_o_id), "%s",
             OCSQL_valueByPos(sql2, 1));
    snprintf(out.d_tax, sizeof(out.d_tax), "%s",
             OCSQL_valueByPos(sql2, 2));
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql2, errhp));
  }
  if (errcode != 0 && errcode != OCI_NO_DATA) goto end;

  if (sql3 == NULL)
    sql3 = OCSQL_initWithSQL(envhp, errhp,
                             "UPDATE district SET d_next_o_id = :1 + 1 "
                             "WHERE d_id = :2 AND d_w_id = :3");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql3, errhp, svchp,
                                     out.d_next_o_id,
                                     in->d_id,
                                     in->w_id));
  if (errcode != 0) goto end;

  if (sql4 == NULL)
    sql4 = OCSQL_initWithSQL(envhp, errhp,
                             "INSERT INTO orders (o_id, o_d_id, o_w_id, "
                             "  o_c_id, o_entry_d, o_carrier_id, o_ol_cnt, "
                             "  o_all_local) "
                             "VALUES (:1, :2, :3, :4, SYSDATE, NULL, :5, :6)");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql4, errhp, svchp,
                                     out.d_next_o_id,
                                     in->d_id,
                                     in->w_id,
                                     in->c_id,
                                     in->o_ol_cnt,
                                     in->o_all_local));
  if (errcode != 0) goto end;

  if (sql5 == NULL)
    sql5 = OCSQL_initWithSQL(envhp, errhp,
                             "INSERT INTO new_order (no_o_id, no_d_id, "
                             "  no_w_id) "
                             "VALUES (:1, :2, :3)");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql5, errhp, svchp,
                                     out.d_next_o_id,
                                     in->d_id,
                                     in->w_id));
  if (errcode != 0) goto end;

  for (ol_number = 0; ol_number < atol(in->o_ol_cnt); ol_number++)
  {
    if (sql6 == NULL)
      sql6 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT i_price, i_name , i_data "
                               "FROM item "
                               "WHERE i_id = :1");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql6, errhp, svchp,
                                       in->itemid[ol_number]));
    if (errcode != 0)
    {
      if (errcode == OCI_NO_DATA)
      {
        errcode = INVALID_ITEM_ERROR_CODE;
        if (strlen(errmsg) + strlen(INVALID_ITEM_ERROR_MESSAGE) < errmsgsize)
          strcat(errmsg, INVALID_ITEM_ERROR_MESSAGE);
      }
      goto end;        
    }
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql6, errhp));
    if (errcode != 0)
    {
      if (errcode == OCI_NO_DATA)
      {
        errcode = INVALID_ITEM_ERROR_CODE;
        if (strlen(errmsg) + strlen(INVALID_ITEM_ERROR_MESSAGE) < errmsgsize)
          strcat(errmsg, INVALID_ITEM_ERROR_MESSAGE);
      }
      goto end;        
    }
    while (errcode == 0)
    {
      snprintf(out.price[ol_number], sizeof(out.price[ol_number]), "%s",
               OCSQL_valueByPos(sql6, 1));
      snprintf(out.iname[ol_number], sizeof(out.iname[ol_number]), "%s",
               OCSQL_valueByPos(sql6, 2));
      snprintf(out.i_data, sizeof(out.i_data), "%s",
               OCSQL_valueByPos(sql6, 3));
      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCSQL_fetch(sql6, errhp));
    }
    if (errcode != 0 && errcode != OCI_NO_DATA) goto end;

    if (sql7 == NULL)
      sql7 = OCSQL_initWithSQL(envhp, errhp,
                               "SELECT s_quantity, s_data, "
                               "  s_dist_01, s_dist_02, s_dist_03, s_dist_04, "
                               "  s_dist_05, s_dist_06, s_dist_07, s_dist_08, "
                               "  s_dist_09, s_dist_10 "
                               "FROM stock "
                               "WHERE s_i_id = :1 AND s_w_id = :2");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql7, errhp, svchp,
                                       in->itemid[ol_number],
                                       in->supware[ol_number]));
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize, OCSQL_fetch(sql7, errhp));
    if (errcode != 0) goto end;
    while (errcode == 0)
    {
      snprintf(out.s_quantity, sizeof(out.s_quantity), "%s",
               OCSQL_valueByPos(sql7, 1));
      snprintf(out.s_data, sizeof(out.s_data), "%s",
               OCSQL_valueByPos(sql7, 2));
      snprintf(out.s_dist_xx[0], sizeof(out.s_dist_xx[0]), "%s",
               OCSQL_valueByPos(sql7, 3 + 0));
      snprintf(out.s_dist_xx[1], sizeof(out.s_dist_xx[1]), "%s",
               OCSQL_valueByPos(sql7, 3 + 1));
      snprintf(out.s_dist_xx[2], sizeof(out.s_dist_xx[2]), "%s",
               OCSQL_valueByPos(sql7, 3 + 2));
      snprintf(out.s_dist_xx[3], sizeof(out.s_dist_xx[3]), "%s",
               OCSQL_valueByPos(sql7, 3 + 3));
      snprintf(out.s_dist_xx[4], sizeof(out.s_dist_xx[4]), "%s",
               OCSQL_valueByPos(sql7, 3 + 4));
      snprintf(out.s_dist_xx[5], sizeof(out.s_dist_xx[5]), "%s",
               OCSQL_valueByPos(sql7, 3 + 5));
      snprintf(out.s_dist_xx[6], sizeof(out.s_dist_xx[6]), "%s",
               OCSQL_valueByPos(sql7, 3 + 6));
      snprintf(out.s_dist_xx[7], sizeof(out.s_dist_xx[7]), "%s",
               OCSQL_valueByPos(sql7, 3 + 7));
      snprintf(out.s_dist_xx[8], sizeof(out.s_dist_xx[8]), "%s",
               OCSQL_valueByPos(sql7, 3 + 8));
      snprintf(out.s_dist_xx[9], sizeof(out.s_dist_xx[9]), "%s",
               OCSQL_valueByPos(sql7, 3 + 9));
      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCSQL_fetch(sql7, errhp));
    }
    if (errcode != 0 && errcode != OCI_NO_DATA) goto end;

    snprintf(out.ol_dist_info, sizeof(out.ol_dist_info), "%s",
             out.s_dist_xx[atol(in->d_id) - 1]);
    snprintf(out.stock[ol_number], sizeof(out.stock[ol_number]), "%s",
             out.s_quantity);

    if ((strstr(out.i_data, "original") != NULL) &&
        (strstr(out.s_data, "original") != NULL))
      out.bg[ol_number] = 'B';
    else
      out.bg[ol_number] = 'G';

    /*
     * If the retrieved value for S_QUANTITY exceeds OL_QUANTITY by 10 or more,
     * then S_QUANTITY is decreased by OL_QUANTITY; otherwise S_QUANTITY is
     * updated to (S_QUANTITY - OL_QUANTITY)+91.
     */
    if (atol(out.s_quantity) >= atol(in->qty[ol_number]) + 10)
      snprintf(out.s_quantity, sizeof(out.s_quantity), "%ld",
               atol(out.s_quantity) - atol(in->qty[ol_number]));
    else
      snprintf(out.s_quantity, sizeof(out.s_quantity), "%ld",
               atol(out.s_quantity) - atol(in->qty[ol_number]) + 91);
    
    /*
     * S_YTD is increased by OL_QUANTITY and S_ORDER_CNT is incremented by 1.
     * If the order-line is remote, then S_REMOTE_CNT is incremented by 1.
     */
    if (sql8 == NULL)
      sql8 = OCSQL_initWithSQL(envhp, errhp,
                               "UPDATE stock SET s_quantity = :1, "
                               "  s_ytd = s_ytd + :2, "
                               "  s_order_cnt = s_order_cnt + 1, "
                               "  s_remote_cnt = s_remote_cnt + :3 "
                               "WHERE s_i_id = :4 "
                               "AND s_w_id = :5");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql8, errhp, svchp,
                                       out.s_quantity,
                                       in->qty[ol_number],
                                       atol(in->w_id) !=
                                       atol(in->supware[ol_number]) ?
                                       "1" : "0",
                                       in->itemid[ol_number],
                                       in->supware[ol_number]));
    if (errcode != 0) goto end;

    /*
     * The amount for the item in the order (OL_AMOUNT) is computed as:
     *   OL_QUANTITY*I_PRICE
     */
    snprintf(out.ol_amount, sizeof(out.ol_amount), "%f",
             atol(in->qty[ol_number]) * atof(out.price[ol_number]));
    snprintf(out.amt[ol_number], sizeof(out.amt[ol_number]), "%s",
             out.ol_amount);
    /*
     * The total-amount for the complete order is computed as:
     *   sum(OL_AMOUNT) *(1 - C_DISCOUNT) *(1 + W_TAX + D_TAX)
     */
    total += atof(out.ol_amount) * (1 + atof(out.w_tax) + atof(out.d_tax)) *
             (1 - atof(out.c_discount));

    snprintf(ol_number_str, sizeof(ol_number_str), "%ld", ol_number + 1);
    if (sql9 == NULL)
      sql9 = OCSQL_initWithSQL(envhp, errhp,
                               "INSERT INTO order_line (ol_o_id, ol_d_id, "
                               "  ol_w_id, ol_number, ol_i_id, "
                               "  ol_supply_w_id, ol_quantity, ol_amount, "
                               "  ol_dist_info) "
                               "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9)");
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql9, errhp, svchp,
                                       out.d_next_o_id,
                                       in->d_id,
                                       in->w_id,
                                       ol_number_str,
                                       in->itemid[ol_number],
                                       in->supware[ol_number],
                                       in->qty[ol_number],
                                       out.ol_amount,
                                       out.ol_dist_info));
    if (errcode != 0) goto end;
  }

  snprintf(out.total, sizeof(out.total), "%f", total);
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

  return errcode;
}

void OCTACBenchNewOrder_afterTX(struct timeval think_time)
{
  long think_time_nsec;
  struct timespec think_timespec;

  think_time_nsec = (think_time.tv_sec * 1000000 + think_time.tv_usec) * 1000;
  think_time_nsec = - log(TARandom_drand()) * think_time_nsec;

  think_timespec.tv_sec = think_time_nsec / 1000000000;
  think_timespec.tv_nsec = think_time_nsec % 1000000000;
  nanosleep(&think_timespec, NULL);
}
