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

  if (option->mode == OCTA_TPCB)
  {
    switch (option->command)
    {
    case OCTA_SETUP:
      return OCTABSetup_main(option);
    case OCTA_LOAD:
      return OCTABLoad_main(option);  
    case OCTA_BENCH:
      return OCTABBench_main(option);  
    case OCTA_TEARDOWN:
      return OCTABTeardown_main(option);  
    default:
      break;
    }
  }

  if (option->mode == OCTA_TPCC)
  {
    switch (option->command)
    {
    case OCTA_SETUP:
      return OCTACSetup_main(option);
    case OCTA_LOAD:
      return OCTACLoad_main(option);
    case OCTA_BENCH:
      return OCTACBench_main(option);
    case OCTA_TEARDOWN:
      return OCTACTeardown_main(option);
    default:
      break;
    }
  }

  free(option);

  return 0;
}
