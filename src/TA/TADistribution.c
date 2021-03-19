/*
 * TADistribution.c
 * TA
 *
 * Created by Takashi Hashizume on 03/25/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "TADistribution.h"
#include "config.h"

struct __TADistribution
{
  int deep_copied;
  unsigned int count;

#ifdef DISABLE_HIGH_PRECISION_DISTRIBUTION
#define NUM_BUCKETS 101
#else
#define NUM_BUCKETS 1001
#endif

#define SCALE 7
  /*
   * buckets[0][] seconds,
   * buckets[1][] deci seconds,
   * buckets[2][] centi seconds,
   * buckets[3][] milli seconds
   * buckets[4][] decimilli seconds
   * buckets[5][] centimilli seconds
   * buckets[6][] micro seconds
   */
  unsigned int buckets[SCALE][NUM_BUCKETS];
};
typedef struct __TADistribution __TADistribution;

#define minbucket(tvp)                                                  \
  {                                                                     \
    (tvp)->tv_sec = 0;                                                  \
    (tvp)->tv_usec = 1; /* 1 / (10 ** (SCALE - 1)) sec */               \
  }

#define METRIC_PREFIX_SYMBOLS { "", "d", "c", "m", "dm", "cm", "u" }

TADistribution TADistribution_init()
{
  struct __TADistribution *self = malloc(sizeof(struct __TADistribution));
  int i = 0;
  int j = 0;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->deep_copied = FALSE;
  self->count = 0;
  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      self->buckets[i][j] = 0;
    }
  }

  return self;
}

TADistribution TADistribution_initWithJSON(const char *json)
{
  struct __TADistribution *self = malloc(sizeof(struct __TADistribution));
  char *s;
  char *e;
  char *end;
  int i = 0;
  int j = 0;

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  s = strstr(json, "{deep_copied:");
  e = strstr(json, ",count:");
  if (s == NULL || e == NULL)
    return NULL;
  s = s + strlen("{deep_copied:");
  self->deep_copied = (int) strtol(s, &end, 10);
  if (end == s || errno == ERANGE)
    return NULL;

  s = e + strlen(",count:");
  e = strstr(s, ",buckets:");
  if (e == NULL)
    return NULL;
  self->count = (unsigned int) strtoul(s, &end, 10);
  if (end == s || errno == ERANGE)
    return NULL;

  s = e + strlen(",buckets:");
  e = strstr(s, "}");
  if (e == NULL)
    return NULL;
  s = s + strlen("[");
  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      self->buckets[i][j] = (unsigned int) strtoul(s, &end, 10);
      if (end == s || errno == ERANGE)
        return NULL;
      s = end + 1;
    }
  }

  return self;
}

void TADistribution_release(TADistribution self)
{
  if (self->deep_copied == FALSE)
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
  int j = 0;

  dest->deep_copied = TRUE;
  dest->count = self->count;
  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      dest->buckets[i][j] = self->buckets[i][j];
    }
  }
}

void TADistribution_setElapsedTime(TADistribution self, struct timeval elaps)
{
  struct timeval min_bucket_tv;
  struct timeval min_tv;
  struct timeval max_tv;
  int decimal = 0;
  int i = 0;

  self->count++;

  timerclear(&min_bucket_tv);
  timerclear(&min_tv);
  timerclear(&max_tv);

  minbucket(&min_bucket_tv);

  for (i = SCALE - 1, decimal = 1; i >= 0; i--, decimal = decimal * 10)
  {
    timermlt(&min_bucket_tv, decimal, &min_tv);
    timermlt(&min_tv, NUM_BUCKETS - 1, &max_tv);

    if (timercmp(&elaps, &min_tv, <))
      self->buckets[i][0]++;
    else if (timercmp(&elaps, &max_tv, <))
      self->buckets[i][timeval2usec(elaps) / (timeval2usec(min_bucket_tv) * decimal)]++;
    else
      self->buckets[i][NUM_BUCKETS - 1]++;
  }
}

struct timeval TADistribution_percentile(TADistribution self, int percent)
{
  struct timeval min_bucket_tv;
  struct timeval min_tv;
  struct timeval ret;
  unsigned int count = 0;
  int accum = 0;
  int decimal = 0;
  int i = 0;
  int j = 0;

  timerclear(&min_bucket_tv);
  timerclear(&min_tv);
  timerclear(&ret);

  minbucket(&min_bucket_tv);

  if (self->count == 0)
    return ret;

  for (i = SCALE - 1, decimal = 1; i >= 0; i--, decimal = decimal * 10)
  {
    timermlt(&min_bucket_tv, decimal, &min_tv);

    for (j = 0, count = 0; j < NUM_BUCKETS; j++)
      count += self->buckets[i][j];

    for (j = 0, accum = 0; j < NUM_BUCKETS - 1; j++)
    {
      accum += self->buckets[i][j];
      if ((percent == 0 && accum > 0) ||
          (percent > 0 && accum * 100 / count >= percent))
      {
        timermlt(&min_tv, j, &ret);
        return ret;
      }
    }
  }

  ret.tv_sec = 9999;
  ret.tv_usec = 0;

  return ret;
}

