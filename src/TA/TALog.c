/*
 * TALog.c
 * TA
 *
 * Created by Takashi Hashizume on 08/04/11.
 * Copyright 2011 Takashi Hashizume. All rights reserved.
 */

#include "TALog.h"

struct __TALog
{
  int level;
  char filename[FILENAME_MAX];
  FILE *stream;
};

static void TALog_log(TALog self, const char *level_str, const char *str);

TALog TALog_initWithFilename(const char *filename)
{
  struct __TALog *self = malloc(sizeof(struct __TALog));
  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->level = TALog_INFO;
  if (filename == NULL)
  {
    strcpy(self->filename, "");
    self->stream = stdout;
  }
  else
  {
    strcpy(self->filename, filename);
    if ((self->stream = fopen(filename, "a")) == NULL)
    {
      free(self);
      return NULL;
    }
  }

  return self;
}

void TALog_release(TALog self)
{
  if (self->stream != stdout)
    fclose(self->stream);

  free(self);
}

size_t TALog_sizeof()
{
  return sizeof(struct __TALog);
}

TALog TALog_nextAddr(TALog self)
{
  return self + 1;
}

void TALog_deepCopy(TALog self, TALog dest)
{
  dest->level = self->level;
  strcpy(dest->filename, self->filename);
  if (self->stream == stdout)
    dest->stream = stdout;
  else
    dest->stream = fopen(self->filename, "a");
}

int TALog_level(TALog self)
{
  return self->level;
}

void TALog_setLevel(TALog self, int level)
{
  self->level = level;
}

void TALog_fatal(TALog self, const char *str)
{
  if (self->level >= TALog_FATAL)
    TALog_log(self, "FATAL", str);
}

void TALog_error(TALog self, const char *str)
{
  if (self->level >= TALog_ERROR)
    TALog_log(self, "ERROR", str);
}

void TALog_warn(TALog self, const char *str)
{
  if (self->level >= TALog_WARN)
    TALog_log(self, "WARN", str);
}

void TALog_info(TALog self, const char *str)
{
  if (self->level >= TALog_INFO)
    TALog_log(self, "INFO", str);
}

void TALog_debug(TALog self, const char *str)
{
  if (self->level >= TALog_DEBUG)
    TALog_log(self, "DEBUG", str);
}

/* private */
static void TALog_log(TALog self, const char *level_str, const char *str)
{
  struct timeval now_tv;
  char strnow[24] = "0000-00-00 00:00:00.000";

  timerclear(&now_tv);

  gettimeofday(&now_tv, (struct timezone *)0);
  timeval2str(strnow, now_tv);

  fprintf(self->stream, "%s [%s] %s\n", strnow, level_str, str);
  fflush(self->stream);
}
