/*
 * OCTACDropTable.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACDROPTABLE_H_
#define _OCTACDROPTABLE_H_

#define DROP_TABLE_ORDER_LINE                                           \
  "DROP TABLE order_line"

#define DROP_TABLE_STOCK                                                \
  "DROP TABLE stock"

#define DROP_TABLE_ITEM                                                 \
  "DROP TABLE item"

#define DROP_TABLE_NEW_ORDER                                            \
  "DROP TABLE new_order"

#define DROP_TABLE_ORDERS                                               \
  "DROP TABLE orders"

#define DROP_TABLE_HISTORY                                              \
  "DROP TABLE history"

#define DROP_TABLE_CUSTOMER                                             \
  "DROP TABLE customer"

#define DROP_TABLE_DISTRICT                                             \
  "DROP TABLE district"

#define DROP_TABLE_WAREHOUSE                                            \
  "DROP TABLE warehouse"

#define OCTAC_DROP_TABLE_INPUT                                          \
  {                                                                     \
    { DROP_TABLE_ORDER_LINE,                                            \
      DROP_TABLE_STOCK,                                                 \
      DROP_TABLE_ITEM,                                                  \
      DROP_TABLE_NEW_ORDER,                                             \
      DROP_TABLE_ORDERS,                                                \
      DROP_TABLE_HISTORY,                                               \
      DROP_TABLE_CUSTOMER,                                              \
      DROP_TABLE_DISTRICT,                                              \
      DROP_TABLE_WAREHOUSE }, /* sqls */                                \
    9,                        /* sql_count */                           \
    ""                        /* tablespace */                          \
  }

#endif /* _OCTACDROPTABLE_H_ */
