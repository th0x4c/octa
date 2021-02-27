/*
 * test_TALock.c
 * TA
 *
 * Created by Takashi Hashizume on 02/27/21.
 * Copyright 2021 Takashi Hashizume. All rights reserved.
 */

#include <TA/TALock.h>
#include "../munit.h"
#include <sys/shm.h>  /* shmget shmat shmdt shmctl */
#include <stdio.h>    /* fprintf */
#include <string.h>   /* strerror */
#include <errno.h>    /* errno */
#include <stdlib.h>   /* exit srandom random */
#include <unistd.h>   /* fork getpid */
#include <time.h>     /* time */
#include <sys/wait.h> /* waitpid */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

#define COUNT 1000000

static int shmid;
static long *balance_a;
static long *balance_b;

static void transfer(long *src, long *dst, int amount)
{
  *src -= amount;
  *dst += amount;
}

static void test_TALock_lock()
{
  TALock talock = TALock_init();
  pid_t pid;
  int i;

  /* shared memory */
  shmid = shmget(IPC_PRIVATE, sizeof(long) * 2, 0666 | IPC_CREAT);
  if (shmid < 0)
  {
    fprintf(stderr, "shmget failed [%s]\n", strerror(errno));
    exit(1);
  }
  balance_a = (long *) shmat(shmid, 0, 0);
  if (balance_a == (void *)-1)
  {
    fprintf(stderr, "shmat failed [%s]\n", strerror(errno));
    exit(1);
  }
  balance_b = balance_a + 1;

  *balance_a = 0;
  *balance_b = 0;

  switch (pid = fork())
  {
  case -1:
    fprintf(stderr, "fork failed\n");
    exit(1);
  case 0:
    /* child */
    srandom(time(NULL) * getpid());

    for (i = 0; i < COUNT; i++)
    {
      TALock_lock(talock);
      transfer(balance_a, balance_b, (int) random() % 1999999 - 999999);
      TALock_unlock(talock);
    }

    /* shared memory */
    if (shmdt(balance_a) == -1)
    {
      fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
      exit(1);
    }

    exit(0);
    break;
  default:
    /* parent */
    break;
  }

  srandom(time(NULL) * getpid());

  for (i = 0; i < COUNT; i++)
  {
    TALock_lock(talock);
    transfer(balance_b, balance_a, (int) random() % 1999999 - 999999);
    TALock_unlock(talock);
  }

  waitpid(pid, NULL, 0);

  mu_assert(*balance_a + *balance_b == 0);

  TALock_release(talock);

  /* shared memory */
  if (shmdt(balance_a) == -1)
  {
    fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
    exit(1);
  }
  if (shmctl(shmid, IPC_RMID, 0) == -1)
  {
    fprintf(stderr, "shmctl failed [%s]\n", strerror(errno));
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TALock_lock);
  mu_show_failures();
  return mu_nfail != 0;
}
