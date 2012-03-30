/*
 * OCTABDropTable.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABDROPTABLE_H_
#define _OCTABDROPTABLE_H_

#define DROP_TABLE_HISTORY                                              \
  "DROP TABLE history"

#define DROP_TABLE_ACCOUNT                                              \
  "DROP TABLE account"

#define DROP_TABLE_TELLER                                               \
  "DROP TABLE teller"

#define DROP_TABLE_BRANCH                                               \
  "DROP TABLE branch"

#define OCTAB_DROP_TABLE_INPUT                                          \
  {                                                                     \
    { DROP_TABLE_HISTORY,                                               \
      DROP_TABLE_ACCOUNT,                                               \
      DROP_TABLE_TELLER,                                                \
      DROP_TABLE_BRANCH }, /* sqls */                                   \
    4,                     /* sql_count */                              \
    ""                     /* tablespace */                             \
  }

#endif /* _OCTABDROPTABLE_H_ */
