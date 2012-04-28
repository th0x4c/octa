/*
 * OCTACAddConstraint.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACADDCONSTRAINT_H_
#define _OCTACADDCONSTRAINT_H_

/* Primary key */
#define ADD_CONSTRAINT_WAREHOUSE_PK                                     \
  "ALTER TABLE warehouse ADD CONSTRAINT warehouse_pk \n"                \
  "  PRIMARY KEY ( w_id ) \n"                                           \
  "  USING INDEX warehouse_pk"

#define ADD_CONSTRAINT_DISTRICT_PK                                      \
  "ALTER TABLE district ADD CONSTRAINT district_pk \n"                  \
  "  PRIMARY KEY ( d_w_id, d_id ) \n"                                   \
  "  USING INDEX district_pk"

#define ADD_CONSTRAINT_CUSTOMER_PK                                      \
  "ALTER TABLE customer ADD CONSTRAINT customer_pk \n"                  \
  "  PRIMARY KEY ( c_w_id, c_d_id, c_id ) \n"                           \
  "  USING INDEX customer_pk"

#define ADD_CONSTRAINT_NEW_ORDER_PK                                     \
  "ALTER TABLE new_order ADD CONSTRAINT new_order_pk \n"                \
  "  PRIMARY KEY ( no_w_id, no_d_id, no_o_id ) \n"                      \
  "  USING INDEX new_order_pk"

#define ADD_CONSTRAINT_ORDERS_PK                                        \
  "ALTER TABLE orders ADD CONSTRAINT orders_pk \n"                      \
  "  PRIMARY KEY ( o_w_id, o_d_id, o_id ) \n"                           \
  "  USING INDEX orders_pk"

#define ADD_CONSTRAINT_ORDER_LINE_PK                                    \
  "ALTER TABLE order_line ADD CONSTRAINT order_line_pk \n"              \
  "  PRIMARY KEY ( ol_w_id, ol_d_id, ol_o_id, ol_number ) \n"           \
  "  USING INDEX order_line_pk"

#define ADD_CONSTRAINT_ITEM_PK                                          \
  "ALTER TABLE item ADD CONSTRAINT item_pk \n"                          \
  "  PRIMARY KEY ( i_id ) \n"                                           \
  "  USING INDEX item_pk"

#define ADD_CONSTRAINT_STOCK_PK                                         \
  "ALTER TABLE stock ADD CONSTRAINT stock_pk \n"                        \
  "  PRIMARY KEY ( s_w_id, s_i_id ) \n"                                 \
  "  USING INDEX stock_pk"

/* Foreign key */
#define ADD_CONSTRAINT_DISTRICT_FK                                      \
  "ALTER TABLE district ADD CONSTRAINT district_fk \n"                  \
  "  FOREIGN KEY ( d_w_id ) \n"                                         \
  "  REFERENCES warehouse ( w_id )"

#define ADD_CONSTRAINT_CUSTOMER_FK                                      \
  "ALTER TABLE customer ADD CONSTRAINT customer_fk \n"                  \
  "  FOREIGN KEY ( c_w_id, c_d_id ) \n"                                 \
  "  REFERENCES district ( d_w_id, d_id )"

#define ADD_CONSTRAINT_HISTORY_FK1                                      \
  "ALTER TABLE history ADD CONSTRAINT history_fk1 \n"                   \
  "  FOREIGN KEY ( h_c_w_id, h_c_d_id, h_c_id ) \n"                     \
  "  REFERENCES customer ( c_w_id, c_d_id, c_id )"

#define ADD_CONSTRAINT_HISTORY_FK2                                      \
  "ALTER TABLE history ADD CONSTRAINT history_fk2 \n"                   \
  "  FOREIGN KEY ( h_w_id, h_d_id ) \n"                                 \
  "  REFERENCES district ( d_w_id, d_id )"

#define ADD_CONSTRAINT_NEW_ORDER_FK                                     \
  "ALTER TABLE new_order ADD CONSTRAINT new_order_fk \n"                \
  "  FOREIGN KEY ( no_w_id, no_d_id, no_o_id ) \n"                      \
  "  REFERENCES orders ( o_w_id, o_d_id, o_id )"

#define ADD_CONSTRAINT_ORDERS_FK                                        \
  "ALTER TABLE orders ADD CONSTRAINT orders_fk \n"                      \
  "  FOREIGN KEY ( o_w_id, o_d_id, o_c_id ) \n"                         \
  "  REFERENCES customer ( c_w_id, c_d_id, c_id )"

#define ADD_CONSTRAINT_ORDER_LINE_FK1                                   \
  "ALTER TABLE order_line ADD CONSTRAINT order_line_fk1 \n"             \
  "  FOREIGN KEY ( ol_w_id, ol_d_id, ol_o_id ) \n"                      \
  "  REFERENCES orders ( o_w_id, o_d_id, o_id )"

#define ADD_CONSTRAINT_ORDER_LINE_FK2                                   \
  "ALTER TABLE order_line ADD CONSTRAINT order_line_fk2 \n"             \
  "  FOREIGN KEY ( ol_supply_w_id, ol_i_id ) \n"                        \
  "  REFERENCES stock ( s_w_id, s_i_id )"

#define ADD_CONSTRAINT_STOCK_FK1                                        \
  "ALTER TABLE stock ADD CONSTRAINT stock_fk1 \n"                       \
  "  FOREIGN KEY ( s_w_id ) \n"                                         \
  "  REFERENCES warehouse ( w_id )"

#define ADD_CONSTRAINT_STOCK_FK2                                        \
  "ALTER TABLE stock ADD CONSTRAINT stock_fk2 \n"                       \
  "  FOREIGN KEY ( s_i_id ) \n"                                         \
  "  REFERENCES item ( i_id )"

#define OCTAC_ADD_CONSTRAINT_INPUT                                      \
  {                                                                     \
    { ADD_CONSTRAINT_WAREHOUSE_PK,                                      \
      ADD_CONSTRAINT_DISTRICT_PK,                                       \
      ADD_CONSTRAINT_CUSTOMER_PK,                                       \
      ADD_CONSTRAINT_NEW_ORDER_PK,                                      \
      ADD_CONSTRAINT_ORDERS_PK,                                         \
      ADD_CONSTRAINT_ORDER_LINE_PK,                                     \
      ADD_CONSTRAINT_ITEM_PK,                                           \
      ADD_CONSTRAINT_STOCK_PK,                                          \
      ADD_CONSTRAINT_DISTRICT_FK,                                       \
      ADD_CONSTRAINT_CUSTOMER_FK,                                       \
      ADD_CONSTRAINT_HISTORY_FK1,                                       \
      ADD_CONSTRAINT_HISTORY_FK2,                                       \
      ADD_CONSTRAINT_NEW_ORDER_FK,                                      \
      ADD_CONSTRAINT_ORDERS_FK,                                         \
      ADD_CONSTRAINT_ORDER_LINE_FK1,                                    \
      ADD_CONSTRAINT_ORDER_LINE_FK2,                                    \
      ADD_CONSTRAINT_STOCK_FK1,                                         \
      ADD_CONSTRAINT_STOCK_FK2 }, /* sqls */                            \
    18,                           /* sql_count */                       \
    ""                            /* tablespace */                      \
  }

#endif /* _OCTACADDCONSTRAINT_H_ */
