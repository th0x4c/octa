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
  static OCTADDLInput create_table_input = OCTAB_CREATE_TABLE_INPUT;
  static OCTADDLInput create_index_input = OCTAB_CREATE_INDEX_INPUT;
  static OCTADDLInput add_constraint_input = OCTAB_ADD_CONSTRAINT_INPUT;
  static OCTADDLInput analyze_input = OCTAB_ANALYZE_INPUT;

  return OCTASetup_main(option, create_table_input, OCTABLoad_main,
                        create_index_input, add_constraint_input,
                        analyze_input);
}
