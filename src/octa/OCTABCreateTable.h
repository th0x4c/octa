/*
 * OCTABCreateTable.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABCREATETABLE_H_
#define _OCTABCREATETABLE_H_

#define CREATE_TABLE_ACCOUNT "CREATE TABLE account \n"             \
                             "( \n"                                \
                             "  account_id       NUMBER(10,0), \n" \
                             "  branch_id        NUMBER(10,0), \n" \
                             "  account_balance  NUMBER(10,0), \n" \
                             "  filler           VARCHAR2(97) \n"  \
                             ") \n"                                \
                             "TABLESPACE %s"

#define CREATE_TABLE_TELLER "CREATE TABLE teller \n"             \
                            "( \n"                               \
                            "  teller_id       NUMBER(10,0), \n" \
                            "  branch_id       NUMBER(10,0), \n" \
                            "  teller_balance  NUMBER(10,0), \n" \
                            "  filler          CHAR(97) \n"      \
                             ") \n"                              \
                             "TABLESPACE %s"

#define CREATE_TABLE_BRANCH "CREATE TABLE branch \n"             \
                            "( \n"                               \
                            "  branch_id       NUMBER(10,0), \n" \
                            "  branch_balance  NUMBER(10,0), \n" \
                            "  filler          CHAR(98) \n"      \
                             ") \n"                              \
                             "TABLESPACE %s"

#define CREATE_TABLE_HISTORY "CREATE TABLE history \n"           \
                             "( \n"                              \
                             "  teller_id         NUMBER, \n"    \
                             "  branch_id         NUMBER, \n"    \
                             "  account_id        NUMBER, \n"    \
                             "  amount            NUMBER, \n"    \
                             "  timestamp         TIMESTAMP, \n" \
                             "  filler            CHAR(39) \n"   \
                             ") \n"                              \
                             "TABLESPACE %s"


#define CREATE_TABLE_INPUT {                                      \
                             { CREATE_TABLE_ACCOUNT,              \
                               CREATE_TABLE_TELLER,               \
                               CREATE_TABLE_BRANCH,               \
                               CREATE_TABLE_HISTORY }, /* sqls */ \
                             4,       /* sql_count */             \
                             ""       /* tablespace */            \
                           }

#endif /* _OCTABCREATETABLE_H_ */
