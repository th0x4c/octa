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
#include <TA/TA.h>

#define CNUM 1
#define MAXITEMS 100000
#define DIST_PER_WARE 10
#define CUST_PER_DIST 3000
#define ORD_PER_DIST 3000
#define INSERTS_PER_COMMIT 100

void OCTACConfig_insertOriginal(char *data);
void OCTACConfig_makeAddress(char *street_1, char *street_2, char *city,
                             char *state, char *zip);
void OCTACConfig_lastname(long num, char *name);
void OCTACConfig_initPermutation(long seq[], size_t seqsize);

#endif /* _OCTACCONFIG_H_ */
