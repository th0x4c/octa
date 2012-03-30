/*
 * OCTABCreateIndex.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABCREATEINDEX_H_
#define _OCTABCREATEINDEX_H_

#define CREATE_INDEX_ACCOUNT_PK                                         \
  "CREATE UNIQUE INDEX account_pk \n"                                   \
  "ON account ( account_id ) \n"                                        \
  "TABLESPACE %s"

#define CREATE_INDEX_TELLER_PK                                          \
  "CREATE UNIQUE INDEX teller_pk \n"                                    \
  "ON teller ( teller_id ) \n"                                          \
  "TABLESPACE %s"

#define CREATE_INDEX_BRANCH_PK                                          \
  "CREATE UNIQUE INDEX branch_pk \n"                                    \
  "ON branch ( branch_id ) \n"                                          \
  "TABLESPACE %s"

#define OCTAB_CREATE_INDEX_INPUT                                        \
  {                                                                     \
    { CREATE_INDEX_ACCOUNT_PK,                                          \
      CREATE_INDEX_TELLER_PK,                                           \
      CREATE_INDEX_BRANCH_PK }, /* sqls */                              \
    3,                          /* sql_count */                         \
    ""                          /* tablespace */                        \
  }

#endif /* _OCTABCREATEINDEX_H_ */
