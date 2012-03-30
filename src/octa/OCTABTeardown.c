/*
 * OCTABTeardown.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTABTeardown.h"

int OCTABTeardown_main(const OCTAOption *option)
{
  static OCTADDLInput drop_table_input = OCTAB_DROP_TABLE_INPUT;

  return OCTATeardown_main(option, drop_table_input);
}
