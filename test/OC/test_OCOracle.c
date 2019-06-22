/*
 * test_OCOracle.c
 * OC
 *
 * Created by Takashi Hashizume on 03/14/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <OC/OCOracle.h>
#include "../munit.h"

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

struct txinout
{
  int empno;
#define ENAME_SIZE 10
  char ename[ENAME_SIZE];
};
typedef struct txinout txinout;

static int TXSample(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                    void **inout, char *errmsg, size_t errmsgsize)
{
  OCIStmt   *stmthp = (OCIStmt *)0;
  OCIDefine *defnp  = (OCIDefine *)0;
  OCIBind   *bnd1p  = (OCIBind *)0;
  int errcode = 0;

  static char *tx1sql = "SELECT ename FROM emp WHERE empno = :1";

  txinout *txiop = (txinout *)*inout;
  int empno = txiop->empno;
  char ename[ENAME_SIZE] = "";

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp,
                                      OCI_HTYPE_STMT, (size_t) 0,
                                      (dvoid **) 0));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp, (text *)tx1sql,
                                      (ub4) strlen((char *) tx1sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1, (dvoid *) &empno,
                                    (sword) sizeof(empno), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0,
                                    (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIDefineByPos(stmthp, &defnp, errhp, 1,
                                      (dvoid *) ename, (sword) sizeof(ename),
                                      SQLT_STR, (dvoid *) 0, (ub2 *) 0,
                                      (ub2 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0,
                                      (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

 end:
  (void) OCIHandleFree((dvoid*) stmthp, OCI_HTYPE_STMT);

  snprintf(txiop->ename, ENAME_SIZE, "%s", ename);
  return errcode;
}

static void test_OCOracle_execTX()
{
  OCOracle oracle = OCOracle_init();
  txinout *txiop = NULL;
  int errcode = 0;

  txiop = malloc(sizeof(txinout));
  if (txiop == NULL)
  {
    fprintf(stdout, "Cannot malloc txinout\n");
    exit(1);
  }
  errcode = OCOracle_connect(oracle, "scott", "tiger", "xe");
  if (errcode != 0)
    printf("err: %d, msg: %s", OCOracle_errorCode(oracle),
           OCOracle_errorMessage(oracle));

  txiop->empno = 7934;
  errcode = OCOracle_execTX(oracle, (void **)&txiop, TXSample);
  if (errcode != 0)
    printf("err: %d, msg: %s\n", OCOracle_errorCode(oracle),
           OCOracle_errorMessage(oracle));

  mu_assert(errcode == 0);

  printf("empno: %d, ename: %s\n", txiop->empno, txiop->ename);
  mu_assert(strcmp(txiop->ename, "MILLER") == 0);

  OCOracle_release(oracle);
  free(txiop);
}

int main(int argc, char *argv[])
{
  mu_run_test(test_OCOracle_execTX);
  mu_show_failures();
  return mu_nfail != 0;
}
