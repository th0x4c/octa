/*
 * OCTABLoad.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTABLOAD_H_
#define _OCTABLOAD_H_

#include <sys/sem.h> /* semop semget semctl */
#include <sys/shm.h> /* shmget shmat shmdt shmctl */
#include <stdio.h>   /* fprintf snprintf */
#include <string.h>  /* strerror */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCIHandleAlloc OCIStmtPrepare OCIBindByPos  *
                      * OCIStmtExecute OCIHandleFree OCITransCommit */
#include <OC/OC.h>
#include <TA/TA.h>
#include "config.h"
#include "OCTAOption.h"

#define ACCOUNTS_PER_BRANCH 100000
#define TELLERS_PER_BRANCH 10
#define INSERTS_PER_COMMIT 100

int OCTABLoad_main(const OCTAOption *opt);

#endif /* _OCTABLOAD_H_ */
