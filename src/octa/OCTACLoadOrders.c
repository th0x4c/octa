/*
 * OCTACLoadOrders.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoadOrders.h"

struct OCTACLoadOrdersInput
{
  char o_id[9];          /* NUMBER(8, 0) */
  char o_d_id[3];        /* NUMBER(2, 0) */
  char o_w_id[40];       /* NUMBER */
  char o_c_id[6];        /* NUMBER(5, 0) */
  char o_carrier_id[3];  /* NUMBER(2, 0) */
  char o_ol_cnt[3];      /* NUMBER(2, 0) */
  char ol_number[3];     /* NUMBER(2, 0) */
  char ol_i_id[7];       /* NUMBER(6, 0) */
  char ol_quantity[3];   /* NUMBER(2, 0) */
  char ol_amount[8];     /* NUMBER(6, 2) */
  char ol_dist_info[25]; /* CHAR(24) */
};
typedef struct OCTACLoadOrdersInput OCTACLoadOrdersInput;

void OCTACLoadOrders_beforeTX(int o_id, int o_d_id, int o_w_id, int o_c_id,
                              void **inout)
{
  OCTACLoadOrdersInput *in;

  in = malloc(sizeof(OCTACLoadOrdersInput));
  if (in == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACLoadOrdersInput\n");
    exit(1);
  }

  snprintf(in->o_id, sizeof(in->o_id), "%d", o_id);
  snprintf(in->o_d_id, sizeof(in->o_d_id), "%d", o_d_id);
  snprintf(in->o_w_id, sizeof(in->o_w_id), "%d", o_w_id);
  snprintf(in->o_c_id, sizeof(in->o_c_id), "%d", o_c_id);
  snprintf(in->o_carrier_id, sizeof(in->o_carrier_id), "%ld",
           TARandom_number(1, 10));
  snprintf(in->o_ol_cnt, sizeof(in->o_ol_cnt), "%ld", TARandom_number(5, 15));
  snprintf(in->ol_i_id, sizeof(in->ol_i_id), "%ld",
           TARandom_number(1, MAXITEMS));
  snprintf(in->ol_quantity, sizeof(in->ol_quantity), "%d", 5);
  /* 
   * OL_AMOUNT = 0.00 if OL_O_ID < 2,101, random within [0.01 .. 9,999.99]
   * otherwise
   */
  if (atol(in->o_id) > ORD_PER_DIST - 900)
    snprintf(in->ol_amount, sizeof(in->ol_amount), "%.2f",
             TARandom_number(1, 999999) / 100.0);
  else
    snprintf(in->ol_amount, sizeof(in->ol_amount), "%.2f", 0.00);

  TARandom_getAlphaString(in->ol_dist_info, 24, 24);

  *inout = in;
}

int OCTACLoadOrders_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                             void **inout, char *errmsg, size_t errmsgsize)
{
  OCTACLoadOrdersInput *in = (OCTACLoadOrdersInput *)*inout;
  OCSQL sql = NULL;
  int errcode = 0;
  long ol = 1;
  static char *insert_orders_sql_1 =
    "INSERT INTO orders (o_id, o_c_id, o_d_id, o_w_id, o_entry_d, "
    "o_carrier_id, o_ol_cnt, o_all_local) "
    "VALUES (:1, :2, :3, :4, SYSDATE, NULL, :5, 1)";
  static char *insert_new_order_sql =
    "INSERT INTO new_order (no_o_id, no_d_id, no_w_id) "
    "VALUES (:1, :2, :3)";
  static char *insert_orders_sql_2 =
    "INSERT INTO orders (o_id, o_c_id, o_d_id, o_w_id, o_entry_d, "
    "o_carrier_id, o_ol_cnt, o_all_local) "
    "VALUES (:1, :2, :3, :4, SYSDATE, :5, :6, 1)";
  static char *insert_order_line_sql_1 =
    "INSERT INTO order_line (ol_o_id, ol_d_id, ol_w_id, ol_number, ol_i_id, "
    "ol_supply_w_id, ol_quantity, ol_amount, ol_dist_info, ol_delivery_d) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9, NULL)";
  static char *insert_order_line_sql_2 =
    "INSERT INTO order_line (ol_o_id, ol_d_id, ol_w_id, ol_number, ol_i_id, "
    "ol_supply_w_id, ol_quantity, ol_amount, ol_dist_info, ol_delivery_d) "
    "VALUES (:1, :2, :3, :4, :5, :6, :7, :8, :9, SYSDATE)";
  static int num_inserts = 0;

  /* the last 900 orders have not been delivered */
  if (atol(in->o_id) > ORD_PER_DIST - 900)
  {
    /* orders */
    sql = OCSQL_initWithSQL(envhp, errhp, insert_orders_sql_1);
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql, errhp, svchp,
                                       in->o_id,
                                       in->o_c_id,
                                       in->o_d_id,
                                       in->o_w_id,
                                       in->o_ol_cnt));
    OCSQL_release(sql);
    if (errcode != 0)
      return errcode;

    /* new_order */
    sql = OCSQL_initWithSQL(envhp, errhp, insert_new_order_sql);
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql, errhp, svchp,
                                       in->o_id,
                                       in->o_d_id,
                                       in->o_w_id));
    OCSQL_release(sql);
    if (errcode != 0)
      return errcode;
  }
  else
  {
    /* orders */
    sql = OCSQL_initWithSQL(envhp, errhp, insert_orders_sql_2);
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql, errhp, svchp,
                                       in->o_id,
                                       in->o_c_id,
                                       in->o_d_id,
                                       in->o_w_id,
                                       in->o_carrier_id,
                                       in->o_ol_cnt));
    OCSQL_release(sql);
    if (errcode != 0)
      return errcode;
  }

  /* order_line */
  for (ol = 1; ol <= atol(in->o_ol_cnt); ol++)
  {
    snprintf(in->ol_number, sizeof(in->ol_number), "%ld", ol);

    if (atol(in->o_id) > ORD_PER_DIST - 900)
      sql = OCSQL_initWithSQL(envhp, errhp, insert_order_line_sql_1);
    else
      sql = OCSQL_initWithSQL(envhp, errhp, insert_order_line_sql_2);

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCSQL_execute(sql, errhp, svchp,
                                       in->o_id,
                                       in->o_d_id,
                                       in->o_w_id,
                                       in->ol_number,
                                       in->ol_i_id,
                                       in->o_w_id,
                                       in->ol_quantity,
                                       in->ol_amount,
                                       in->ol_dist_info));
    OCSQL_release(sql);
    if (errcode != 0)
      return errcode;
  }

  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

  return errcode;
}

void OCTACLoadOrders_afterTX(void **inout)
{
  OCTACLoadOrdersInput *in = (OCTACLoadOrdersInput *)*inout;

  free(in);
}
