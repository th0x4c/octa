/*
 * OCTABAddConstraint.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABADDCONSTRAINT_H_
#define _OCTABADDCONSTRAINT_H_

/* Primary key */
#define ADD_CONSTRAINT_ACCOUNT_PK                                       \
  "ALTER TABLE account \n"                                              \
  "ADD CONSTRAINT account_pk \n"                                        \
  "PRIMARY KEY ( account_id ) \n"                                       \
  "USING INDEX account_pk"

#define ADD_CONSTRAINT_TELLER_PK                                        \
  "ALTER TABLE teller \n"                                               \
  "ADD CONSTRAINT teller_pk \n"                                         \
  "PRIMARY KEY ( teller_id ) \n"                                        \
  "USING INDEX teller_pk"

#define ADD_CONSTRAINT_BRANCH_PK                                        \
  "ALTER TABLE branch \n"                                               \
  "ADD CONSTRAINT branch_pk \n"                                         \
  "PRIMARY KEY ( branch_id ) \n"                                        \
  "USING INDEX branch_pk"

/* Foreign key */
#define ADD_CONSTRAINT_ACCOUNT_FK                                       \
  "ALTER TABLE account \n"                                              \
  "ADD CONSTRAINT account_fk \n"                                        \
  "FOREIGN KEY ( branch_id ) \n"                                        \
  "REFERENCES branch ( branch_id )"

#define ADD_CONSTRAINT_TELLER_FK                                        \
  "ALTER TABLE teller \n"                                               \
  "ADD CONSTRAINT teller_fk \n"                                         \
  "FOREIGN KEY ( branch_id ) \n"                                        \
  "REFERENCES branch ( branch_id )"

#define ADD_CONSTRAINT_HISTORY_FK1                                      \
  "ALTER TABLE history \n"                                              \
  "ADD CONSTRAINT history_fk1 \n"                                       \
  "FOREIGN KEY ( branch_id ) \n"                                        \
  "REFERENCES branch ( branch_id )"

#define ADD_CONSTRAINT_HISTORY_FK2                                      \
  "ALTER TABLE history \n"                                              \
  "ADD CONSTRAINT history_fk2 \n"                                       \
  "FOREIGN KEY ( teller_id ) \n"                                        \
  "REFERENCES teller ( teller_id )"

#define ADD_CONSTRAINT_HISTORY_FK3                                      \
  "ALTER TABLE history \n"                                              \
  "ADD CONSTRAINT history_fk3 \n"                                       \
  "FOREIGN KEY ( account_id ) \n"                                       \
  "REFERENCES account ( account_id )"

#define OCTAB_ADD_CONSTRAINT_INPUT                                      \
  {                                                                     \
    { ADD_CONSTRAINT_ACCOUNT_PK,                                        \
      ADD_CONSTRAINT_TELLER_PK,                                         \
      ADD_CONSTRAINT_BRANCH_PK,                                         \
      ADD_CONSTRAINT_ACCOUNT_FK,                                        \
      ADD_CONSTRAINT_TELLER_FK,                                         \
      ADD_CONSTRAINT_HISTORY_FK1,                                       \
      ADD_CONSTRAINT_HISTORY_FK2,                                       \
      ADD_CONSTRAINT_HISTORY_FK3 }, /* sqls */                          \
    8,                              /* sql_count */                     \
    ""                              /* tablespace */                    \
  }

#endif /* _OCTABADDCONSTRAINT_H_ */
