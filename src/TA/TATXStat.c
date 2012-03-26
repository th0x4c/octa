/*
 * TATXStat.c
 * TA
 *
 * Created by Takashi Hashizume on 08/05/11.
 * Copyright 2011 Takashi Hashizume. All rights reserved.
 */

#include "TATXStat.h"

struct __TATXStat
{
#define MAX_NAME_SIZE 64
  char name[MAX_NAME_SIZE];
  unsigned int count;
  struct timeval first_time;
  struct timeval start_time;
  struct timeval end_time;
  struct timeval elapsed_time;
  struct timeval total_elapsed_time;
  struct timeval max_elapsed_time;
  struct timeval min_elapsed_time;
  unsigned int error_count;
  int error_code;
#define MAX_MSG_SIZE 256
  char error_message[MAX_MSG_SIZE];
  TADistribution distribution;
};

TATXStat TATXStat_init()
{
  struct __TATXStat *self = malloc(sizeof(struct __TATXStat));
  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  strcpy(self->name, "");
  self->count = 0;
  timerclear(&(self->first_time));
  timerclear(&(self->start_time));
  timerclear(&(self->end_time));
  timerclear(&(self->elapsed_time));
  timerclear(&(self->total_elapsed_time));
  timerclear(&(self->max_elapsed_time));
  timerclear(&(self->min_elapsed_time));
  self->error_count = 0;
  self->error_code = 0;
  strcpy(self->error_message, "");
  self->distribution = TADistribution_init();

  return self;
}

void TATXStat_release(TATXStat self)
{
  TADistribution_release(self->distribution);
  free(self);
}

size_t TATXStat_sizeof()
{
  return sizeof(struct __TATXStat) + TADistribution_sizeof();
}

TATXStat TATXStat_nextAddr(TATXStat self)
{
  TATXStat ret = NULL;

  ret = self + 1;
  ret = (TATXStat) TADistribution_nextAddr((TADistribution)ret);
  return ret;
}

void TATXStat_deepCopy(TATXStat self, TATXStat dest)
{
  strcpy(dest->name, self->name);
  dest->count = self->count;
  dest->first_time = self->first_time;
  dest->start_time = self->start_time;
  dest->end_time = self->end_time;
  dest->elapsed_time = self->elapsed_time;
  dest->total_elapsed_time = self->total_elapsed_time;
  dest->max_elapsed_time = self->max_elapsed_time;
  dest->min_elapsed_time = self->min_elapsed_time;
  dest->error_count = self->error_count;
  dest->error_code = self->error_code;
  strcpy(dest->error_message, self->error_message);
  dest->distribution = (TADistribution) (dest + 1);
  TADistribution_deepCopy(self->distribution, dest->distribution);
}

void TATXStat_setName(TATXStat self, const char *name)
{
  strncpy(self->name, name, MAX_NAME_SIZE);
  self->name[MAX_NAME_SIZE - 1] = '\0';
}

char *TATXStat_name(TATXStat self)
{
  return self->name;
}

unsigned int TATXStat_count(TATXStat self)
{
  return self->count;
}

struct timeval TATXStat_firstTime(TATXStat self)
{
  return self->first_time;
}

struct timeval TATXStat_startTime(TATXStat self)
{
  return self->start_time;
}

struct timeval TATXStat_endTime(TATXStat self)
{
  return self->end_time;
}

struct timeval TATXStat_elapsedTime(TATXStat self)
{
  return self->elapsed_time;
}

struct timeval TATXStat_totalElapsedTime(TATXStat self)
{
  return self->total_elapsed_time;
}

struct timeval TATXStat_maxElapsedTime(TATXStat self)
{
  return self->max_elapsed_time;
}

struct timeval TATXStat_minElapsedTime(TATXStat self)
{
  return self->min_elapsed_time;
}

unsigned int TATXStat_errorCount(TATXStat self)
{
  return self->error_count;
}

void TATXStat_setError(TATXStat self, int errcode, const char *errmessage)
{
  self->error_code = errcode;
  strncpy(self->error_message, errmessage, MAX_MSG_SIZE);
  self->error_message[MAX_MSG_SIZE - 1] = '\0';
  self->error_count += 1;
}

int TATXStat_errorCode(TATXStat self)
{
  return self->error_code;
}

char *TATXStat_errorMessage(TATXStat self)
{
  return self->error_message;
}

TADistribution TATXStat_distribution(TATXStat self)
{
  return self->distribution;
}

void TATXStat_start(TATXStat self)
{
  self->error_code = 0;
  strcpy(self->error_message, "");

  gettimeofday(&(self->start_time), (struct timezone *)0);

  if (! timerisset(&(self->first_time)))
    self->first_time = self->start_time;
}

void TATXStat_end(TATXStat self)
{
  gettimeofday(&(self->end_time), (struct timezone *)0);
  self->count += 1;
  timersub(&(self->end_time), &(self->start_time), &(self->elapsed_time));
  timeradd(&(self->total_elapsed_time), &(self->elapsed_time),
           &(self->total_elapsed_time));
  if (self->count == 1)
  {
    self->max_elapsed_time = self->elapsed_time;
    self->min_elapsed_time = self->elapsed_time;
  }
  else
  {
    if (timercmp(&(self->elapsed_time), &(self->max_elapsed_time), >))
      self->max_elapsed_time = self-> elapsed_time;

    if (timercmp(&(self->elapsed_time), &(self->min_elapsed_time), <))
      self->min_elapsed_time = self-> elapsed_time;
  }
  TADistribution_setElapsedTime(self->distribution, self->elapsed_time);
}

