/*
 * OCTACSetup.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/31/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACSetup.h"
#include "OCTACCreateTable.h"
#include "OCTACCreateIndex.h"
#include "OCTACAddConstraint.h"
#include "OCTACAnalyze.h"

int OCTACSetup_main(const OCTAOption *option)
{
  static OCTADDLInput create_table_input = OCTAC_CREATE_TABLE_INPUT;
  static OCTADDLInput create_index_input = OCTAC_CREATE_INDEX_INPUT;
  static OCTADDLInput add_constraint_input = OCTAC_ADD_CONSTRAINT_INPUT;
  static OCTADDLInput analyze_input = OCTAC_ANALYZE_INPUT;

  return OCTASetup_main(option, create_table_input, OCTACLoad_main,
                        create_index_input, add_constraint_input,
                        analyze_input);
}

