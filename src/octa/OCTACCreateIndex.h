/*
 * OCTACCreateIndex.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACCREATEINDEX_H_
#define _OCTACCREATEINDEX_H_

#define CREATE_INDEX_WAREHOUSE_PK                                       \
  "CREATE UNIQUE INDEX warehouse_pk \n"                                 \
  "  ON warehouse ( w_id ) \n"                                          \
  "  TABLESPACE %s"

#define CREATE_INDEX_DISTRICT_PK                                        \
  "CREATE UNIQUE INDEX district_pk \n"                                  \
  "  ON district ( d_w_id, d_id ) \n"                                   \
  "  TABLESPACE %s"

#define CREATE_INDEX_CUSTOMER_PK                                        \
  "CREATE UNIQUE INDEX customer_pk \n"                                  \
  "  ON customer ( c_w_id, c_d_id, c_id ) \n"                           \
  "  TABLESPACE %s"

#define CREATE_INDEX_NEW_ORDER_PK                                       \
  "CREATE UNIQUE INDEX new_order_pk \n"                                 \
  "  ON new_order ( no_w_id, no_d_id, no_o_id ) \n"                     \
  "  TABLESPACE %s"

#define CREATE_INDEX_ORDERS_PK                                          \
  "CREATE UNIQUE INDEX orders_pk \n"                                    \
  "  ON orders ( o_w_id, o_d_id, o_id ) \n"                             \
  "  TABLESPACE %s"

#define CREATE_INDEX_ORDER_LINE_PK                                      \
  "CREATE UNIQUE INDEX order_line_pk \n"                                \
  "  ON order_line ( ol_w_id, ol_d_id, ol_o_id, ol_number ) \n"         \
  "  TABLESPACE %s"

#define CREATE_INDEX_ITEM_PK                                            \
  "CREATE UNIQUE INDEX item_pk \n"                                      \
  "  ON item ( i_id ) \n"                                               \
  "  TABLESPACE %s"

#define CREATE_INDEX_STOCK_PK                                           \
  "CREATE UNIQUE INDEX stock_pk \n"                                     \
  "  ON stock ( s_w_id, s_i_id ) \n"                                    \
  "  TABLESPACE %s"

/* Additional index */
#define CREATE_INDEX_CUSTOMER_IDX                                       \
  "CREATE UNIQUE INDEX customer_idx \n"                                 \
  "  ON customer ( c_last, c_w_id, c_d_id, c_first, c_id ) \n"          \
  "  TABLESPACE %s"

#define CREATE_INDEX_ORDERS_IDX                                         \
  "CREATE UNIQUE INDEX orders_idx \n"                                   \
  "  ON orders ( o_c_id, o_d_id, o_w_id, o_id ) \n"                     \
  "  TABLESPACE %s"

#define OCTAC_CREATE_INDEX_INPUT                                        \
  {                                                                     \
    { CREATE_INDEX_WAREHOUSE_PK,                                        \
      CREATE_INDEX_DISTRICT_PK,                                         \
      CREATE_INDEX_CUSTOMER_PK,                                         \
      CREATE_INDEX_NEW_ORDER_PK,                                        \
      CREATE_INDEX_ORDERS_PK,                                           \
      CREATE_INDEX_ORDER_LINE_PK,                                       \
      CREATE_INDEX_ITEM_PK,                                             \
      CREATE_INDEX_STOCK_PK,                                            \
      CREATE_INDEX_CUSTOMER_IDX,                                        \
      CREATE_INDEX_ORDERS_IDX }, /* sqls */                             \
    10,                          /* sql_count */                        \
    ""                           /* tablespace */                       \
  }

#endif /* _OCTACCREATEINDEX_H_ */
