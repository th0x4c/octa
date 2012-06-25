/*
 * OCTASetup.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/30/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTASetup.h"

int OCTASetup_main(const OCTAOption *option, OCTADDLInput create_table,
                   int (*load)(const OCTAOption *opt),
                   OCTADDLInput create_index, OCTADDLInput add_constraint,
                   OCTADDLInput analyze)
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

  /* Create tables */
  *ddlinput = create_table;
  strncpy(ddlinput->tablespace, option->table_tablespace,
          TABLESPACE_NAME_SIZE - 1);
  ddlinput->tablespace[TABLESPACE_NAME_SIZE - 1] = '\0';
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  OCOracle_release(oracle);

  /* Load data */
  ret = load(option);

  oracle = OCOracle_init();
  ret = OCOracle_connect(oracle, option->username, option->password,
                         option->tnsname);
  if (ret != 0) goto end;

  /* Create indexes */
  *ddlinput = create_index;
  strncpy(ddlinput->tablespace, option->index_tablespace,
          TABLESPACE_NAME_SIZE - 1);
  ddlinput->tablespace[TABLESPACE_NAME_SIZE - 1] = '\0';
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  /* Add constraints */
  *ddlinput = add_constraint;
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  /* Analyze */
  *ddlinput = analyze;
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
