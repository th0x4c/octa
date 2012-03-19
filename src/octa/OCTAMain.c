/*
 * OCTAMain.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTAMain.h"

int main(int argc, char *argv[])
{
  OCTAOption *option;
  int ret = 0;

  option = malloc(sizeof(OCTAOption));
  if (option == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTATPCBOption\n");
    exit(1);
  }
  memset(option, 0, sizeof(*option));

  OCTAOption_getOption(argc, argv, option);
  switch (option->command)
  {
  case OCTA_SETUP:
    return OCTABSetup_main(option);
    break;
  case OCTA_LOAD:
    return OCTABLoad_main(option);  
    break;
  case OCTA_TEARDOWN:
    return OCTABTeardown_main(option);  
    break;
  default:
    break;
  }

  free(option);

  return 0;
}
