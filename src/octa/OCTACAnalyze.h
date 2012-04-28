/*
 * OCTACAnalyze.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/28/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACANALYZE_H_
#define _OCTACANALYZE_H_

#define ANALYZE_WAREHOUSE                                               \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'WAREHOUSE', \n"          \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_DISTRICT                                                \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'DISTRICT', \n"           \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_CUSTOMER                                                \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'CUSTOMER', \n"           \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_HISTORY                                                 \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'HISTORY', \n"            \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_NEW_ORDER                                               \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'NEW_ORDER', \n"          \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_ORDERS                                                  \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'ORDERS', \n"             \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_ORDER_LINE                                              \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'ORDER_LINE', \n"         \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_ITEM                                                    \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'ITEM', \n"               \
  "                                cascade => true); \n"                \
  "END;"

#define ANALYZE_STOCK                                                   \
  "DECLARE \n"                                                          \
  "  username USER_USERS.USERNAME%TYPE; \n"                             \
  "BEGIN \n"                                                            \
  "  SELECT USERNAME into username FROM USER_USERS; \n"                 \
  "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username, \n"             \
  "                                tabname => 'STOCK', \n"              \
  "                                cascade => true); \n"                \
  "END;"

#define OCTAC_ANALYZE_INPUT                                             \
  {                                                                     \
    { ANALYZE_WAREHOUSE,                                                \
      ANALYZE_DISTRICT,                                                 \
      ANALYZE_CUSTOMER,                                                 \
      ANALYZE_HISTORY,                                                  \
      ANALYZE_NEW_ORDER,                                                \
      ANALYZE_ORDERS,                                                   \
      ANALYZE_ORDER_LINE,                                               \
      ANALYZE_ITEM,                                                     \
      ANALYZE_STOCK }, /* sqls */                                       \
    9,                 /* sql_count */                                  \
    ""                 /* tablespace */                                 \
  }

#endif /* _OCTACANALYZE_H_ */
