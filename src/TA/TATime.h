/*
 * TATime.h
 * TA
 *
 * Created by Takashi Hashizume on 06/04/08.
 * Copyright 2008 Takashi Hashizume. All rights reserved.
 */

#ifndef _TATIME_H_
#define _TATIME_H_

#include <time.h>   /* strftime localtime */
#include <stdio.h>  /* snprintf */
#include <string.h> /* strcat */

#define timeval2sec(tv) ((double)((tv).tv_sec + (tv).tv_usec*1e-6))

#define timeval2usec(tv) ((tv).tv_sec * 1000000 + (tv).tv_usec)

#define timeval2str(strp, tv)                                           \
  {                                                                     \
    char msec[4] = "000";                                               \
    (void) strftime((strp), sizeof((strp)), "%Y-%m-%d %H:%M:%S.",       \
                    localtime(&((tv).tv_sec)));                         \
    snprintf(msec, 4, "%03ld", (long int)((tv).tv_usec / 1000));        \
    strcat((strp), msec);                                               \
  }

#define timermlt(tvp, multi, vvp)                                       \
  do                                                                    \
  {                                                                     \
    (vvp)->tv_sec = (tvp)->tv_sec * (multi);                            \
    (vvp)->tv_usec = (tvp)->tv_usec * (multi);                          \
    (vvp)->tv_sec += (vvp)->tv_usec / 1000000;                          \
    (vvp)->tv_usec = (vvp)->tv_usec % 1000000;                          \
  }                                                                     \
  while (0)

#define usec2timeval(usec, tvp)                                         \
  do                                                                    \
  {                                                                     \
    (tvp)->tv_sec = (usec) / 1000000;                                   \
    (tvp)->tv_usec = (usec) % 1000000;                                  \
  }                                                                     \
  while (0)

/* Operations on timevals. They should be defined in <time.h> */
#ifndef timerclear
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif
#ifndef timerisset
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#endif
#ifndef timercmp
#define timercmp(tvp, uvp, cmp)                                         \
        (((tvp)->tv_sec == (uvp)->tv_sec) ?                             \
            ((tvp)->tv_usec cmp (uvp)->tv_usec) :                       \
            ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif
#ifndef timeradd
#define timeradd(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;       \
                if ((vvp)->tv_usec >= 1000000) {                        \
                        (vvp)->tv_sec++;                                \
                        (vvp)->tv_usec -= 1000000;                      \
                }                                                       \
        } while (0)
#endif
#ifndef timersub
#define timersub(tvp, uvp, vvp)                                         \
        do {                                                            \
                (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;          \
                (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;       \
                if ((vvp)->tv_usec < 0) {                               \
                        (vvp)->tv_sec--;                                \
                        (vvp)->tv_usec += 1000000;                      \
                }                                                       \
        } while (0)
#endif

#endif /* _TATIME_H_ */
