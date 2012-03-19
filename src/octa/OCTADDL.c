/*
 * OCTADDL.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTADDL.h"

int OCTADDL_TX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp, void **inout,
               char *errmsg, size_t errmsgsize)
{
  OCTADDLInput *input = (OCTADDLInput *)*inout;
  OCIStmt   *stmthp = (OCIStmt *)0;
  int errcode = 0;
  char sql[MAX_SQL_SIZE] = "";
  int i = 0;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp,
                                      OCI_HTYPE_STMT, (size_t) 0,
                                      (dvoid **) 0));
  if (errcode != 0) goto end;

  for (i = 0; i < input->sql_count; i++)
  {
    if (strlen(input->tablespace) > 0)
      snprintf(sql, MAX_SQL_SIZE, input->sqls[i], input->tablespace);
    else
    {
      strncpy(sql, input->sqls[i], MAX_SQL_SIZE);
      sql[MAX_SQL_SIZE - 1] = '\0';
    }

    printf("SQL> %s\n\n", sql);

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmthp, errhp, (text *)sql,
                                        (ub4) strlen(sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                                        (CONST OCISnapshot *) NULL,
                                        (OCISnapshot *) NULL, OCI_DEFAULT));
    /* ignore ORA-942 when dropping a table */
    if (errcode == 942)
    {
      printf("Ignore %s\n", errmsg);
      errcode = 0;
    }
    if (errcode != 0) goto end;
  }

 end:
  (void) OCIHandleFree((dvoid*) stmthp, OCI_HTYPE_STMT);

  return errcode;
}
