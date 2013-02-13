/*
 * OCTABBench.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/20/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTABBench.h"

struct OCTABBenchInput
{
  OCTAOption option;
  OCOracle oracle;
  int session_id;
  int amount;
  int branch_id;
  int teller_id;
  int account_id;
};
typedef struct OCTABBenchInput OCTABBenchInput;

static void OCTABBench_beforeSetup(TASessionManager self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  OCTAOption option = io->option;
  struct timeval start_time;
  char start_time_str[24] = "0000-00-00 00:00:00.000";
  int i = 0;

  OCTAOption_print(option);

  timerclear(&start_time);
  gettimeofday(&start_time, (struct timezone *)0);
  timeval2str(start_time_str, start_time);

  for (i = 0; i < TASessionManager_numberOfSessions(self); i++)
  {
    TASession_setPeriodInterval(TASessionManager_sessions(self)[i], start_time,
                                (int) timeval2sec(option.rampup_time),
                                (int) timeval2sec(option.measurement_interval),
                                (int) timeval2sec(option.rampdown_time));
  }

  printf("----------------------------------------------------------------\n");
  printf("              Start Time : %s\n", start_time_str);
  printf("----------------------------------------------------------------\n");
}

static void OCTABBench_setup(TASession self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;

  io->session_id = TASession_ID(self);
  io->oracle = OCOracle_init();
  OCOracle_connect(io->oracle, io->option.username, io->option.password,
                   io->option.tnsname);
  TASession_setPeriod(self, TASession_RAMPUP);
  TASession_setStatus(self, TASession_RUNNING);

  srandom(time(NULL) * getpid());
}

static void OCTABBench_beforeTX(TASession self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  int last_branch_id = io->option.scale_factor;
  int account_branch;

  io->amount = (int) random() % 1999999 - 999999;
  io->teller_id = (int) random() % (last_branch_id * TELLERS_PER_BRANCH) + 1;
  io->branch_id = (io->teller_id - 1) / TELLERS_PER_BRANCH + 1;

  account_branch = io->branch_id;
  if (last_branch_id > 1)
  {
    if (random() % 100 >= LOCAL_BRANCH_PERCENT)
    {
      while (account_branch == io->branch_id)
        account_branch = (int) random() % last_branch_id + 1;
    }
  }

  io->account_id = ACCOUNTS_PER_BRANCH * (account_branch - 1) +
                   random() % ACCOUNTS_PER_BRANCH + 1;
}

static int OCTABBench_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  static OCIStmt   *stmt1p = (OCIStmt *)NULL;
  static OCIStmt   *stmt2p = (OCIStmt *)NULL;
  static OCIStmt   *stmt3p = (OCIStmt *)NULL;
  static OCIStmt   *stmt4p = (OCIStmt *)NULL;
  static OCIStmt   *stmt5p = (OCIStmt *)NULL;
  OCIDefine *defnp  = (OCIDefine *)0;
  OCIBind   *bnd1p  = (OCIBind *)0;
  OCIBind   *bnd2p  = (OCIBind *)0;
  OCIBind   *bnd3p  = (OCIBind *)0;
  OCIBind   *bnd4p  = (OCIBind *)0;
  OCIBind   *bnd5p  = (OCIBind *)0;
  int errcode = 0;
  static char *update_account_sql = UPDATE_ACCOUNT_SQL;
  static char *select_account_sql = SELECT_ACCOUNT_SQL;
  static char *update_teller_sql = UPDATE_TELLER_SQL;
  static char *update_branch_sql = UPDATE_BRANCH_SQL;
  static char *insert_history_sql = INSERT_HISTORY_SQL;
  int account_balance;
  char info[20] = "00000 12345678901";

  /* UPDATE account */
  if (stmt1p == NULL)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmt1p,
                                        OCI_HTYPE_STMT, (size_t) 0,
                                        (dvoid **) 0));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmt1p, errhp,
                                        (text *)update_account_sql,
                                        (ub4) strlen(update_account_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt1p, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt1p, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmt1p, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* SELECT account */
  if (stmt2p == NULL)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmt2p,
                                        OCI_HTYPE_STMT, (size_t) 0,
                                        (dvoid **) 0));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmt2p, errhp,
                                        (text *)select_account_sql,
                                        (ub4) strlen(select_account_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt2p, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIDefineByPos(stmt2p, &defnp, errhp, 1,
                                      (dvoid *) &account_balance,
                                      (sword) sizeof(account_balance),
                                      SQLT_INT, (dvoid *) 0, (ub2 *) 0,
                                      (ub2 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmt2p, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  while (OCOCIERROR(errhp, errmsg, errmsgsize,
                    OCIStmtFetch2(stmt2p, errhp, (ub4) 1, OCI_FETCH_NEXT,
                                  (sb4) 0, OCI_DEFAULT)) != OCI_NO_DATA)
  {}

  /* UPDATE teller */
  if (stmt3p == NULL)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmt3p,
                                        OCI_HTYPE_STMT, (size_t) 0,
                                        (dvoid **) 0));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmt3p, errhp,
                                        (text *)update_teller_sql,
                                        (ub4) strlen(update_teller_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt3p, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt3p, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->teller_id),
                                    (sword) sizeof(io->teller_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmt3p, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* UPDATE branch */
  if (stmt4p == NULL)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmt4p,
                                        OCI_HTYPE_STMT, (size_t) 0,
                                        (dvoid **) 0));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmt4p, errhp,
                                        (text *)update_branch_sql,
                                        (ub4) strlen(update_branch_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt4p, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt4p, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->branch_id),
                                    (sword) sizeof(io->branch_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmt4p, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* INSERT history */
  snprintf(info, sizeof(info), "%5d 12345678901", io->session_id);

  if (stmt5p == NULL)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmt5p,
                                        OCI_HTYPE_STMT, (size_t) 0,
                                        (dvoid **) 0));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmt5p, errhp,
                                        (text *)insert_history_sql,
                                        (ub4) strlen(insert_history_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt5p, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->teller_id),
                                    (sword) sizeof(io->teller_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt5p, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->branch_id),
                                    (sword) sizeof(io->branch_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt5p, &bnd3p, errhp, 3,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt5p, &bnd4p, errhp, 4,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmt5p, &bnd5p, errhp, 5,
                                    (dvoid *) info,
                                    (sword) strlen(info) + 1, SQLT_STR,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmt5p, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  /* if (stmt1p != NULL) */
  /* { */
  /*   (void) OCIHandleFree((dvoid*) stmt1p, OCI_HTYPE_STMT); */
  /*   stmt1p = NULL; */
  /* } */
  /* if (stmt2p != NULL) */
  /* { */
  /*   (void) OCIHandleFree((dvoid*) stmt2p, OCI_HTYPE_STMT); */
  /*   stmt2p = NULL; */
  /* } */
  /* if (stmt3p != NULL) */
  /* { */
  /*   (void) OCIHandleFree((dvoid*) stmt3p, OCI_HTYPE_STMT); */
  /*   stmt3p = NULL; */
  /* } */
  /* if (stmt4p != NULL) */
  /* { */
  /*   (void) OCIHandleFree((dvoid*) stmt4p, OCI_HTYPE_STMT); */
  /*   stmt4p = NULL; */
  /* } */
  /* if (stmt5p != NULL) */
  /* { */
  /*   (void) OCIHandleFree((dvoid*) stmt5p, OCI_HTYPE_STMT); */
  /*   stmt5p = NULL; */
  /* } */

  return errcode;
}

