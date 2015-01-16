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

  OCTACConfig_sleepKeyingTime(keying_time);
}

int OCTACBenchNewOrder_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize)
{
  OCTACBenchNewOrderInOut *io = (OCTACBenchNewOrderInOut *)*inout;
  OCTACBenchNewOrderInput *in = &(io->input);
  OCTACBenchNewOrderOutput *out = &(io->output);
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

  memset(out, 0, sizeof(*out));

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
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql1, errhp,
                                       out->c_discount,
                                       out->c_last,
                                       out->c_credit,
                                       out->w_tax));
  if (errcode != 0) goto end;

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
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql2, errhp,
                                       out->d_next_o_id,
                                       out->d_tax));
  if (errcode != 0) goto end;

  if (sql3 == NULL)
    sql3 = OCSQL_initWithSQL(envhp, errhp,
                             "UPDATE district SET d_next_o_id = :1 + 1 "
                             "WHERE d_id = :2 AND d_w_id = :3");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql3, errhp, svchp,
                                     out->d_next_o_id,
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
                                     out->d_next_o_id,
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
                                     out->d_next_o_id,
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
    if (errcode != 0) goto end;
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql6, errhp,
                                         out->price[ol_number],
                                         out->iname[ol_number],
                                         out->i_data));
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
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_fetchInto(sql7, errhp,
                                         out->s_quantity,
                                         out->s_data,
                                         out->s_dist_xx[0],
                                         out->s_dist_xx[1],
                                         out->s_dist_xx[2],
                                         out->s_dist_xx[3],
                                         out->s_dist_xx[4],
                                         out->s_dist_xx[5],
                                         out->s_dist_xx[6],
                                         out->s_dist_xx[7],
                                         out->s_dist_xx[8],
                                         out->s_dist_xx[9]));
    if (errcode != 0) goto end;

    snprintf(out->ol_dist_info, sizeof(out->ol_dist_info), "%s",
             out->s_dist_xx[atol(in->d_id) - 1]);
    snprintf(out->stock[ol_number], sizeof(out->stock[ol_number]), "%s",
             out->s_quantity);

    if ((strstr(out->i_data, "original") != NULL) &&
        (strstr(out->s_data, "original") != NULL))
      out->bg[ol_number] = 'B';
    else
      out->bg[ol_number] = 'G';

    /*
     * If the retrieved value for S_QUANTITY exceeds OL_QUANTITY by 10 or more,
     * then S_QUANTITY is decreased by OL_QUANTITY; otherwise S_QUANTITY is
     * updated to (S_QUANTITY - OL_QUANTITY)+91.
     */
    if (atol(out->s_quantity) >= atol(in->qty[ol_number]) + 10)
      snprintf(out->s_quantity, sizeof(out->s_quantity), "%ld",
               atol(out->s_quantity) - atol(in->qty[ol_number]));
    else
      snprintf(out->s_quantity, sizeof(out->s_quantity), "%ld",
               atol(out->s_quantity) - atol(in->qty[ol_number]) + 91);

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
                                       out->s_quantity,
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
    snprintf(out->ol_amount, sizeof(out->ol_amount), "%f",
             atol(in->qty[ol_number]) * atof(out->price[ol_number]));
    snprintf(out->amt[ol_number], sizeof(out->amt[ol_number]), "%s",
             out->ol_amount);
    /*
     * The total-amount for the complete order is computed as:
     *   sum(OL_AMOUNT) *(1 - C_DISCOUNT) *(1 + W_TAX + D_TAX)
     */
    total += atof(out->ol_amount) * (1 + atof(out->w_tax) + atof(out->d_tax)) *
             (1 - atof(out->c_discount));

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
                                       out->d_next_o_id,
                                       in->d_id,
                                       in->w_id,
                                       ol_number_str,
                                       in->itemid[ol_number],
                                       in->supware[ol_number],
                                       in->qty[ol_number],
                                       out->ol_amount,
                                       out->ol_dist_info));
    if (errcode != 0) goto end;
  }

  snprintf(out->total, sizeof(out->total), "%f", total);
  OCITransCommit(svchp, errhp, (ub4) 0);

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
  /* if (sql6 != NULL) */
  /* { */
  /*   OCSQL_release(sql6); */
  /*   sql6 = NULL; */
  /* } */
  /* if (sql7 != NULL) */
  /* { */
  /*   OCSQL_release(sql7); */
  /*   sql7 = NULL; */
  /* } */
  /* if (sql8 != NULL) */
  /* { */
  /*   OCSQL_release(sql8); */
  /*   sql8 = NULL; */
  /* } */
  /* if (sql9 != NULL) */
  /* { */
  /*   OCSQL_release(sql9); */
  /*   sql9 = NULL; */
  /* } */

  return errcode;
}

void OCTACBenchNewOrder_afterTX(OCTACBenchNewOrderInOut *inout,
                                struct timeval think_time)
{
  OCTACConfig_sleepThinkTime(think_time);
}
