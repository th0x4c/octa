/*
 * OCTADDL.h
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCTADDL_H_
#define _OCTADDL_H_

#include <stdio.h>  /* snprintf printf */
#include <string.h> /* strlen strncpy */
#include <oci.h>    /* OCIHandleAlloc OCIStmtPrepare OCIStmtExecute *
                     * OCIHandleFree                                */
#include <OC/OC.h>

struct OCTADDLInput
{
#define MAX_SQL_COUNT 64
#define MAX_SQL_SIZE 2048
  char sqls[MAX_SQL_COUNT][MAX_SQL_SIZE];
  int sql_count;
#define TABLESPACE_NAME_SIZE 30
  char tablespace[TABLESPACE_NAME_SIZE];
};
typedef struct OCTADDLInput OCTADDLInput;

int OCTADDL_TX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp, void **inout,
               char *errmsg, size_t errmsgsize);

#endif /* _OCTADDL_H_ */