static int OCTABBench_TX(TASession self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  return OCOracle_execTX(io->oracle, inout, OCTABBench_oracleTX);
}

static int OCTABBench_rollbackTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize)
{
  OCITransRollback(svchp, errhp, OCI_DEFAULT);
}

static void OCTABBench_errorTX(TASession self, void **inout, int error_code,
                              char *error_message, size_t error_message_size)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;

  OCOracle_execTX(io->oracle, inout, OCTABBench_rollbackTX);
  snprintf(error_message, error_message_size, "err: %d, msg: %s", 
           error_code, OCOracle_errorMessage(io->oracle));
}

static void OCTABBench_afterTX(TASession self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  struct timespec think_time;

  if (timerisset(&(io->option.think_time[0])))
  {
    think_time.tv_sec = io->option.think_time[0].tv_sec;
    think_time.tv_nsec = io->option.think_time[0].tv_usec * 1000;
    nanosleep(&think_time, NULL);
  }
}

static char *OCTABBench_selectTX(TASession self)
{
#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 64
#endif
  static char tx_name[MAX_NAME_SIZE] = "OCTAB bench";

  return tx_name;
}

static void OCTABBench_teardown(TASession self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  TATXStat stat = TASession_statByNameInPeriodInPhase(self, "OCTAB bench",
                                                      TASession_MEASUREMENT,
                                                      TASession_TX);
  TALog log = TASession_log(self);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  OCOracle_release(io->oracle);
  TALog_info(log, TATXStat_description(stat, desc, DESC_SIZE));
}

