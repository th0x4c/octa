/*
 * OCOracle.h
 * OC
 *
 * Created by Takashi Hashizume on 04/06/08.
 * Copyright 2008 Takashi Hashizume. All rights reserved.
 */

#ifndef _OCORACLE_H_
#define _OCORACLE_H_

#include <stdlib.h> /* malloc free */
#include <string.h> /* memset strcpy strlen */
#include <stdio.h>  /* snprintf */
#include <oci.h>

typedef struct __OCOracle *OCOracle;

#define IGNORE_OCI_NO_DATA
#define OCOCIERROR(errhp, errmsg, errmsgsize, function)       \
        OCOracle_OCIError(__FILE__, __LINE__, (errhp),        \
                          (errmsg), (errmsgsize), (function))

OCOracle OCOracle_init();
void OCOracle_release(OCOracle self);
int OCOracle_errorCode(OCOracle self);
char *OCOracle_errorMessage(OCOracle self);
int OCOracle_connect(OCOracle self, const char *username, const char *password,
                     const char *dbname);
int OCOracle_execTX(OCOracle self, void **inout,
                    int (*TX)(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize));
sword OCOracle_OCIError(const char *fname, int lineno, OCIError *errhp,
                        char *errmsg, size_t errmsgsize, sword status);

#endif /* _OCORACLE_H_ */
