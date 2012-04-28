/*
 * OCTACTeardown.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/01/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACTeardown.h"

int OCTACTeardown_main(const OCTAOption *option)
{
  static OCTADDLInput drop_table_input = OCTAC_DROP_TABLE_INPUT;

  return OCTATeardown_main(option, drop_table_input);
}

