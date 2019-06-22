/*
 * test_OCSQL.c
 * OC
 *
 * Created by Takashi Hashizume on 04/14/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include <OC/OCSQL.h>
#include "../munit.h"
#include <stdio.h>       /* printf */
#include <string.h>      /* strlen */

int mu_nfail=0;
int mu_ntest=0;
int mu_nassert=0;

static text *username = (text *) "SCOTT";
static text *password = (text *) "tiger";
static text *dbname   = (text *) "XE";
static OCIEnv *envhp;
static OCIError *errhp;
static  OCISvcCtx *svchp;

static OCSQL sql = NULL;

static void setup()
{
  OCISession *authp = (OCISession *) 0;
  OCIServer *srvhp;
  sword errcode = 0;
#define ERR_MSG_SIZE 512
  char errmsg[ERR_MSG_SIZE];

  errcode = OCIEnvCreate((OCIEnv **) &envhp, (ub4) OCI_DEFAULT,
                  (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                  (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                  (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0);

  if (errcode != 0) {
    (void) printf("OCIEnvCreate failed with errcode = %d.\n", errcode);
    exit(1);
  }

  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR,
                   (size_t) 0, (dvoid **) 0);

  /* server contexts */
  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
                   (size_t) 0, (dvoid **) 0);

  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
                   (size_t) 0, (dvoid **) 0);

  (void) OCIServerAttach( srvhp, errhp, (text *)dbname, strlen((char *)dbname),
                          0);

  /* set attribute server context in the service context */
  (void) OCIAttrSet( (dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *)srvhp,
                     (ub4) 0, OCI_ATTR_SERVER, (OCIError *) errhp);

  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **)&authp,
                        (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);

  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) username, (ub4) strlen((char *)username),
                 (ub4) OCI_ATTR_USERNAME, errhp);

  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) password, (ub4) strlen((char *)password),
                 (ub4) OCI_ATTR_PASSWORD, errhp);

  OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
             OCISessionBegin ( svchp,  errhp, authp, OCI_CRED_RDBMS,
                               (ub4) OCI_DEFAULT));

  (void) OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                   (dvoid *) authp, (ub4) 0,
                   (ub4) OCI_ATTR_SESSION, errhp);
}

static void teardown()
{
  if (sql)
    OCSQL_release(sql);

  if (envhp)
    (void) OCIHandleFree((dvoid *) envhp, OCI_HTYPE_ENV);
  return;
}

static void test_OCSQL_execute()
{
  static char *sqltxt = "SELECT * FROM emp WHERE empno = :1";
#define ERR_MSG_SIZE 512
  char errmsg[ERR_MSG_SIZE];
  sword errcode = 0;
  char *empno = "7934";

  sql = OCSQL_initWithSQL(envhp, errhp, sqltxt);
  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCSQL_execute(sql, errhp, svchp, empno));
  if (errcode !=0)
    printf("%s\n", errmsg);

  mu_assert(errcode == 0);
}

static void test_OCSQL_fetch()
{
#define ERR_MSG_SIZE 512
  char errmsg[ERR_MSG_SIZE];
  sword errcode = 0;
  int define_count = 0;
  int i = 0;
  OCSQL sqlemp = NULL;

  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE, OCSQL_fetch(sql, errhp));
  if (errcode != 0)
    printf("%s\n", errmsg);

  mu_assert(errcode == 0 || errcode == 1405);
  mu_assert(strcmp(OCSQL_valueByPos(sql, 2), "MILLER") == 0);

  for (i = 0; i < OCSQL_defineCount(sql, errhp); i++)
  {
    printf("\"%s\" ", OCSQL_valueByPos(sql, i + 1));
  }
  printf("\n");

  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE, OCSQL_fetch(sql, errhp));
  mu_assert(errcode == OCI_NO_DATA);

  sqlemp = OCSQL_initWithSQL(envhp, errhp, "SELECT * FROM emp ORDER BY empno");
  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCSQL_execute(sqlemp, errhp, svchp));
  while (OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE, OCSQL_fetch(sqlemp, errhp)) !=
         OCI_NO_DATA)
  {
    for (i = 0; i < OCSQL_defineCount(sqlemp, errhp); i++)
    {
      printf("\"%s\" ", OCSQL_valueByPos(sqlemp, i + 1));
    }
    printf("\n");
  }
  mu_assert(strcmp(OCSQL_valueByPos(sql, 2), "MILLER") == 0);
  OCSQL_release(sqlemp);
}

static void test_OCSQL_fetchInto()
{
#define ERR_MSG_SIZE 512
  char errmsg[ERR_MSG_SIZE];
  sword errcode = 0;
  OCSQL sqlemp = NULL;
  char empno[5];  /* NUMBER(4) */
  char ename[11]; /* VARCHAR2(10) */

  sqlemp = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT empno, ename FROM emp "
                             "WHERE empno = 7934");
  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCSQL_execute(sqlemp, errhp, svchp));
  errcode = OCOCIERROR(errhp, errmsg, ERR_MSG_SIZE,
                       OCSQL_fetchInto(sqlemp, errhp, empno, ename));
  mu_assert(errcode == 0);
  mu_assert(strcmp(empno, "7934") == 0);
  mu_assert(strcmp(ename, "MILLER") == 0);
  OCSQL_release(sqlemp);
}

int main(int argc, char *argv[])
{
  setup();
  mu_run_test(test_OCSQL_execute);
  mu_run_test(test_OCSQL_fetch);
  mu_run_test(test_OCSQL_fetchInto);
  teardown();
  mu_show_failures();
  return mu_nfail != 0;
}
