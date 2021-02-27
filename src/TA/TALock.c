/*
 * TALock.c
 * TA
 *
 * Created by Takashi Hashizume on 02/27/21.
 * Copyright 2021 Takashi Hashizume. All rights reserved.
 */

#include "TALock.h"
#include "config.h"

struct __TALock
{
  int semid;
};
typedef struct __TALock __TALock;

#ifndef HAVE_UNION_SEMUN
union semun {
  int             val;            /* value for SETVAL */
  struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
  unsigned short  *array;         /* array for GETALL & SETALL */
};
#endif

static TABool TALock_semop(TALock self, short op);

TALock TALock_init()
{
  struct __TALock *self = malloc(sizeof(struct __TALock));
  union semun sem_union;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
  sem_union.val = 1;
  if (semctl(self->semid, 0, SETVAL, sem_union) == -1)
  {
    fprintf(stderr, "semctl failed [%s]\n", strerror(errno));
    return NULL;
  }

  return self;
}

void TALock_release(TALock self)
{
  union semun sem_union;

  if (semctl(self->semid, 0, IPC_RMID, sem_union) == -1)
  {
    fprintf(stderr, "semctl failed [%s]\n", strerror(errno));
    exit(1);
  }

  free(self);
}

TABool TALock_lock(TALock self)
{
  return TALock_semop(self, -1); /* P */
}

TABool TALock_unlock(TALock self)
{
  return TALock_semop(self, 1); /* V */
}

/* private */
static TABool TALock_semop(TALock self, short op)
{
  struct sembuf sops;
  sops.sem_num = 0;
  sops.sem_op = op;
  sops.sem_flg = SEM_UNDO;
  if (semop(self->semid, &sops, 1) == -1)
  {
    fprintf(stderr, "semop failed [%s]\n", strerror(errno));
    return FALSE;
  }

  return TRUE;
}
