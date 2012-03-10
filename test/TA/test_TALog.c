/*
 * test_TALog.c
 * TA
 *
 * Created by Takashi Hashizume on 03/02/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <TA/TALog.h>
#include "../munit.h"
#include <stdio.h>  /* fopen fgets fclose */
#include <unistd.h> /* unlink */
#include <string.h> /* strcmp strchr */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

static void test_TALog_level()
{
  TALog talog = TALog_initWithFilename(NULL);
  mu_assert(TALog_level(talog) == TALog_INFO);
  TALog_release(talog);
}

static void test_TALog_setLevel()
{
  TALog talog = TALog_initWithFilename(NULL);
  TALog_setLevel(talog, TALog_OFF);
  mu_assert(TALog_level(talog) == TALog_OFF);
  TALog_setLevel(talog, TALog_FATAL);
  mu_assert(TALog_level(talog) == TALog_FATAL);
  TALog_setLevel(talog, TALog_ERROR);
  mu_assert(TALog_level(talog) == TALog_ERROR);
  TALog_setLevel(talog, TALog_WARN);
  mu_assert(TALog_level(talog) == TALog_WARN);
  TALog_setLevel(talog, TALog_INFO);
  mu_assert(TALog_level(talog) == TALog_INFO);
  TALog_setLevel(talog, TALog_DEBUG);
  mu_assert(TALog_level(talog) == TALog_DEBUG);
  TALog_setLevel(talog, TALog_ALL);
  mu_assert(TALog_level(talog) == TALog_ALL);
  TALog_release(talog);
}

static void test_TALog_fatal()
{
  char *filename = "test_TALog_fatal.log";
#define STRSIZE 64
  char str[STRSIZE] = "";
  TALog talog = TALog_initWithFilename(filename);

  TALog_fatal(talog, "fatal log");
  TALog_release(talog);
  FILE *fp = fopen(filename, "r");
  mu_assert(fgets(str, STRSIZE, fp) != NULL);
  mu_assert(strcmp(strchr(str, '['), "[FATAL] fatal log\n") == 0);
  fclose(fp);
  unlink(filename);
}

static void test_TALog_error()
{
  char *filename = "test_TALog_error.log";
#define STRSIZE 64
  char str[STRSIZE] = "";
  TALog talog = TALog_initWithFilename(filename);

  TALog_error(talog, "error log");
  TALog_release(talog);
  FILE *fp = fopen(filename, "r");
  mu_assert(fgets(str, STRSIZE, fp) != NULL);
  mu_assert(strcmp(strchr(str, '['), "[ERROR] error log\n") == 0);
  fclose(fp);
  unlink(filename);
}

static void test_TALog_warn()
{
  char *filename = "test_TALog_warn.log";
#define STRSIZE 64
  char str[STRSIZE] = "";
  TALog talog = TALog_initWithFilename(filename);

  TALog_warn(talog, "warn log");
  TALog_release(talog);
  FILE *fp = fopen(filename, "r");
  mu_assert(fgets(str, STRSIZE, fp) != NULL);
  mu_assert(strcmp(strchr(str, '['), "[WARN] warn log\n") == 0);
  fclose(fp);
  unlink(filename);
}

static void test_TALog_info()
{
  char *filename = "test_TALog_info.log";
#define STRSIZE 64
  char str[STRSIZE] = "";
  TALog talog = TALog_initWithFilename(filename);

  TALog_info(talog, "info log");
  TALog_release(talog);
  FILE *fp = fopen(filename, "r");
  mu_assert(fgets(str, STRSIZE, fp) != NULL);
  mu_assert(strcmp(strchr(str, '['), "[INFO] info log\n") == 0);
  fclose(fp);
  unlink(filename);
}

static void test_TALog_debug()
{
  char *filename = "test_TALog_debug.log";
#define STRSIZE 64
  char str[STRSIZE] = "";
  TALog talog = TALog_initWithFilename(filename);

  TALog_debug(talog, "debug log");
  TALog_release(talog);
  FILE *fp = fopen(filename, "r");
  mu_assert(fgets(str, STRSIZE, fp) == NULL);
  mu_assert(strcmp(str, "") == 0);
  fclose(fp);
  unlink(filename);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TALog_level);
  mu_run_test(test_TALog_setLevel);
  mu_run_test(test_TALog_fatal);
  mu_run_test(test_TALog_error);
  mu_run_test(test_TALog_warn);
  mu_run_test(test_TALog_info);
  mu_run_test(test_TALog_debug);
  mu_show_failures();
  return mu_nfail != 0;
}
