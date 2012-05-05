/*
 * TARandom.c
 * TA
 *
 * Created by Takashi Hashizume on 03/31/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "TARandom.h"

static int seed = FALSE;

long TARandom_rand()
{
  if (seed == FALSE)
  {
    srand48(time(NULL) * getpid());
    seed = TRUE;
  }

  return lrand48();
}

double TARandom_drand()
{
  if (seed == FALSE)
  {
    srand48(time(NULL) * getpid());
    seed = TRUE;
  }

  return drand48();
}

long TARandom_number(long min, long max)
{
  return min + TARandom_rand() % (max - min + 1);
}

int TARandom_getString(char *str, int min, int max, const char *charset)
{
  int len = TARandom_number(min, max);
  int charsetlen = strlen(charset);
  int i;

  for (i = 0; i < len; i++)
    str[i] = charset[TARandom_rand() % charsetlen];

  str[len] = '\0';
  return len;
}

int TARandom_getAlphaString(char *str, int min, int max)
{
  char *charset = "abcdefghijklmnopqrstuvwxyz"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "0123456789";

  return TARandom_getString(str, min, max, charset);
}

int TARandom_getNumberString(char *str, int min, int max)
{
  char *charset = "0123456789";

  return TARandom_getString(str, min, max, charset);
}

int TARandom_indexInRatio(const int ratio[], int indexsize)
{
  long total = 0;
  long randnum = 0;
  long accum = 0;
  int i = 0;

  for (i = 0; i < indexsize; i++)
    total += ratio[i];

  randnum = TARandom_number(1, total);

  for (i = 0; i < indexsize; i++)
  {
    accum += ratio[i];
    if (randnum <= accum)
      return i;
  }
}
