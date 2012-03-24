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
  struct timeval start_time;
  struct timeval measurement_start_time;
  struct timeval rampdown_start_time;
  struct timeval stop_time;
  int session_id;
  int amount;
  int branch_id;
  int teller_id;
  int account_id;
};
typedef struct OCTABBenchInput OCTABBenchInput;

static void OCTABBench_setPeriod(TASession session, OCTABBenchInput *input)
{
  struct timeval current_time;

  timerclear(&current_time);
  gettimeofday(&current_time, (struct timezone *)0);
  switch (TASession_period(session))
  {
  case TASession_RAMPUP:
    if (timercmp(&(input->measurement_start_time), &current_time, <))
      TASession_setPeriod(session, TASession_MEASUREMENT);
  case TASession_MEASUREMENT:
    if (timercmp(&(input->rampdown_start_time), &current_time, <))
      TASession_setPeriod(session, TASession_RAMPDOWN);
  case TASession_RAMPDOWN:
    if (timercmp(&(input->stop_time), &current_time, <))
      TASession_setStatus(session, TASession_STOP);
    break;
  default:
    break;
  }
}

static void OCTABBench_beforeSetup(TASessionManager self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  OCTAOption option = io->option;
  char start_time[24] = "0000-00-00 00:00:00.000";

  gettimeofday(&(io->start_time), (struct timezone *)0);
  timeval2str(start_time, io->start_time);

  timeradd(&(io->start_time), &(option.rampup_time),
           &(io->measurement_start_time));
  timeradd(&(io->measurement_start_time), &(option.measurement_interval),
           &(io->rampdown_start_time));
  timeradd(&(io->rampdown_start_time), &(option.rampdown_time),
           &(io->stop_time));

  printf("----------------------------------------------------------------\n");
  printf("        OCTA (OCI Transaction Application) %s\n", VERSION);
  printf("----------------------------------------------------------------\n");
  printf("              Database username : %s\n", option.username);
  printf("              Database password : %s\n", option.password);
  printf("  Connect identifier (tnsnames) : %s\n", option.tnsname);
  printf("             Number of sessions : %d\n", option.num_sessions);
  printf("                   Scale factor : %d\n", option.scale_factor);
  printf("  Measurement interval (in sec) : %9.3f\n",
         timeval2sec(option.measurement_interval));
  printf("          Ramp-up time (in sec) : %9.3f\n",
         timeval2sec(option.rampup_time));
  printf("        Ramp-down time (in sec) : %9.3f\n",
         timeval2sec(option.rampdown_time));
  printf("            Think time (in sec) : %9.3f\n",
         timeval2sec(option.think_time));
  printf("----------------------------------------------------------------\n");
  printf("              Start Time : %s\n", start_time);
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

  OCTABBench_setPeriod(self, io);
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
  OCIStmt   *stmthp = (OCIStmt *)0;
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

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp,
                                      OCI_HTYPE_STMT, (size_t) 0,
                                      (dvoid **) 0));
  if (errcode != 0) goto end;

  /* UPDATE account */
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)update_account_sql,
                                      (ub4) strlen(update_account_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* SELECT account */
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)select_account_sql,
                                      (ub4) strlen(select_account_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIDefineByPos(stmthp, &defnp, errhp, 1,
                                      (dvoid *) &account_balance,
                                      (sword) sizeof(account_balance),
                                      SQLT_INT, (dvoid *) 0, (ub2 *) 0,
                                      (ub2 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  while (OCOCIERROR(errhp, errmsg, errmsgsize,
                    OCIStmtFetch2(stmthp, errhp, (ub4) 1, OCI_FETCH_NEXT,
                                  (sb4) 0, OCI_DEFAULT)) != OCI_NO_DATA)
  {}

  /* UPDATE teller */
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)update_teller_sql,
                                      (ub4) strlen(update_teller_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->teller_id),
                                    (sword) sizeof(io->teller_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* UPDATE branch */
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)update_branch_sql,
                                      (ub4) strlen(update_branch_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->branch_id),
                                    (sword) sizeof(io->branch_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  /* INSERT history */
  snprintf(info, sizeof(info), "%5d 12345678901", io->session_id);

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)insert_history_sql,
                                      (ub4) strlen(insert_history_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) &(io->teller_id),
                                    (sword) sizeof(io->teller_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                    (dvoid *) &(io->branch_id),
                                    (sword) sizeof(io->branch_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd3p, errhp, 3,
                                    (dvoid *) &(io->account_id),
                                    (sword) sizeof(io->account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd4p, errhp, 4,
                                    (dvoid *) &(io->amount),
                                    (sword) sizeof(io->amount), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd5p, errhp, 5,
                                    (dvoid *) info,
                                    (sword) strlen(info) + 1, SQLT_STR,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  (void) OCIHandleFree((dvoid*) stmthp, OCI_HTYPE_STMT);

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

  think_time.tv_sec = io->option.think_time.tv_sec;
  think_time.tv_nsec = io->option.think_time.tv_usec * 1000;
  nanosleep(&think_time, NULL);
  OCTABBench_setPeriod(self, io);
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
#define HEADER_PER_LINE 20
  static int monitor_count = 0;
  static TATXStat pre_summary_rampup = NULL;
  static TATXStat pre_summary_measurement = NULL;
  static TATXStat pre_summary_rampdown = NULL;
  TATXStat summary_rampup, summary_measurement, summary_rampdown;
  TATXStat diff_rampup, diff_measurement, diff_rampdown;
  struct timeval current_time;
  char current_time_str[24] = "0000-00-00 00:00:00.000";
  char short_time_str[15];
  struct timeval avg;
  struct timespec monitor_interval;

  monitor_interval.tv_sec = MONITOR_INTERVAL;
  monitor_interval.tv_nsec = 0;
  nanosleep(&monitor_interval, NULL);

  if (pre_summary_rampup == NULL)
    pre_summary_rampup = TATXStat_init();
  if (pre_summary_measurement == NULL)
    pre_summary_measurement = TATXStat_init();
  if (pre_summary_rampdown == NULL)
    pre_summary_rampdown = TATXStat_init();

  timerclear(&avg);
  timerclear(&current_time);
  gettimeofday(&current_time, (struct timezone *)0);
  timeval2str(current_time_str, current_time);
  sprintf(short_time_str, "%.*s", 14, current_time_str + 5);

  summary_rampup = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                     "OCTAB bench", TASession_RAMPUP, TASession_TX);
  summary_measurement = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                          "OCTAB bench", TASession_MEASUREMENT, TASession_TX);
  summary_rampdown = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                       "OCTAB bench", TASession_RAMPDOWN, TASession_TX);

  diff_rampup = TATXStat_minus(summary_rampup, pre_summary_rampup);
  diff_measurement = TATXStat_minus(summary_measurement,
                                    pre_summary_measurement);
  diff_rampdown = TATXStat_minus(summary_rampdown, pre_summary_rampdown);

  if ((monitor_count % HEADER_PER_LINE) == 0)
  {
    printf("Time           Period      Count    Error    AVG      TPS\n");
    printf("-------------- ----------- -------- -------- -------- --------\n");
  }
  monitor_count++;

  if (TATXStat_count(diff_rampup) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_rampup);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Ramp-up",
           TATXStat_count(diff_rampup),
           TATXStat_errorCount(diff_rampup),
           timeval2sec(avg), TATXStat_tps(diff_rampup));
  }
  if (TATXStat_count(diff_measurement) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_measurement);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Measurement",
           TATXStat_count(diff_measurement),
           TATXStat_errorCount(diff_measurement),
           timeval2sec(avg), TATXStat_tps(diff_measurement));
  }
  if (TATXStat_count(diff_rampdown) > 0)
  {
    avg = TATXStat_avgElapsedTime(diff_rampdown);
    printf("%s %-11s %8d %8d %8.6f %8.3f\n", short_time_str, "Ramp-down",
           TATXStat_count(diff_rampdown),
           TATXStat_errorCount(diff_rampdown),
           timeval2sec(avg), TATXStat_tps(diff_rampdown));
  }
  fflush(stdout);

  TATXStat_release(pre_summary_rampup);
  TATXStat_release(pre_summary_measurement);
  TATXStat_release(pre_summary_rampdown);
  TATXStat_release(diff_rampup);
  TATXStat_release(diff_measurement);
  TATXStat_release(diff_rampdown);

  pre_summary_rampup = summary_rampup;
  pre_summary_measurement = summary_measurement;
  pre_summary_rampdown = summary_rampdown;
}

