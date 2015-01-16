/*
 * OCSQL.c
 * OC
 *
 * Created by Takashi Hashizume on 04/14/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCSQL.h"

struct __OCSQL
{
  OCIStmt *stmthp;
  char *sql;
  ub2 stmt_type;
  ub4 bind_count;
  ub4 define_count;
  OCIDefine **defineps;
  char **values;
};
typedef struct __OCSQL __OCSQL;

OCSQL OCSQL_initWithSQL(OCIEnv *envhp, OCIError *errhp, const char *sql)
{
  struct __OCSQL *self = malloc(sizeof(struct __OCSQL));
  int errcode = 0;
#define ERR_MSG_SIZE 512
  char errmsg[ERR_MSG_SIZE];

  if (self == NULL)
    return NULL;

  memset(self, 0, sizeof(*self));

  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCIHandleAlloc((dvoid *) envhp,
                                      (dvoid **) &(self->stmthp),
                                      OCI_HTYPE_STMT, (size_t) 0,
                                      (dvoid **) 0));
  if (errcode != 0)
  {
    (void) OCIHandleFree((dvoid*) self->stmthp, OCI_HTYPE_STMT);
    return NULL;
  }
  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCIStmtPrepare(self->stmthp, errhp,
                                      (text *) sql, (ub4) strlen(sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0)
  {
    (void) OCIHandleFree((dvoid*) self->stmthp, OCI_HTYPE_STMT);
    return NULL;
  }

  self->sql = malloc(sizeof(char) * (strlen(sql) + 1));
  if (self->sql == NULL)
    return NULL;

  strcpy(self->sql, sql);

  OCIAttrGet((void *) self->stmthp, (ub4) OCI_HTYPE_STMT,
             (void *) &(self->stmt_type), (ub4 *) NULL,
             (ub4) OCI_ATTR_STMT_TYPE, errhp);
  OCIAttrGet((void *) self->stmthp, (ub4) OCI_HTYPE_STMT,
             (void *) &(self->bind_count), (ub4 *) NULL,
             (ub4) OCI_ATTR_BIND_COUNT, errhp);
  self->define_count = 0;
  self->defineps = NULL;
  self->values = NULL;

  return self;
}

void OCSQL_release(OCSQL self)
{
  char *sql;
  ub4 bind_count;
  ub4 define_count;
  OCIDefine **defineps;
  char **values;

  int i = 0;

  (void) OCIHandleFree((dvoid*) self->stmthp, OCI_HTYPE_STMT);
  free(self->sql);
  free(self->defineps);
  for (i = 0; i < self->define_count; i++)
  {
    if (self->values[i] != NULL)
      free(self->values[i]);
  }
  if (self->values != NULL)
    free(self->values);

  free(self);
}

sword OCSQL_execute(OCSQL self, OCIError *errhp, OCISvcCtx *svchp, ...)
{
  va_list ap;
  OCIBind **bndps = NULL;
  ub4 iters = 1;
  char *valuep;
  sword status = 0;
  int i = 0;

  va_start(ap, svchp);

  bndps = malloc(sizeof(OCIBind *) * self->bind_count);

  for (i = 0; i < self->bind_count; i++)
  {
    valuep = va_arg(ap, char *);
    status = OCIBindByPos(self->stmthp, &bndps[i], errhp, i + 1,
                          (dvoid *) valuep, (sword) strlen(valuep) + 1,
                          SQLT_STR, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0,
                          (ub4 *) 0, OCI_DEFAULT);
    if (status != 0)
    {
      va_end(ap);
      return status;
    }
  }
  va_end(ap);

  if (self->stmt_type == OCI_STMT_SELECT)
    iters = 0;
  else
    iters = 1;

  status = OCIStmtExecute(svchp, self->stmthp, errhp, (ub4) iters, (ub4) 0,
                          (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL,
                          OCI_DEFAULT);
  free(bndps);
  return status;
}

int OCSQL_defineCount(OCSQL self, OCIError *errhp)
{
  if (self->define_count == 0)
    OCIAttrGet((void *) self->stmthp, (ub4) OCI_HTYPE_STMT,
               (void *) &(self->define_count), (ub4 *) NULL,
               (ub4) OCI_ATTR_PARAM_COUNT, errhp);

  return (int) self->define_count;
}

sword OCSQL_fetch(OCSQL self, OCIError *errhp)
{
  OCIParam *parmdp = (OCIParam *) 0;
  ub2 dtype;
  text *col_name;
  ub4 col_name_len = 0;
  ub4 char_semantics = 0;
  ub2 col_width = 0;
  sword status = 0;
  int i = 0;

  if (self->defineps == NULL)
  {
    self->defineps = malloc(sizeof(OCIDefine *) * OCSQL_defineCount(self,
                                                                    errhp));

    self->values = malloc(sizeof(char *) * OCSQL_defineCount(self, errhp));

    for (i = 0; i < self->define_count; i++)
    {
      /* Request a parameter descriptor for position i+1 in the select-list */
      status = OCIParamGet((void *) self->stmthp, (ub4) OCI_HTYPE_STMT, errhp,
                           (void **) &parmdp, (ub4) i + 1);
      if (status != 0) return status;
      /* Retrieve the datatype attribute */
      status = OCIAttrGet((void *) parmdp, (ub4) OCI_DTYPE_PARAM,
                          (void *) &dtype, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
                          errhp);
      if (status != 0) return status;
      /* Retrieve the column name attribute */
      status = OCIAttrGet((void *) parmdp, (ub4) OCI_DTYPE_PARAM,
                          (void **) &col_name, (ub4 *) &col_name_len,
                          (ub4) OCI_ATTR_NAME, errhp);
      if (status != 0) return status;

      /* Retrieve the length semantics for the column */
      status = OCIAttrGet((void *) parmdp, (ub4) OCI_DTYPE_PARAM,
                          (void *) &char_semantics, (ub4 *) 0,
                          (ub4) OCI_ATTR_CHAR_USED, errhp);
      if (status != 0) return status;
      if (char_semantics)
      {
        /* Retrieve the column width in characters */
        status = OCIAttrGet((void *) parmdp, (ub4) OCI_DTYPE_PARAM,
                            (void *) &col_width, (ub4 *) 0,
                            (ub4) OCI_ATTR_CHAR_SIZE, errhp);
        if (status != 0) return status;
      }
      else
      {
        /* Retrieve the column width in bytes */
        status = OCIAttrGet((void *) parmdp, (ub4) OCI_DTYPE_PARAM,
                            (void*) &col_width, (ub4 *) 0,
                            (ub4) OCI_ATTR_DATA_SIZE, errhp);
        if (status != 0) return status;
      }

      self->values[i] = malloc(sizeof(char) * (col_width + 1));

      status = OCIDefineByPos(self->stmthp, &self->defineps[i], errhp, i + 1,
                              (void *) self->values[i], (sb4) col_width + 1,
                              SQLT_STR, (void *) 0, (ub2 *) 0, (ub2 *) 0,
                              OCI_DEFAULT);
      if (status != 0) return status;
    }
  }

  status = OCIStmtFetch2(self->stmthp, errhp, (ub4) 1, OCI_FETCH_NEXT, (sb4) 0,
                         OCI_DEFAULT);
  return status;
}

char *OCSQL_valueByPos(OCSQL self, int pos)
{
  return self->values[pos - 1];
}

sword OCSQL_fetchInto(OCSQL self, OCIError *errhp, ...)
{
  va_list ap;
  char *valuep;
  sword status = 0;
  int i = 0;

  status = OCSQL_fetch(self, errhp);
  if (status != 0)
    return status;

  va_start(ap, errhp);
  for (i = 0; i < OCSQL_defineCount(self, errhp); i++)
  {
    valuep = va_arg(ap, char *);
    sprintf(valuep, "%s", OCSQL_valueByPos(self, i + 1));
  }
  va_end(ap);

  return status;
}

