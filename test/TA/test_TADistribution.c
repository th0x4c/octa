/*
 * test_TADistribution.c
 * TA
 *
 * Created by Takashi Hashizume on 03/25/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <TA/TADistribution.h>
#include "../munit.h"
#include <TA/TATime.h>
#include "config.h"
#include <stdlib.h> /* malloc exit free */
#include <string.h> /* strcmp */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

static void test_TADistribution_initWithJSON()
{
#define TEST_COUNT 1000
  TADistribution tadist1 = NULL;
  TADistribution tadist2 = NULL;
  char *json1;
  char *json2;
  struct timeval msec, dsec, sec;
  struct timeval elaps;
  int i = 0;

  timerclear(&msec);
  msec.tv_sec = 0;
  msec.tv_usec = 1000;
  timerclear(&dsec);
  dsec.tv_sec = 0;
  dsec.tv_usec = 100000;
  timerclear(&sec);
  sec.tv_sec = 1;
  sec.tv_usec = 0;
  timerclear(&elaps);

  tadist1 = TADistribution_init();
  json1 = malloc(TADistribution_JSONMaxLength() + 1);
  json2 = malloc(TADistribution_JSONMaxLength() + 1);
  if (json1 == NULL || json2 == NULL)
    exit(1);

  for (i = 0; i < TEST_COUNT; i++)
  {
    timermlt(&msec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist1, elaps);
    timermlt(&dsec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist1, elaps);
    timermlt(&sec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist1, elaps);
  }
  TADistribution_print(tadist1);
  mu_assert(TADistribution_JSON(tadist1, json1, TADistribution_JSONMaxLength() + 1) != NULL);
  printf("JSON[%ld]: %s\n", TADistribution_JSONMaxLength(), json1);

  tadist2 = TADistribution_initWithJSON(json1);
  TADistribution_print(tadist2);
  mu_assert(TADistribution_JSON(tadist2, json2, TADistribution_JSONMaxLength() + 1) != NULL);
  printf("JSON[%ld]: %s\n", TADistribution_JSONMaxLength(), json2);

  mu_assert(strcmp(json1, json2) == 0);

  TADistribution_release(tadist1);
  TADistribution_release(tadist2);
  free(json1);
  free(json2);
}

static void test_TADistribution_percentile()
{
#define TEST_COUNT 1000
  TADistribution tadist = NULL;
  struct timeval msec, dsec, sec;
  struct timeval elaps, percentile, expectedtv;
  int i = 0;

  timerclear(&msec);
  msec.tv_sec = 0;
  msec.tv_usec = 1000;
  timerclear(&dsec);
  dsec.tv_sec = 0;
  dsec.tv_usec = 100000;
  timerclear(&sec);
  sec.tv_sec = 1;
  sec.tv_usec = 0;
  timerclear(&elaps);
  timerclear(&percentile);
  timerclear(&expectedtv);

  tadist = TADistribution_init();
  for (i = 0; i < TEST_COUNT; i++)
  {
    timermlt(&msec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist, elaps);
  }
  percentile = TADistribution_percentile(tadist, 90);
  expectedtv.tv_sec = 0;
  expectedtv.tv_usec = 900000; /* 900msec */
  printf("90%% percentile: %f\n", timeval2sec(percentile));
  mu_assert(timercmp(&percentile, &expectedtv, ==));
  TADistribution_print(tadist);
  TADistribution_release(tadist);

  tadist = TADistribution_init();
  for (i = 0; i < TEST_COUNT; i++)
  {
    timermlt(&dsec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist, elaps);
  }
  percentile = TADistribution_percentile(tadist, 90);
  expectedtv.tv_sec = 90;
  expectedtv.tv_usec = 0; /* 90sec */
  printf("90%% percentile: %f\n", timeval2sec(percentile));
  mu_assert(timercmp(&percentile, &expectedtv, ==));
  TADistribution_release(tadist);

  tadist = TADistribution_init();
  for (i = 0; i < TEST_COUNT; i++)
  {
    timermlt(&sec, i + 1, &elaps);
    TADistribution_setElapsedTime(tadist, elaps);
  }
  percentile = TADistribution_percentile(tadist, 90);
#ifdef DISABLE_HIGH_PRECISION_DISTRIBUTION
  /* TADistribution_percentile returns 9999sec if exact percentile >= 100sec */
  expectedtv.tv_sec = 9999;
  expectedtv.tv_usec = 0;
#else
  expectedtv.tv_sec = 900;
  expectedtv.tv_usec = 0; /* 900sec */
#endif
  printf("90%% percentile: %f\n", timeval2sec(percentile));
  mu_assert(timercmp(&percentile, &expectedtv, ==));
  TADistribution_release(tadist);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TADistribution_initWithJSON);
  mu_run_test(test_TADistribution_percentile);
  mu_show_failures();
  return mu_nfail != 0;
}
