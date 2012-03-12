/*
 * test_TATXStat.c
 * TA
 *
 * Created by Takashi Hashizume on 03/03/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <TA/TATXStat.h>
#include "../munit.h"
#include <unistd.h> /* sleep */
#include <stdlib.h> /* malloc exit */
#include <stdio.h>  /* fprintf printf */
#include <string.h> /* strncpy strcmp */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

#define TEST_COUNT 5
static TATXStat tatxstat = NULL;
static TATXStat tatxstat2 = NULL;
static TATXStat tatxstat3 = NULL;
static int errcode = 0;
static char errmsg[256] = "";

static void testTX()
{
  sleep(1);
}

static void setup()
{
  int i = 0;
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  tatxstat = TATXStat_init();

  tatxstat2 = malloc(TATXStat_sizeof());
  if (tatxstat2 == NULL)
  {
    fprintf(stdout, "Cannot malloc TATXStat\n");
    exit(1);
  }

  TATXStat_setName(tatxstat, "unit test");

  for (i = 1; i <= TEST_COUNT; i++)
  {
    TATXStat_start(tatxstat);
    testTX();
    TATXStat_end(tatxstat);

    if (i == 2)
      TATXStat_deepCopy(tatxstat, tatxstat2);

    if (i == 3)
      TATXStat_setError(tatxstat, 999, "Error Message");

    if (TATXStat_errorCode(tatxstat))
    {
      errcode = TATXStat_errorCode(tatxstat);
      strncpy(errmsg, TATXStat_errorMessage(tatxstat), sizeof(errmsg) - 1);
      errmsg[sizeof(errmsg) - 1] = '\0';
    }

    printf("%s\n", TATXStat_description(tatxstat, desc, DESC_SIZE));
  }
}

static void teardown()
{
  TATXStat_release(tatxstat);
  TATXStat_release(tatxstat2);
  TATXStat_release(tatxstat3);
}

static void test_TATXStat_name()
{
  mu_assert(strcmp(TATXStat_name(tatxstat), "unit test") == 0);
}

static void test_TATXStat_count()
{
  mu_assert(TATXStat_count(tatxstat) == TEST_COUNT);
}

static void test_TATXStat_totalElapsedTime()
{
  struct timeval totalelatv, difftv, expectedtv, twentymsectv, withinmsectv;
  int i = 0;

  timerclear(&totalelatv);
  timerclear(&difftv);
  timerclear(&expectedtv);
  timerclear(&twentymsectv);
  timerclear(&withinmsectv);

  totalelatv = TATXStat_totalElapsedTime(tatxstat);
  expectedtv.tv_sec = TEST_COUNT;
  expectedtv.tv_usec = 0;
  twentymsectv.tv_sec = 0;
  twentymsectv.tv_usec = 20000;
  for (i = 0; i < TEST_COUNT; i++)
  {
    timeradd(&withinmsectv, &twentymsectv, &withinmsectv);
  }

  timersub(&totalelatv, &expectedtv, &difftv);

  mu_assert(timercmp(&difftv, &twentymsectv, <));
}

static void test_TATXStat_errorCount()
{
  mu_assert(TATXStat_errorCount(tatxstat) == 1);
}

static void test_TATXStat_errorCode()
{
  mu_assert(errcode == 999);
}

static void test_TATXStat_errorMessage()
{
  mu_assert(strcmp(errmsg, "Error Message") == 0);
}

static void test_TATXStat_avgElapsedTime()
{
  struct timeval avgelatv, difftv, expectedtv, twentymsectv;

  timerclear(&avgelatv);
  timerclear(&difftv);
  timerclear(&expectedtv);
  timerclear(&twentymsectv);

  avgelatv = TATXStat_avgElapsedTime(tatxstat);
  expectedtv.tv_sec = 1;
  expectedtv.tv_usec = 0;
  twentymsectv.tv_sec = 0;
  twentymsectv.tv_usec = 20000;

  timersub(&avgelatv, &expectedtv, &difftv);

  mu_assert(timercmp(&difftv, &twentymsectv, <));
}

static void test_TATXStat_tps()
{
  double tps = TATXStat_tps(tatxstat);

  mu_assert(1 / (1.0 + 0.02) < tps && tps < 1 / (1.0 - 0.02));
}

static void test_TATXStat_deepCopy()
{
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  printf("%s\n", TATXStat_description(tatxstat2, desc, DESC_SIZE));

  mu_assert(strcmp(TATXStat_name(tatxstat2), TATXStat_name(tatxstat)) == 0);
  mu_assert(TATXStat_count(tatxstat2) == 2);
}

static void test_TATXStat_minus()
{
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  tatxstat3 = TATXStat_minus(tatxstat, tatxstat2);
  printf("%s\n", TATXStat_description(tatxstat3, desc, DESC_SIZE));

  mu_assert(strcmp(TATXStat_name(tatxstat3), TATXStat_name(tatxstat)) == 0);
  mu_assert(TATXStat_count(tatxstat3) == TEST_COUNT - 2);
}

static void test_TATXStat_plus()
{
  TATXStat tatxstat5 = NULL;
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  tatxstat5 = TATXStat_plus(tatxstat2, tatxstat3);
  printf("%s\n", TATXStat_description(tatxstat5, desc, DESC_SIZE));

  mu_assert(strcmp(TATXStat_name(tatxstat5), "unit testunit test") == 0);
  mu_assert(TATXStat_count(tatxstat5) == TEST_COUNT);

  TATXStat_release(tatxstat5);
}

int main(int argc, char *argv[])
{
  setup();
  mu_run_test(test_TATXStat_name);
  mu_run_test(test_TATXStat_count);
  mu_run_test(test_TATXStat_totalElapsedTime);
  mu_run_test(test_TATXStat_errorCount);
  mu_run_test(test_TATXStat_errorCode);
  mu_run_test(test_TATXStat_errorMessage);
  mu_run_test(test_TATXStat_avgElapsedTime);
  mu_run_test(test_TATXStat_tps);
  mu_run_test(test_TATXStat_deepCopy);
  mu_run_test(test_TATXStat_minus);
  mu_run_test(test_TATXStat_plus);
  teardown();
  mu_show_failures();
  return mu_nfail != 0;
}
