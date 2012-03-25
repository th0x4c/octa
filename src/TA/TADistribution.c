/*
 * TADistribution.c
 * TA
 *
 * Created by Takashi Hashizume on 03/25/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "TADistribution.h"

struct __TADistribution
{
  int count;
#define BUCKETS 101
  int msec[BUCKETS]; /* milli */
  int csec[BUCKETS]; /* centi */
  int dsec[BUCKETS]; /* deci */
  int sec[BUCKETS];
};
typedef struct __TADistribution __TADistribution;

TADistribution TADistribution_init()
{
  struct __TADistribution *self = malloc(sizeof(struct __TADistribution));
  int i = 0;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->count = 0;
  for (i = 0; i < BUCKETS; i++)
  {
    self->msec[i] = 0;
    self->csec[i] = 0;
    self->dsec[i] = 0;
    self->sec[i] = 0;
  }

  return self;
}

void TADistribution_release(TADistribution self)
{
  free(self);
}

size_t TADistribution_sizeof()
{
  return sizeof(struct __TADistribution);
}

TADistribution TADistribution_nextAddr(TADistribution self)
{
  return self + 1;
}

void TADistribution_deepCopy(TADistribution self, TADistribution dest)
{
  int i = 0;

  dest->count = self->count;
  for (i = 0; i < BUCKETS; i++)
  {
    dest->msec[i] = self->msec[i];
    dest->csec[i] = self->csec[i];
    dest->dsec[i] = self->dsec[i];
    dest->sec[i] = self->sec[i];
  }
}

void TADistribution_setElapsedTime(TADistribution self, struct timeval elaps)
{
  struct timeval msec, csec, dsec, sec;
  struct timeval max_msec, max_csec, max_dsec, max_sec;

  self->count++;

  timerclear(&msec);
  timerclear(&csec);
  timerclear(&dsec);
  timerclear(&sec);
  timerclear(&max_msec);
  timerclear(&max_csec);
  timerclear(&max_dsec);
  timerclear(&max_sec);

  msec.tv_sec = 0;
  msec.tv_usec = 1000;
  csec.tv_sec = 0;
  csec.tv_usec = 10000;
  dsec.tv_sec = 0;
  dsec.tv_usec = 100000;
  sec.tv_sec = 1;
  sec.tv_usec = 0;

  timermlt(&msec, BUCKETS - 1, &max_msec);
  timermlt(&csec, BUCKETS -1, &max_csec);
  timermlt(&dsec, BUCKETS -1, &max_dsec);
  timermlt(&sec, BUCKETS - 1, &max_sec);

  if (timercmp(&elaps, &msec, <))
    self->msec[0]++;
  else if (timercmp(&elaps, &max_msec, <))
  {
    self->msec[(elaps.tv_sec * 1000000 + elaps.tv_usec) / 1000]++;
  }
  else
    self->msec[BUCKETS - 1]++;

  if (timercmp(&elaps, &csec, <))
    self->csec[0]++;
  else if (timercmp(&elaps, &max_csec, <))
  {
    self->csec[(elaps.tv_sec * 1000000 + elaps.tv_usec) / 10000]++;
  }
  else
    self->csec[BUCKETS - 1]++;

  if (timercmp(&elaps, &dsec, <))
    self->dsec[0]++;
  else if (timercmp(&elaps, &max_dsec, <))
  {
    self->dsec[(elaps.tv_sec * 1000000 + elaps.tv_usec) / 100000]++;
  }
  else
    self->dsec[BUCKETS - 1]++;

  if (timercmp(&elaps, &sec, <))
    self->sec[0]++;
  else if (timercmp(&elaps, &max_sec, <))
  {
    self->sec[(elaps.tv_sec * 1000000 + elaps.tv_usec) / 1000000]++;
  }
  else
    self->sec[BUCKETS - 1]++;
}

struct timeval TADistribution_percentile(TADistribution self, int percent)
{
  struct timeval msec, csec, dsec, sec;
  struct timeval ret;
  int accum = 0;
  int i = 0;

  timerclear(&msec);
  timerclear(&csec);
  timerclear(&dsec);
  timerclear(&sec);
  timerclear(&ret);

  msec.tv_sec = 0;
  msec.tv_usec = 1000;
  csec.tv_sec = 0;
  csec.tv_usec = 10000;
  dsec.tv_sec = 0;
  dsec.tv_usec = 100000;
  sec.tv_sec = 1;
  sec.tv_usec = 0;

  for (i = 0, accum = 0; i < BUCKETS; i++)
  {
    accum += self->msec[i];
    if (accum * 100 / self->count >= percent)
      break;
  }
  if (i != BUCKETS - 1)
  {
    timermlt(&msec, i, &ret);
    return ret;
  }

  for (i = 0, accum = 0; i < BUCKETS; i++)
  {
    accum += self->csec[i];
    if (accum * 100 / self->count >= percent)
      break;
  }
  if (i != BUCKETS - 1)
  {
    timermlt(&csec, i, &ret);
    return ret;
  }

  for (i = 0, accum = 0; i < BUCKETS; i++)
  {
    accum += self->dsec[i];
    if (accum * 100 / self->count >= percent)
      break;
  }
  if (i != BUCKETS - 1)
  {
    timermlt(&dsec, i, &ret);
    return ret;
  }