struct timeval TATXStat_avgElapsedTime(TATXStat self)
{
  struct timeval avg;

  timerclear(&avg);

  if (self->count == 0)
    return avg;

  avg.tv_sec = self->total_elapsed_time.tv_sec / self->count;
  avg.tv_usec = (self->total_elapsed_time.tv_usec / self->count) +
    ((self->total_elapsed_time.tv_sec % self->count) * 1000000 / self->count);
  return avg;
}

double TATXStat_tps(TATXStat self)
{
  struct timeval actual_elapsed_time;

  timerclear(&actual_elapsed_time);
  timersub(&(self->end_time), &(self->first_time), &actual_elapsed_time);

  if (timerisset(&actual_elapsed_time))
    return (double) self->count / timeval2sec(actual_elapsed_time);
  else
    return 0;
}

char *TATXStat_description(TATXStat self, char *output, size_t outputsize)
{
  char first_time[24] = "0000-00-00 00:00:00.000";
  char end_time[24] = "0000-00-00 00:00:00.000";

  timeval2str(first_time, self->first_time);
  timeval2str(end_time, self->end_time);

  snprintf(output, outputsize, "tx: \"%s\", "
           "count: %u, "
           "error: %d, "
           "first: \"%s\", "
           "end: \"%s\", "
           "elapsed: %f, "
           "total: %f, "
           "avg: %f, "
           "max: %f, "
           "min: %f, "
           "tps: %f",
           self->name,
           self->count,
           self->error_count,
           first_time,
           end_time,
           timeval2sec(self->elapsed_time),
           timeval2sec(self->total_elapsed_time),
           timeval2sec(TATXStat_avgElapsedTime(self)),
           timeval2sec(self->max_elapsed_time),
           timeval2sec(self->min_elapsed_time),
           TATXStat_tps(self));
  return output;
}

TATXStat TATXStat_plus(TATXStat self, TATXStat txstat)
{
  TATXStat ret = TATXStat_init();

  strcpy(ret->name, self->name);
  strncat(ret->name, txstat->name, MAX_NAME_SIZE - strlen(ret->name));
  ret->count = self->count + txstat->count;

  if (timerisset(&(self->first_time)) && timerisset(&(txstat->first_time)))
  {
    if (timercmp(&(self->first_time), &(txstat->first_time), <))
      ret->first_time = self->first_time;
    else
      ret->first_time = txstat->first_time;
  }
  else
  {
    if (timercmp(&(self->first_time), &(txstat->first_time), >))
      ret->first_time = self->first_time;
    else
      ret->first_time = txstat->first_time;
  }

  if (timercmp(&(self->end_time), &(txstat->end_time), >))
  {
    ret->start_time = self->start_time;
    ret->end_time = self->end_time;
    ret->elapsed_time = self->elapsed_time;
    ret->error_code = self->error_code;
    strcpy(ret->error_message, self->error_message);
  }
  else
  {
    ret->start_time = txstat->start_time;
    ret->end_time = txstat->end_time;
    ret->elapsed_time = txstat->elapsed_time;
    ret->error_code = txstat->error_code;
    strcpy(ret->error_message, txstat->error_message);
  }

  timeradd(&(self->total_elapsed_time), &(txstat->total_elapsed_time),
           &(ret->total_elapsed_time));

  if (timercmp(&(self->max_elapsed_time), &(txstat->max_elapsed_time), >))
    ret->max_elapsed_time = self->max_elapsed_time;
  else
    ret->max_elapsed_time = txstat->max_elapsed_time;

  if (timerisset(&(self->min_elapsed_time)) &&
      timerisset(&(txstat->min_elapsed_time)))
  {
    if (timercmp(&(self->min_elapsed_time), &(txstat->min_elapsed_time), <))
      ret->min_elapsed_time = self->min_elapsed_time;
    else
      ret->min_elapsed_time = txstat->min_elapsed_time;
  }
  else
  {
    if (timercmp(&(self->min_elapsed_time), &(txstat->min_elapsed_time), >))
      ret->min_elapsed_time = self->min_elapsed_time;
    else
      ret->min_elapsed_time = txstat->min_elapsed_time;
  }

  ret->error_count = self->error_count + txstat->error_count;

  ret->distribution = TADistribution_plus(self->distribution,
                                          txstat->distribution);

  return ret;
}

TATXStat TATXStat_minus(TATXStat self, TATXStat txstat)
{
  TATXStat ret = TATXStat_init();

  strcpy(ret->name, self->name);
  ret->count = self->count - txstat->count;
  if (txstat->count > 0)
    ret->first_time = txstat->end_time;
  else
    ret->first_time = self->first_time;

  ret->start_time = self->start_time;
  ret->end_time = self->end_time;
  ret->elapsed_time = self->elapsed_time;
  timersub(&(self->total_elapsed_time), &(txstat->total_elapsed_time),
           &(ret->total_elapsed_time));

  /**
   * @warning
   * ret->max_elapsed_time is not correct max elapsed time between
   * ret->first_time and ret->end_time.
   */
  ret->max_elapsed_time = self->max_elapsed_time;

  /**
   * @warning
   * ret->min_elapsed_time is not correct min elapswd time between
   * ret->first_time and ret->end_time.
   */
  ret->min_elapsed_time = self->min_elapsed_time;

  ret->error_count = self->error_count - txstat->error_count;
  ret->error_code = self->error_count;
  strcpy(ret->error_message, self->error_message);

  ret->distribution = TADistribution_minus(self->distribution,
                                           txstat->distribution);

  return ret;
}
