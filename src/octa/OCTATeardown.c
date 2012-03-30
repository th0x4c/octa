/*
 * OCTATeardown.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/30/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTATeardown.h"

int OCTATeardown_main(const OCTAOption *option, OCTADDLInput drop_table)
{
  OCTADDLInput *ddlinput;
  OCOracle oracle = OCOracle_init();
  int ret = 0;

  ddlinput = malloc(sizeof(OCTADDLInput));
  if (ddlinput == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTADDLInput\n");
    exit(1);
  }
  memset(ddlinput, 0, sizeof(*ddlinput));

  ret = OCOracle_connect(oracle, option->username, option->password,
                         option->tnsname);
  if (ret != 0) goto end;

  /* Drop tables */
  *ddlinput = drop_table;
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

 end:
  if (ret != 0)
    fprintf(stderr, "err: %d, msg: %s\n", OCOracle_errorCode(oracle),
            OCOracle_errorMessage(oracle));

  OCOracle_release(oracle);
  free(ddlinput);

  return ret;
}