  for (i = 0, accum = 0; i < BUCKETS; i++)
  {
    accum += self->sec[i];
    if (accum * 100 / self->count >= percent)
      break;
  }
  if (i != BUCKETS - 1)
  {
    timermlt(&sec, i, &ret);
    return ret;
  }

  ret.tv_sec = 9999;
  ret.tv_usec = 0;

  return ret;
}

void TADistribution_print(TADistribution self)
{
  int percent = 0;
  int bucket;
  int i = 0;

  printf("Frequency Distribution (msec.)\n");
  printf("==============================\n");
  printf("        0%%        10%%       20%%       30%%       40%%       "
         "50%%       60%%       70%%       80%%       90%%       100%%\n");
  printf("        +---------+---------+---------+---------+---------+"
         "---------+---------+---------+---------+---------+\n");
  for (bucket = 0; bucket < BUCKETS; bucket++)
  {
    if (bucket != BUCKETS - 1)
      printf("<%3dms |", bucket + 1);
    else
      printf(">=%3dms|", bucket);

    percent = self->msec[bucket] * 100 / self->count;
    for (i = 0; i < 100; i++)
    {
      if (i  < percent)
        printf("*");
      else
        printf(" ");
    }
    printf(" %3d%%(%d/%d)\n", percent, self->msec[bucket], self->count);
  }
  printf("\n");

  printf("Frequency Distribution (csec.)\n");
  printf("==============================\n");
  printf("        0%%        10%%       20%%       30%%       40%%       "
         "50%%       60%%       70%%       80%%       90%%       100%%\n");
  printf("        +---------+---------+---------+---------+---------+"
         "---------+---------+---------+---------+---------+\n");
  for (bucket = 0; bucket < BUCKETS; bucket++)
  {
    if (bucket != BUCKETS - 1)
      printf("<%3dcs |", bucket + 1);
    else
      printf(">=%3dcs|", bucket);

    percent = self->csec[bucket] * 100 / self->count;
    for (i = 0; i < 100; i++)
    {
      if (i  < percent)
        printf("*");
      else
        printf(" ");
    }
    printf(" %3d%%(%d/%d)\n", percent, self->csec[bucket], self->count);
  }
  printf("\n");

  printf("Frequency Distribution (dsec.)\n");
  printf("==============================\n");
  printf("        0%%        10%%       20%%       30%%       40%%       "
         "50%%       60%%       70%%       80%%       90%%       100%%\n");
  printf("        +---------+---------+---------+---------+---------+"
         "---------+---------+---------+---------+---------+\n");
  for (bucket = 0; bucket < BUCKETS; bucket++)
  {
    if (bucket != BUCKETS - 1)
      printf("<%3dds |", bucket + 1);
    else
      printf(">=%3dds|", bucket);

    percent = self->dsec[bucket] * 100 / self->count;
    for (i = 0; i < 100; i++)
    {
      if (i  < percent)
        printf("*");
      else
        printf(" ");
    }
    printf(" %3d%%(%d/%d)\n", percent, self->dsec[bucket], self->count);
  }
  printf("\n");

  printf("Frequency Distribution (sec.)\n");
  printf("=============================\n");
  printf("        0%%        10%%       20%%       30%%       40%%       "
         "50%%       60%%       70%%       80%%       90%%       100%%\n");
  printf("        +---------+---------+---------+---------+---------+"
         "---------+---------+---------+---------+---------+\n");
  for (bucket = 0; bucket < BUCKETS; bucket++)
  {
    if (bucket != BUCKETS - 1)
      printf("<%3ds |", bucket + 1);
    else
      printf(">=%3ds|", bucket);

    percent = self->sec[bucket] * 100 / self->count;
    for (i = 0; i < 100; i++)
    {
      if (i  < percent)
        printf("*");
      else
        printf(" ");
    }
    printf(" %3d%%(%d/%d)\n", percent, self->sec[bucket], self->count);
  }
  printf("\n");
}

TADistribution TADistribution_plus(TADistribution self, TADistribution tadist)
{
  TADistribution ret = TADistribution_init();
  int i = 0;

  ret->count = self->count + tadist->count;

  for (i = 0; i < BUCKETS; i++)
  {
    ret->msec[i] = self->msec[i] + tadist->msec[i];
    ret->csec[i] = self->csec[i] + tadist->csec[i];
    ret->dsec[i] = self->dsec[i] + tadist->dsec[i];
    ret->sec[i] = self->sec[i] + tadist->sec[i];
  }

  return ret;
}

TADistribution TADistribution_minus(TADistribution self,
                                    TADistribution tadist)
{
  TADistribution ret = TADistribution_init();
  int i = 0;

  ret->count = self->count - tadist->count;

  for (i = 0; i < BUCKETS; i++)
  {
    ret->msec[i] = self->msec[i] - tadist->msec[i];
    ret->csec[i] = self->csec[i] - tadist->csec[i];
    ret->dsec[i] = self->dsec[i] - tadist->dsec[i];
    ret->sec[i] = self->sec[i] - tadist->sec[i];
  }

  return ret;
}
