/*
 * OCSQL.h
 * OC
 *
 * Created by Takashi Hashizume on 04/14/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCSQL_H_
#define _OCSQL_H_

#include <stdlib.h> /* malloc free */
#include <string.h> /* memset strcpy strlen */
#include <stdarg.h> /* va_start va_arg va_end */
#include <stdio.h>  /* sprintf */
#include <oci.h>
#include "OCOracle.h"

typedef struct __OCSQL *OCSQL;

OCSQL OCSQL_initWithSQL(OCIEnv *envhp, OCIError *errhp, const char *sql);
void OCSQL_release(OCSQL self);
sword OCSQL_execute(OCSQL self, OCIError *errhp, OCISvcCtx *svchp, ...);
int OCSQL_defineCount(OCSQL self, OCIError *errhp);
sword OCSQL_fetch(OCSQL self, OCIError *errhp);
char *OCSQL_valueByPos(OCSQL self, int pos);
sword OCSQL_fetchInto(OCSQL self, OCIError *errhp, ...);

#endif /* _OCSQL_H_ */
