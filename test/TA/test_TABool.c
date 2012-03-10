/*
 * test_TABool.c
 * TA
 *
 * Created by Takashi Hashizume on 03/03/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <TA/TABool.h>
#include "../munit.h"

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

static void test_TABool_TRUE()
{
  mu_assert(TRUE);
}

static void test_TABool_FALSE()
{
  mu_assert(! FALSE);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_TABool_TRUE);
  mu_run_test(test_TABool_FALSE);
  mu_show_failures();
  return mu_nfail != 0;
}