void TADistribution_print(TADistribution self)
{
  struct timeval min_bucket_tv;
  struct timeval min_tv, deca_tv, max_tv;
  struct timeval zeroth_percentile, hundredth_percentile;
  char *metric_prefix_symbols[SCALE] = METRIC_PREFIX_SYMBOLS;
#define STR_SIZE 4
  char str[STR_SIZE];
  unsigned int count = 0;
  unsigned int accum = 0;
  double percent = 0;
  int bucket;
  int decimal = 0;
  int i = 0;
  int j = 0;

  timerclear(&min_bucket_tv);
  timerclear(&min_tv);
  timerclear(&deca_tv);
  timerclear(&max_tv);
  timerclear(&zeroth_percentile);
  timerclear(&hundredth_percentile);

  minbucket(&min_bucket_tv);
  zeroth_percentile = TADistribution_percentile(self, 0);
  hundredth_percentile = TADistribution_percentile(self, 100);

#define PRINTED_BUCKETS 101
  for (i = SCALE - 1, decimal = 1; i >= 0; i--, decimal = decimal * 10)
  {
    timermlt(&min_bucket_tv, decimal, &min_tv);
    timermlt(&min_tv, 10, &deca_tv);
    timermlt(&min_tv, PRINTED_BUCKETS - 1, &max_tv);
    if (timercmp(&deca_tv, &hundredth_percentile, >) ||
        timercmp(&max_tv, &zeroth_percentile, <))
      continue;

    printf("Frequency Distribution (%ssec.)\n", metric_prefix_symbols[i]);
    printf("==============================\n");
    printf("        0%%        10%%       20%%       30%%       40%%       "
           "50%%       60%%       70%%       80%%       90%%       100%%\n");
    printf("        +---------+---------+---------+---------+---------+"
           "---------+---------+---------+---------+---------+\n");
    for (bucket = 0, accum = 0; bucket < PRINTED_BUCKETS; bucket++)
    {
      snprintf(str, STR_SIZE, "%ss", metric_prefix_symbols[i]);
      if (bucket != PRINTED_BUCKETS - 1)
      {
        printf("<%3d%-3s |", bucket + 1, str);
        count = self->buckets[i][bucket];
        accum += self->buckets[i][bucket];
      }
      else
      {
        printf(">=%3d%-3s|", bucket, str);
        count = self->count - accum;
      }

      percent = self->count == 0 ? 0.0 : (double) count * 100 / self->count;
      for (j = 0; j < 100; j++)
      {
        if (j < percent)
          printf("*");
        else
          printf(" ");
      }
      printf(" %5.1f%%(%d/%d)\n", percent, count, self->count);
    }
    printf("\n");
  }
}

TADistribution TADistribution_plus(TADistribution self, TADistribution tadist)
{
  TADistribution ret = TADistribution_init();
  int i = 0;
  int j = 0;

  ret->count = self->count + tadist->count;

  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      ret->buckets[i][j] = self->buckets[i][j] + tadist->buckets[i][j];
    }
  }

  return ret;
}

TADistribution TADistribution_minus(TADistribution self,
                                    TADistribution tadist)
{
  TADistribution ret = TADistribution_init();
  int i = 0;
  int j = 0;

  ret->count = self->count - tadist->count;

  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      ret->buckets[i][j] = self->buckets[i][j] - tadist->buckets[i][j];
    }
  }

  return ret;
}

char *TADistribution_JSON(TADistribution self, char *output, size_t outputsize)
{
#define BUFFER_SIZE 64
  char buffer_str[BUFFER_SIZE];
  int length = 0;
  int i = 0;
  int j = 0;

  length = snprintf(buffer_str, BUFFER_SIZE, "{deep_copied:%d,",
                    self->deep_copied);
  if (outputsize > length)
    strcpy(output, buffer_str);
  else
    return NULL;

  length += snprintf(buffer_str, BUFFER_SIZE, "count:%d,", self->count);
  if (outputsize > length)
    strcat(output, buffer_str);
  else
    return NULL;

  length += snprintf(buffer_str, BUFFER_SIZE, "buckets:[");
  if (outputsize > length)
    strcat(output, buffer_str);
  else
    return NULL;

  for (i = 0; i < SCALE; i++)
  {
    for (j = 0; j < NUM_BUCKETS; j++)
    {
      length += snprintf(buffer_str, BUFFER_SIZE, "%d,", self->buckets[i][j]);
      if (outputsize > length)
        strcat(output, buffer_str);
      else
        return NULL;
    }
  }

  length += snprintf(buffer_str, BUFFER_SIZE, "]}");
  if (outputsize > length)
    strcat(output, buffer_str);
  else
    return NULL;

  return output;
}

size_t TADistribution_JSONMaxLength()
{
  size_t ret = 0;

  ret = strlen("{deep_copied:") + (size_t)log10((double)INT_MAX) + strlen(",");
  ret += strlen("count:") + (size_t)log10((double)UINT_MAX) + strlen(",");
  ret += strlen("buckets:[");
  ret += ((size_t)log10((double)UINT_MAX) + strlen(",")) * SCALE * NUM_BUCKETS;
  ret += strlen("]}");

  return ret;
}
