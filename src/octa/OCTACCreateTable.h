/*
 * OCTACCreateTable.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACCREATETABLE_H_
#define _OCTACCREATETABLE_H_

#define CREATE_TABLE_WAREHOUSE                                          \
  "CREATE TABLE warehouse \n"                                           \
  "( \n"                                                                \
  "  w_id       NUMBER,       -- 2*W unique IDs \n"                     \
  "  w_name     VARCHAR2(10), \n"                                       \
  "  w_street_1 VARCHAR2(20), \n"                                       \
  "  w_street_2 VARCHAR2(20), \n"                                       \
  "  w_city     VARCHAR2(20), \n"                                       \
  "  w_state    CHAR(2), \n"                                            \
  "  w_zip      CHAR(9), \n"                                            \
  "  w_tax      NUMBER(4, 4), \n"                                       \
  "  w_ytd      NUMBER(12, 2) \n"                                       \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_DISTRICT                                           \
  "CREATE TABLE district \n"                                            \
  "( \n"                                                                \
  "  d_id        NUMBER(2, 0), -- 20 unique IDs \n"                     \
  "  d_w_id      NUMBER,       -- 2*W unique IDs \n"                    \
  "  d_name      VARCHAR2(10), \n"                                      \
  "  d_street_1  VARCHAR2(20), \n"                                      \
  "  d_street_2  VARCHAR2(20), \n"                                      \
  "  d_city      VARCHAR2(20), \n"                                      \
  "  d_state     CHAR(2), \n"                                           \
  "  d_zip       CHAR(9), \n"                                           \
  "  d_tax       NUMBER(4, 4), \n"                                      \
  "  d_ytd       NUMBER(12, 2), \n"                                     \
  "  d_next_o_id NUMBER(8, 0)  -- 10,000,000 unique IDs \n"             \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_CUSTOMER                                           \
  "CREATE TABLE customer \n"                                            \
  "( \n"                                                                \
  "  c_id           NUMBER(5, 0), -- 96,000 unique IDs \n"              \
  "  c_d_id         NUMBER(2, 0), -- 20 unique IDs \n"                  \
  "  c_w_id         NUMBER,       -- 2*W unique IDs \n"                 \
  "  c_first        VARCHAR2(16), \n"                                   \
  "  c_middle       CHAR(2), \n"                                        \
  "  c_last         VARCHAR2(16), \n"                                   \
  "  c_street_1     VARCHAR2(20), \n"                                   \
  "  c_street_2     VARCHAR2(20), \n"                                   \
  "  c_city         VARCHAR2(20), \n"                                   \
  "  c_state        CHAR(2), \n"                                        \
  "  c_zip          CHAR(9), \n"                                        \
  "  c_phone        CHAR(16), \n"                                       \
  "  c_since        DATE, \n"                                           \
  "  c_credit       CHAR(2), \n"                                        \
  "  c_credit_lim   NUMBER(12, 2), \n"                                  \
  "  c_discount     NUMBER(4, 4), \n"                                   \
  "  c_balance      NUMBER(12, 2), \n"                                  \
  "  c_ytd_payment  NUMBER(12, 2), \n"                                  \
  "  c_payment_cnt  NUMBER(4, 0), \n"                                   \
  "  c_delivery_cnt NUMBER(4, 0), \n"                                   \
  "  c_data         VARCHAR2(500) \n"                                   \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_HISTORY                                            \
  "CREATE TABLE history \n"                                             \
  "( \n"                                                                \
  "  h_c_id   NUMBER(5, 0), -- 96,000 unique IDs \n"                    \
  "  h_c_d_id NUMBER(2, 0), -- 20 unique IDs \n"                        \
  "  h_c_w_id NUMBER,       -- 2*W unique IDs \n"                       \
  "  h_d_id   NUMBER(2, 0), -- 20 unique IDs \n"                        \
  "  h_w_id   NUMBER,       -- 2*W unique IDs \n"                       \
  "  h_date   DATE, \n"                                                 \
  "  h_amount NUMBER(6, 2), \n"                                         \
  "  h_data   VARCHAR2(24) \n"                                          \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_NEW_ORDER                                          \
  "CREATE TABLE new_order \n"                                           \
  "( \n"                                                                \
  "  no_o_id NUMBER(8, 0), -- 10,000,000 unique IDs \n"                 \
  "  no_d_id NUMBER(2, 0), -- 20 unique IDs \n"                         \
  "  no_w_id NUMBER        -- 2*W unique IDs \n"                        \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_ORDERS                                             \
  "CREATE TABLE orders \n"                                              \
  "( \n"                                                                \
  "  o_id         NUMBER(8, 0), -- 10,000,000 unique IDs \n"            \
  "  o_d_id       NUMBER(2, 0), -- 20 unique IDs \n"                    \
  "  o_w_id       NUMBER,       -- 2*W unique IDs \n"                   \
  "  o_c_id       NUMBER(5, 0), -- 96,000 unique IDs \n"                \
  "  o_entry_d    DATE, \n"                                             \
  "  o_carrier_id NUMBER(2, 0), -- 10 unique IDs, or null \n"           \
  "  o_ol_cnt     NUMBER(2, 0), \n"                                     \
  "  o_all_local  NUMBER(1, 0) \n"                                      \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_ORDER_LINE                                         \
  "CREATE TABLE order_line \n"                                          \
  "( \n"                                                                \
  "  ol_o_id        NUMBER(8, 0), -- 10,000,000 unique IDs \n"          \
  "  ol_d_id        NUMBER(2, 0), -- 20 unique IDs \n"                  \
  "  ol_w_id        NUMBER,       -- 2*W unique IDs \n"                 \
  "  ol_number      NUMBER(2, 0), -- 15 unique IDs \n"                  \
  "  ol_i_id        NUMBER(6, 0), -- 200,000 unique IDs \n"             \
  "  ol_supply_w_id NUMBER,       -- 2*W unique IDs \n"                 \
  "  ol_delivery_d  DATE,         -- date and time, or null \n"         \
  "  ol_quantity    NUMBER(2, 0), \n"                                   \
  "  ol_amount      NUMBER(6, 2), \n"                                   \
  "  ol_dist_info   CHAR(24) \n"                                        \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_ITEM                                               \
  "CREATE TABLE item \n"                                                \
  "( \n"                                                                \
  "  i_id    NUMBER(6, 0), -- 200,000 unique IDs \n"                    \
  "  i_im_id NUMBER(6, 0), -- 200,000 unique IDs \n"                    \
  "  i_name  VARCHAR2(24), \n"                                          \
  "  i_price NUMBER(5, 2), \n"                                          \
  "  i_data  VARCHAR2(50) \n"                                           \
  ") \n"                                                                \
  "TABLESPACE %s"

#define CREATE_TABLE_STOCK                                              \
  "CREATE TABLE stock \n"                                               \
  "( \n"                                                                \
  "  s_i_id       NUMBER(6, 0), -- 200,000 unique IDs \n"               \
  "  s_w_id       NUMBER,       -- 2*W unique IDs \n"                   \
  "  s_quantity   NUMBER(4, 0), \n"                                     \
  "  s_dist_01    CHAR(24), \n"                                         \
  "  s_dist_02    CHAR(24), \n"                                         \
  "  s_dist_03    CHAR(24), \n"                                         \
  "  s_dist_04    CHAR(24), \n"                                         \
  "  s_dist_05    CHAR(24), \n"                                         \
  "  s_dist_06    CHAR(24), \n"                                         \
  "  s_dist_07    CHAR(24), \n"                                         \
  "  s_dist_08    CHAR(24), \n"                                         \
  "  s_dist_09    CHAR(24), \n"                                         \
  "  s_dist_10    CHAR(24), \n"                                         \
  "  s_ytd        NUMBER(8, 0), \n"                                     \
  "  s_order_cnt  NUMBER(4, 0), \n"                                     \
  "  s_remote_cnt NUMBER(4, 0), \n"                                     \
  "  s_data       VARCHAR2(50) \n"                                      \
  ") \n"                                                                \
  "TABLESPACE %s"

#define OCTAC_CREATE_TABLE_INPUT                                        \
  {                                                                     \
    { CREATE_TABLE_WAREHOUSE,                                           \
      CREATE_TABLE_DISTRICT,                                            \
      CREATE_TABLE_CUSTOMER,                                            \
      CREATE_TABLE_HISTORY,                                             \
      CREATE_TABLE_NEW_ORDER,                                           \
      CREATE_TABLE_ORDERS,                                              \
      CREATE_TABLE_ORDER_LINE,                                          \
      CREATE_TABLE_ITEM,                                                \
      CREATE_TABLE_STOCK }, /* sqls */                                  \
    9,                      /* sql_count */                             \
    ""                      /* tablespace */                            \
  }

#endif /* _OCTACCREATETABLE_H_ */
