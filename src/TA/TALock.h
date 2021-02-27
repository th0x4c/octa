/*
 * TALock.h
 * TA
 *
 * Created by Takashi Hashizume on 02/27/21.
 * Copyright 2021 Takashi Hashizume. All rights reserved.
 */

#ifndef _TALOCK_H_
#define _TALOCK_H_

#include <stdlib.h>  /* malloc exit free */
#include <sys/sem.h> /* semget semctl semop */
#include <string.h>  /* memset strerror */
#include <stdio.h>   /* fprintf */
#include <errno.h>   /* errno */
#include "TABool.h"

typedef struct __TALock *TALock;

TALock TALock_init();
void TALock_release(TALock self);
TABool TALock_lock(TALock self);
TABool TALock_unlock(TALock self);

#endif /* _TALOCK_H_ */