static void OCTABBench_afterTeardown(TASessionManager self, void **inout)
{
  OCTABBenchInput *io = (OCTABBenchInput *)*inout;
  TASession *sessions = TASessionManager_sessions(self);
  TATXStat summary_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "OCTAB bench", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_before_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "OCTAB bench", TASession_MEASUREMENT, TASession_BEFORE);
  TATXStat summary_after_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "OCTAB bench", TASession_MEASUREMENT, TASession_AFTER);
  TATXStat summary_rampup_before_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "OCTAB bench", TASession_RAMPUP, TASession_BEFORE);
  struct timeval min, avg, max;
  struct timeval min2, avg2, max2;
  struct timeval rampup_first_time, rampup_time;
  struct timeval first_time, end_time, measurement_interval;
  struct timeval end_timeval;
  char end_time_str[24] = "0000-00-00 00:00:00.000";

  timerclear(&min);
  timerclear(&avg);
  timerclear(&max);
  timerclear(&min2);
  timerclear(&avg2);
  timerclear(&max2);
  min = TATXStat_minElapsedTime(summary_stat);
  avg = TATXStat_avgElapsedTime(summary_stat);
  max = TATXStat_maxElapsedTime(summary_stat);

  OCTABBench_monitor(self);

  printf("================================================================\n");
  printf("================= Numerical Quantities Summary =================\n");
  printf("================================================================\n");
  printf("MQTh, computed Maximum Qualified Throughput %15.2f tpmB\n",
         TATXStat_tps(summary_stat) * 60);
  printf("\n");
  printf("Response Times (minimum/ Average/ maximum) in seconds\n");
  printf("  - %-29s %7.6f / %7.6f / %7.6f\n",
         "", timeval2sec(min), timeval2sec(avg), timeval2sec(max));
  printf("\n");
  printf("Keying/Think Times (in seconds),\n");
  printf("                         Min.          Average           Max.\n");
  min = TATXStat_minElapsedTime(summary_before_stat);
  avg = TATXStat_avgElapsedTime(summary_before_stat);
  max = TATXStat_maxElapsedTime(summary_before_stat);
  min2 = TATXStat_minElapsedTime(summary_after_stat);
  avg2 = TATXStat_avgElapsedTime(summary_after_stat);
  max2 = TATXStat_maxElapsedTime(summary_after_stat);
  printf("  - %-12s %7.3f/%7.3f %7.3f/%7.3f %7.3f/%7.3f\n",
         "", timeval2sec(min), timeval2sec(min2),
         timeval2sec(avg), timeval2sec(avg2),
         timeval2sec(max), timeval2sec(max2));
  printf("\n");
  timerclear(&rampup_time);
  timerclear(&rampup_first_time);
  timerclear(&first_time);
  timerclear(&end_time);
  timerclear(&measurement_interval);
  first_time = TATXStat_firstTime(summary_stat);
  if (TATXStat_count(summary_rampup_before_stat) > 0)
  {
    rampup_first_time = TATXStat_firstTime(summary_rampup_before_stat);
    timersub(&first_time, &rampup_first_time, &rampup_time);
  }
  printf("Test Duration\n");
  printf("  - Ramp-up time %39.3f seconds\n", timeval2sec(rampup_time));
  end_time = TATXStat_endTime(summary_stat);
  timersub(&end_time, &first_time, &measurement_interval);
  printf("  - Measurement interval %31.3f seconds\n",
         timeval2sec(measurement_interval));
  printf("  - Number of transactions (all types)\n");
  printf("    completed in measurement interval %26d\n",
         TATXStat_count(summary_stat));
  printf("================================================================\n");

  timerclear(&end_timeval);
  gettimeofday(&end_timeval, (struct timezone *)0);
  timeval2str(end_time_str, end_timeval);

  printf("----------------------------------------------------------------\n");
  printf("               End Time : %s\n", end_time_str);
  printf("----------------------------------------------------------------\n");

  TATXStat_release(summary_stat);
  TATXStat_release(summary_before_stat);
  TATXStat_release(summary_after_stat);
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
  timerclear(&(io->start_time));
  timerclear(&(io->measurement_start_time));
  timerclear(&(io->rampdown_start_time));
  timerclear(&(io->stop_time));

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
