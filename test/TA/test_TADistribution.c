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

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

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
  mu_run_test(test_TADistribution_percentile);
  mu_show_failures();
  return mu_nfail != 0;
}
