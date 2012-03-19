/*
 * OCTABAnalyze.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABANALYZE_H_
#define _OCTABANALYZE_H_

#define ANALYZE_ACCOUNT "DECLARE \n"                                          \
                        "  username USER_USERS.USERNAME%TYPE; \n"             \
                        "BEGIN \n"                                            \
                        "  SELECT USERNAME into username FROM USER_USERS; \n" \
                        "  DBMS_STATS.GATHER_TABLE_STATS( \n"                 \
                        "               ownname => username, \n"              \
                        "               tabname => 'ACCOUNT', \n"             \
                        "               cascade => true); \n"                 \
                        "END;"

#define ANALYZE_TELLER "DECLARE \n"                                          \
                       "  username USER_USERS.USERNAME%TYPE; \n"             \
                       "BEGIN \n"                                            \
                       "  SELECT USERNAME into username FROM USER_USERS; \n" \
                       "  DBMS_STATS.GATHER_TABLE_STATS( \n"                 \
                       "               ownname => username, \n"              \
                       "               tabname => 'TELLER', \n"              \
                       "               cascade => true); \n"                 \
                       "END;"

#define ANALYZE_BRANCH "DECLARE \n"                                          \
                       "  username USER_USERS.USERNAME%TYPE; \n"             \
                       "BEGIN \n"                                            \
                       "  SELECT USERNAME into username FROM USER_USERS; \n" \
                       "  DBMS_STATS.GATHER_TABLE_STATS( \n"                 \
                       "               ownname => username, \n"              \
                       "               tabname => 'BRANCH', \n"              \
                       "               cascade => true); \n"                 \
                       "END;"

#define ANALYZE_INPUT {                                \
                        { ANALYZE_ACCOUNT,             \
                          ANALYZE_TELLER,              \
                          ANALYZE_BRANCH }, /* sqls */ \
                        3,       /* sql_count */       \
                        ""       /* tablespace */      \
                      }

#endif /* _OCTABANALYZE_H_ */
