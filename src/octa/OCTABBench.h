/*
 * OCTABBench.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/20/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABBENCH_H_
#define _OCTABBENCH_H_

#include <stdio.h>    /* printf snprintf */
#include <stdlib.h>   /* srandom random exit malloc free */
#include <string.h>   /* memset strlen */
#include <unistd.h>   /* getpid */
#include <time.h>     /* nanosleep */
#include <sys/time.h> /* gettimeofday */
#include <oci.h>      /* OCIHandleAlloc OCIStmtPrepare OCIBindByPos *
                       * OCIStmtExecute OCIStmtFetch2 OCIHandleFree *
                       * OCITransCommit OCITransRollback            */
#include <OC/OC.h>
#include <TA/TA.h>
#include "config.h"
#include "OCTAOption.h"

#define ACCOUNTS_PER_BRANCH 100000
#define TELLERS_PER_BRANCH 10
#define LOCAL_BRANCH_PERCENT 85

#define UPDATE_ACCOUNT_SQL "UPDATE account "                             \
                           "SET account_balance = account_balance + :1 " \
                           "WHERE account_id = :2"
#define SELECT_ACCOUNT_SQL "SELECT account_balance " \
                           "FROM account "           \
                           "WHERE account_id = :1"
#define UPDATE_TELLER_SQL "UPDATE teller "                            \
                          "SET teller_balance = teller_balance + :1 " \
                          "WHERE teller_id = :2"
#define UPDATE_BRANCH_SQL "UPDATE branch "                            \
                          "SET branch_balance = branch_balance + :1 " \
                          "WHERE branch_id = :2"
#define INSERT_HISTORY_SQL "INSERT INTO history VALUES "        \
                           "(:1, :2, :3, :4, SYSTIMESTAMP, :5)"

int OCTABBench_main(const OCTAOption *opt);

#endif /* _OCTABBENCH_H_ */
