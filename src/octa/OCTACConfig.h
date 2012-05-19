/*
 * OCTACConfig.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi hashizume. All rights reserved.
 */

#ifndef _OCTACCONFIG_H_
#define _OCTACCONFIG_H_

#include <string.h> /* strlen strcpy strcat */
#include <stdio.h>  /* snpritf */
#include <time.h>   /* nanosleep */
#include <math.h>   /* log */
#include <TA/TA.h>

#define CNUM 1
#define MAXITEMS 100000
#define DIST_PER_WARE 10
#define CUST_PER_DIST 3000
#define ORD_PER_DIST 3000

#define INSERTS_PER_COMMIT 100

#define IDX_NEW_ORDER    0
#define IDX_PAYMENT      1
#define IDX_ORDER_STATUS 2
#define IDX_DELIVERY     3
#define IDX_STOCK_LEVEL  4

#define DEFAULT_PERCENTAGE_NEW_ORDER    45
#define DEFAULT_PERCENTAGE_PAYMENT      43
#define DEFAULT_PERCENTAGE_ORDER_STATUS  4
#define DEFAULT_PERCENTAGE_DELIVERY      4
#define DEFAULT_PERCENTAGE_STOCK_LEVEL   4
#define DEFAULT_PERCENTAGES                                             \
 { DEFAULT_PERCENTAGE_NEW_ORDER,                                        \
   DEFAULT_PERCENTAGE_PAYMENT,                                          \
   DEFAULT_PERCENTAGE_ORDER_STATUS,                                     \
   DEFAULT_PERCENTAGE_DELIVERY,                                         \
   DEFAULT_PERCENTAGE_STOCK_LEVEL }

#define DEFAULT_KEYING_TIME_NEW_ORDER    18000
#define DEFAULT_KEYING_TIME_PAYMENT       3000
#define DEFAULT_KEYING_TIME_ORDER_STATUS  2000
#define DEFAULT_KEYING_TIME_DELIVERY      2000
#define DEFAULT_KEYING_TIME_STOCK_LEVEL   2000
#define DEFAULT_KEYING_TIMES                                            \
  { DEFAULT_KEYING_TIME_NEW_ORDER,                                      \
    DEFAULT_KEYING_TIME_PAYMENT,                                        \
    DEFAULT_KEYING_TIME_ORDER_STATUS,                                   \
    DEFAULT_KEYING_TIME_DELIVERY,                                       \
    DEFAULT_KEYING_TIME_STOCK_LEVEL }

#define DEFAULT_THINK_TIME_NEW_ORDER    12000
#define DEFAULT_THINK_TIME_PAYMENT      12000
#define DEFAULT_THINK_TIME_ORDER_STATUS 10000
#define DEFAULT_THINK_TIME_DELIVERY      5000
#define DEFAULT_THINK_TIME_STOCK_LEVEL   5000
#define DEFAULT_THINK_TIMES                                             \
  { DEFAULT_THINK_TIME_NEW_ORDER,                                       \
    DEFAULT_THINK_TIME_PAYMENT,                                         \
    DEFAULT_THINK_TIME_ORDER_STATUS,                                    \
    DEFAULT_THINK_TIME_DELIVERY,                                        \
    DEFAULT_THINK_TIME_STOCK_LEVEL }

#define UNUSED_I_ID -1
#define INVALID_ITEM_ERROR_CODE -1
#define INVALID_ITEM_ERROR_MESSAGE " (Item number is not valid)"

#define LOGFILE "octa_tpcc.log"

void OCTACConfig_insertOriginal(char *data);
void OCTACConfig_makeAddress(char *street_1, char *street_2, char *city,
                             char *state, char *zip);
void OCTACConfig_lastname(long num, char *name);
long OCTACConfig_NURand(long a, long x, long y);
void OCTACConfig_initPermutation(long seq[], size_t seqsize);
long OCTACConfig_homeWID(int scale_factor, int session_id);
void OCTACConfig_sleepKeyingTime(struct timeval keying_timeval);
void OCTACConfig_sleepThinkTime(struct timeval think_timeval);

#endif /* _OCTACCONFIG_H_ */