static void OCTABBench_monitor(TASessionManager self)
{
#define MONITOR_INTERVAL 4
#define PAGESIZE 20
  struct timespec monitor_interval;

  monitor_interval.tv_sec = MONITOR_INTERVAL;
  monitor_interval.tv_nsec = 0;
  nanosleep(&monitor_interval, NULL);

  TASessionManager_printMonitoredTX(self, "OCTAB bench", PAGESIZE);
}

static void OCTABBench_afterTeardown(TASessionManager self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  TATXStat summary_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "OCTAB bench", TASession_MEASUREMENT, TASession_TX);
  struct timeval end_timeval;
  char end_time_str[24] = "0000-00-00 00:00:00.000";
  static char *tx_names[1] = {"OCTAB bench"};

  TASessionManager_printMonitoredTX(self, "OCTAB bench", PAGESIZE);
  TADistribution_print(TATXStat_distribution(summary_stat));
  TASessionManager_printNumericalQuantitiesSummary(self, tx_names, 1);

  timerclear(&end_timeval);
  gettimeofday(&end_timeval, (struct timezone *)0);
  timeval2str(end_time_str, end_timeval);
  printf("----------------------------------------------------------------\n");
  printf("               End Time : %s\n", end_time_str);
  printf("----------------------------------------------------------------\n");
}

int OCTABBench_main(const OCTAOption *opt)
{
  TASession session_prototype = TASession_init();
  TASessionManager session_manager = NULL;
  OCTABBenchInput *io = NULL;
  int ret = 0;

  io = malloc(sizeof(OCTABBenchInput));
  if (io == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTABBenchInput\n");
    exit(1);
  }
  memset(io, 0, sizeof(*io));
  io->option = *opt;
  io->oracle = NULL;

  TASession_setSetup(session_prototype, OCTABBench_setup);
  TASession_setTX(session_prototype, OCTABBench_TX, "OCTAB bench");
  TASession_setBeforeTX(session_prototype, OCTABBench_beforeTX, "OCTAB bench");
  TASession_setWhenErrorTX(session_prototype, OCTABBench_errorTX,
                           "OCTAB bench");
  TASession_setAfterTX(session_prototype, OCTABBench_afterTX, "OCTAB bench");
  TASession_setSelectTX(session_prototype, OCTABBench_selectTX);
  TASession_setTeardown(session_prototype, OCTABBench_teardown);

  session_manager =
    TASessionManager_initWithSessionPrototype(session_prototype,
                                              opt->num_sessions);
  TASessionManager_setBeforeSetup(session_manager, OCTABBench_beforeSetup);
  TASessionManager_setAfterTeardown(session_manager, OCTABBench_afterTeardown);
  TASessionManager_setMonitor(session_manager, OCTABBench_monitor);

  ret = TASessionManager_main(session_manager, (void **)&io);
  TASession_release(session_prototype);
  free(io);
  return ret;
}
