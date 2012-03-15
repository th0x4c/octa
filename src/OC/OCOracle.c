/*
 * OCOracle.c
 * OC
 *
 * Created by Takashi Hashizume on 04/06/08.
 * Copyright 2008 Takashi Hashizume. All rights reserved.
 */

#include "OCOracle.h"

struct __OCOracle
{
  OCIEnv *envhp;
  OCIError *errhp;
  OCISession *authp;
  OCIServer *srvhp;
  OCISvcCtx *svchp;
  int error_code;
#define ERR_MSG_SIZE 512
  char error_message[ERR_MSG_SIZE];
};

OCOracle OCOracle_init()
{
  struct __OCOracle *self = malloc(sizeof(struct __OCOracle));
  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  self->envhp = NULL;
  self->errhp = NULL;
  self->authp = NULL;
  self->srvhp = NULL;
  self->svchp = NULL;
  self->error_code = 0;
  strcpy(self->error_message, "");

  return self;
}

void OCOracle_release(OCOracle self)
{
  OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
             OCISessionEnd(self->svchp, self->errhp, self->authp,
                           OCI_DEFAULT));
  OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
             OCIServerDetach(self->srvhp, self->errhp, OCI_DEFAULT));
  OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
             OCIHandleFree((dvoid *)self->authp, OCI_HTYPE_SESSION));
  OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
             OCIHandleFree((dvoid *)self->svchp, OCI_HTYPE_SVCCTX));
  OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
             OCIHandleFree((dvoid *)self->srvhp, OCI_HTYPE_SERVER));
  OCIHandleFree((dvoid *)self->errhp, OCI_HTYPE_ERROR);
  OCIHandleFree((dvoid *)self->envhp, OCI_HTYPE_ENV);
  free(self);
}

int OCOracle_errorCode(OCOracle self)
{
  return self->error_code;
}

char *OCOracle_errorMessage(OCOracle self)
{
  return self->error_message;
}

int OCOracle_connect(OCOracle self, const char *username, const char *password,
                     const char *dbname)
{
  self->error_code = OCOCIERROR((OCIError *)self->envhp,
                                self->error_message,
                                ERR_MSG_SIZE,
                                OCIEnvCreate((OCIEnv **) &self->envhp,
                                             (ub4) OCI_DEFAULT, (dvoid *) 0,
                                             (dvoid * (*)(dvoid *,size_t)) 0,
                                             (dvoid * (*)(dvoid *,
                                                          dvoid *,
                                                          size_t)) 0,
                                             (void (*)(dvoid *, dvoid *)) 0,
                                             (size_t) 0, (dvoid **) 0));
  if (self->error_code != OCI_SUCCESS)
    return self->error_code;

  (void) OCIHandleAlloc((dvoid *) self->envhp, (dvoid **) &self->errhp,
                         OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);

  /* server contexts */
  (void) OCIHandleAlloc((dvoid *) self->envhp, (dvoid **) &self->srvhp,
                        OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0);

  (void) OCIHandleAlloc((dvoid *) self->envhp, (dvoid **) &self->svchp,
                        OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0);

  (void) OCIServerAttach(self->srvhp, self->errhp, (text *) dbname,
                         strlen((char *) dbname), 0);

  /* set attribute server context in the service context */
  (void) OCIAttrSet((dvoid *) self->svchp, OCI_HTYPE_SVCCTX,
                    (dvoid *) self->srvhp, (ub4) 0, OCI_ATTR_SERVER,
                    (OCIError *) self->errhp);

  (void) OCIHandleAlloc((dvoid *) self->envhp, (dvoid **) &self->authp,
			(ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);

  (void) OCIAttrSet((dvoid *) self->authp, (ub4) OCI_HTYPE_SESSION,
		    (dvoid *) username, (ub4) strlen((char *) username),
		    (ub4) OCI_ATTR_USERNAME, self->errhp);

  (void) OCIAttrSet((dvoid *) self->authp, (ub4) OCI_HTYPE_SESSION,
		    (dvoid *) password, (ub4) strlen((char *) password),
		    (ub4) OCI_ATTR_PASSWORD, self->errhp);

  self->error_code = OCOCIERROR(self->errhp, self->error_message, ERR_MSG_SIZE,
                                OCISessionBegin(self->svchp, self->errhp,
                                                self->authp, OCI_CRED_RDBMS,
                                                (ub4) OCI_DEFAULT));

  (void) OCIAttrSet((dvoid *) self->svchp, (ub4) OCI_HTYPE_SVCCTX,
		    (dvoid *) self->authp, (ub4) 0, (ub4) OCI_ATTR_SESSION,
                    self->errhp);

  return self->error_code;
}

int OCOracle_execTX(OCOracle self, void **inout,
                    int (*TX)(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize))
{
  self->error_code = TX(self->envhp, self->errhp, self->svchp, inout,
                        self->error_message, ERR_MSG_SIZE);
  return self->error_code;
}

sword OCOracle_OCIError(const char *fname, int lineno, OCIError *errhp,
                        char *errmsg, size_t errmsgsize, sword status)
{
  text errbuf[512];
  sb4 errcode;

  switch (status)
  {
    case OCI_SUCCESS:
      break;
    case OCI_SUCCESS_WITH_INFO:
      OCIErrorGet((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode, errbuf,
                  (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
      snprintf(errmsg, errmsgsize,
               "Module %s Line %d Error - OCI_SUCCESS_WITH_INFO Error- %s",
               fname, lineno, errbuf);
      break;
    case OCI_NEED_DATA:
      snprintf(errmsg, errmsgsize, "Module %s Line %d Error - OCI_NEED_DATA",
               fname, lineno);
      break;
    case OCI_NO_DATA:
#ifndef IGNORE_OCI_NO_DATA
      snprintf(errmsg, errmsgsize, "Module %s Line %d Error - OCI_NO_DATA",
               fname, lineno);
#else
      return 0;
#endif
      break;
    case OCI_ERROR:
      OCIErrorGet((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode, errbuf,
                  (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
      snprintf(errmsg, errmsgsize, "Module %s Line %d Error - %s",
               fname, lineno, errbuf);
      return (sword) errcode;
    case OCI_INVALID_HANDLE:
      snprintf(errmsg, errmsgsize,
               "Module %s Line %d Error - OCI_INVALID_HANDLE", fname, lineno);
      break;
    case OCI_STILL_EXECUTING:
      snprintf(errmsg, errmsgsize,
               "Module %s Line %d Error - OCI_STILL_EXECUTE", fname, lineno);
      break;
    case OCI_CONTINUE:
      snprintf(errmsg, errmsgsize, "Module %s Line %d Error - OCI_CONTINUE",
               fname, lineno);
      break;
    default:
      snprintf(errmsg, errmsgsize, "Module %s Line %d Status - %d",
               fname, lineno, status);
      break;
  }
  return status;
}
