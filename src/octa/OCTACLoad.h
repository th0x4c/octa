/*
 * OCTACLoad.h
 * OCTA
 *
 * Created by Takashi Hashizume on 04/01/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTACLOAD_H_
#define _OCTACLOAD_H_

#include <sys/sem.h> /* semop semget semctl */
#include <sys/shm.h> /* shmget shmat shmdt shmctl */
#include <stdio.h>   /* fprintf snprintf printf fflush */
#include <string.h>  /* strerror */
#include <stdlib.h>  /* exit malloc free */
#include <oci.h>     /* OCITransCommit OCITransRollback */
#include <OC/OC.h>
#include <TA/TA.h>
#include "config.h"
#include "OCTAOption.h"
#include "OCTACConfig.h"
#include "OCTACLoadItem.h"
#include "OCTACLoadWarehouse.h"
#include "OCTACLoadStock.h"
#include "OCTACLoadDistrict.h"
#include "OCTACLoadCustomer.h"
#include "OCTACLoadOrders.h"

int OCTACLoad_main(const OCTAOption *opt);

#endif /* _OCTACLOAD_H_ */
