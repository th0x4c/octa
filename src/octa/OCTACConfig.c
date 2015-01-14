/*
 * OCTACConfig.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/07/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACConfig.h"

void OCTACConfig_insertOriginal(char *data)
{
  int pos = TARandom_number(0, strlen(data) - strlen("original"));
  data[pos] = 'o';
  data[pos + 1] = 'r';
  data[pos + 2] = 'i';
  data[pos + 3] = 'g';
  data[pos + 4] = 'i';
  data[pos + 5] = 'n';
  data[pos + 6] = 'a';
  data[pos + 7] = 'l';
}

void OCTACConfig_makeAddress(char *street_1, char *street_2, char *city,
                             char *state, char *zip)
{
  TARandom_getAlphaString(street_1, 10, 20);
  TARandom_getAlphaString(street_2, 10, 20);
  TARandom_getAlphaString(city, 10, 20);
  TARandom_getAlphaString(state, 2, 2);
  /*
   * The warehouse zip code (W_ZIP), the district zip code (D_ZIP) and the
   *  customer zip code (C_ZIP) must be generated by the concatenation of:
   *    1. A random n-string of 4 numbers, and
   *    2. The constant string '11111'.
   *  Given a random n-string between 0 and 9999, the zip codes are determined
   *  by concatenating the n-string and the constant '11111'. This will create
   *  10,000 unique zip codes. For example, the n-string 0503 concatenated with
   *  11111, will make the zip code 050311111
   */
  snprintf(zip, 9, "%ld11111", TARandom_number(0, 9999));
}

void OCTACConfig_lastname(long num, char *name)
{
  static char *n[] = {"BAR", "OUGHT", "ABLE", "PRI", "PRES", "ESE", "ANTI",
                      "CALLY", "ATION", "EING"};

  strcpy(name, n[num / 100]);
  strcat(name, n[(num / 10) % 10]);
  strcat(name, n[num % 10]);
}

long OCTACConfig_NURand(long a, long x, long y)
{
  return (((TARandom_number(0, a) | TARandom_number(x, y)) + CNUM) %
          (y - x + 1)) + x;
}

void OCTACConfig_initPermutation(long seq[], size_t seqsize)
{
  long i = 0;
  long j = 0;
  long tmp = 0;

  for (i = 0; i < seqsize; i++)
  {
    seq[i] = i + 1;

    j = TARandom_number(0, i);
    tmp = seq[i];
    seq[i] = seq[j];
    seq[j] = tmp;
  }
}

long OCTACConfig_homeWID(int scale_factor, int session_id)
{
  return ((session_id - 1) % scale_factor) + 1;
}

void OCTACConfig_sleepKeyingTime(struct timeval keying_timeval)
{
  struct timespec keying_timespec;

  if (timerisset(&keying_timeval))
  {
    keying_timespec.tv_sec = keying_timeval.tv_sec;
    keying_timespec.tv_nsec = keying_timeval.tv_usec * 1000;
    nanosleep(&keying_timespec, NULL);
  }
}

void OCTACConfig_sleepThinkTime(struct timeval think_timeval)
{
  long think_time_nsec;
  struct timespec think_timespec;

  if (timerisset(&think_timeval))
  {
    think_time_nsec = timeval2usec(think_timeval) * 1000;
    think_time_nsec = - log(TARandom_drand()) * think_time_nsec;

    think_timespec.tv_sec = think_time_nsec / 1000000000;
    think_timespec.tv_nsec = think_time_nsec % 1000000000;
    nanosleep(&think_timespec, NULL);
  }
}
