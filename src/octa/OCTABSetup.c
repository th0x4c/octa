/*
 * OCTABSetup.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTABSetup.h"

int OCTABSetup_main(const OCTAOption *option)
{
  OCTADDLInput *ddlinput;
  static OCTADDLInput create_table_input = CREATE_TABLE_INPUT;
  static OCTADDLInput create_index_input = CREATE_INDEX_INPUT;
  static OCTADDLInput add_constraint_input = ADD_CONSTRAINT_INPUT;
  static OCTADDLInput analyze_input = ANALYZE_INPUT;
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
  *ddlinput = create_table_input;
  strncpy(ddlinput->tablespace, option->table_tablespace,
          TABLESPACE_NAME_SIZE - 1);
  ddlinput->tablespace[TABLESPACE_NAME_SIZE - 1] = '\0';
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  /* Load data */
  ret = OCTABLoad_main(option);

  /* Create indexes */
  *ddlinput = create_index_input;
  strncpy(ddlinput->tablespace, option->index_tablespace,
          TABLESPACE_NAME_SIZE - 1);
  ddlinput->tablespace[TABLESPACE_NAME_SIZE - 1] = '\0';
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  /* Add constraints */
  *ddlinput = add_constraint_input;
  ret = OCOracle_execTX(oracle, (void **)&ddlinput, OCTADDL_TX);
  if (ret != 0) goto end;

  /* Analyze */
  *ddlinput = analyze_input;
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

