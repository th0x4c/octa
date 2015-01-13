/*
 * OCTABLoad.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/18/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTABLoad.h"
#include "config.h"

#ifndef HAVE_UNION_SEMUN
union semun {
  int             val;            /* value for SETVAL */
  struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
  unsigned short  *array;         /* array for GETALL & SETALL */
};
#endif

/* All sessions share the following variables */
static OCTAOption option;
static int semid;
static int shmid;
static int total_account;
static int *loaded_account;

/* Each session has its own vairable */
static OCOracle oracle;

#define SEMAPHORE_P                                            \
  {                                                            \
    struct sembuf sops;                                        \
    sops.sem_num = 0;                                          \
    sops.sem_op = -1; /* P */                                  \
    sops.sem_flg = SEM_UNDO;                                   \
    if (semop(semid, &sops, 1) == -1)                          \
    {                                                          \
      fprintf(stderr, "semop failed [%s]\n", strerror(errno)); \
      exit(1);                                                 \
    }                                                          \
  }

#define SEMAPHORE_V                                            \
  {                                                            \
    struct sembuf sops;                                        \
    sops.sem_num = 0;                                          \
    sops.sem_op = 1; /* V */                                   \
    sops.sem_flg = SEM_UNDO;                                   \
    if (semop(semid, &sops, 1) == -1)                          \
    {                                                          \
      fprintf(stderr, "semop failed [%s]\n", strerror(errno)); \
      exit(1);                                                 \
    }                                                          \
  }

static void OCTABLoad_beforeSetup(TASessionManager self, void **inout)
{
  union semun sem_union;

  /* semaphore */
  semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
  sem_union.val = 1;
  if (semctl(semid, 0, SETVAL, sem_union) == -1)
  {
    fprintf(stderr, "semctl failed [%s]\n", strerror(errno));
    exit(1);
  }
  /* shared memory */
  shmid = shmget(IPC_PRIVATE, sizeof(int), 0666 | IPC_CREAT);
  if (shmid < 0)
  {
    fprintf(stderr, "shmget failed [%s]\n", strerror(errno));
    exit(1);
  }
  loaded_account = (int *) shmat(shmid, 0, 0);
  if (loaded_account == (void *)-1)
  {
    fprintf(stderr, "shmat failed [%s]\n", strerror(errno));
    exit(1);
  }
  *loaded_account = 0;
}

static void OCTABLoad_setup(TASession self, void **inout)
{
  oracle = OCOracle_init();
  OCOracle_connect(oracle, option.username, option.password,
                   option.tnsname);
  TASession_setStatus(self, TASession_RUNNING);
}

static void OCTABLoad_beforeTX(TASession self, void **inout)
{
  int *account_id = (int *)*inout;

  SEMAPHORE_P;
  *loaded_account += 1;
  *account_id = *loaded_account;
  SEMAPHORE_V;

  if (*account_id > total_account)
    TASession_setStatus(self, TASession_STOP);
}

static int OCTABLoad_oracleTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize)
{
  int *account_id = (int *)*inout;
  int branch_id = (*account_id - 1) / ACCOUNTS_PER_BRANCH + 1;
  int first_teller_id = (branch_id - 1) * TELLERS_PER_BRANCH + 1;
  int last_teller_id = first_teller_id + TELLERS_PER_BRANCH - 1;
  OCIStmt   *stmthp = (OCIStmt *)0;
  OCIBind   *bnd1p  = (OCIBind *)0;
  OCIBind   *bnd2p  = (OCIBind *)0;
  int errcode = 0;
  static char *insert_branch_sql =
    "INSERT INTO branch (branch_id, branch_balance, filler) "
    "VALUES (:1, 0, '12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678')";
  static char *insert_teller_sql =
    "INSERT INTO teller (teller_id, branch_id, teller_balance, filler) "
    "VALUES (:1, :2, 0,'1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567')";
  static char *insert_account_sql =
    "INSERT INTO account (account_id, branch_id, account_balance, filler) "
    "VALUES (:1, :2, 0, '1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567')";
  static int num_inserts = 0;
  int i = 0;

  if (*account_id > total_account)
    return 0;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp,
                                      OCI_HTYPE_STMT, (size_t) 0,
                                      (dvoid **) 0));
  if (errcode != 0) goto end;

  if (*account_id % ACCOUNTS_PER_BRANCH == 1)
  {
    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtPrepare(stmthp, errhp,
                                        (text *)insert_branch_sql,
                                        (ub4) strlen(insert_branch_sql),
                                        (ub4) OCI_NTV_SYNTAX,
                                        (ub4) OCI_DEFAULT));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                      (dvoid *) &branch_id,
                                      (sword) sizeof(branch_id), SQLT_INT,
                                      (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                      (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
    if (errcode != 0) goto end;

    errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                         OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                        (ub4) 0, (CONST OCISnapshot *) NULL,
                                        (OCISnapshot *) NULL, OCI_DEFAULT));
    if (errcode != 0) goto end;

    for (i = first_teller_id; i <= last_teller_id; i++)
    {
      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCIStmtPrepare(stmthp, errhp,
                                          (text *)insert_teller_sql,
                                          (ub4) strlen(insert_teller_sql),
                                          (ub4) OCI_NTV_SYNTAX,
                                          (ub4) OCI_DEFAULT));
      if (errcode != 0) goto end;

      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                        (dvoid *) &i,
                                        (sword) sizeof(i), SQLT_INT,
                                        (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                        (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
      if (errcode != 0) goto end;

      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                        (dvoid *) &branch_id,
                                        (sword) sizeof(branch_id), SQLT_INT,
                                        (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                        (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
      if (errcode != 0) goto end;

      errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                           OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                          (ub4) 0, (CONST OCISnapshot *) NULL,
                                          (OCISnapshot *) NULL, OCI_DEFAULT));
      if (errcode != 0) goto end;
    }
  }

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtPrepare(stmthp, errhp,
                                      (text *)insert_account_sql,
                                      (ub4) strlen(insert_account_sql),
                                      (ub4) OCI_NTV_SYNTAX,
                                      (ub4) OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd1p, errhp, 1,
                                    (dvoid *) account_id,
                                    (sword) sizeof(*account_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIBindByPos(stmthp, &bnd2p, errhp, 2,
                                    (dvoid *) &branch_id,
                                    (sword) sizeof(branch_id), SQLT_INT,
                                    (dvoid *) 0, (ub2 *) 0, (ub2 *) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT));
  if (errcode != 0) goto end;

  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1,
                                      (ub4) 0, (CONST OCISnapshot *) NULL,
                                      (OCISnapshot *) NULL, OCI_DEFAULT));
  if (errcode != 0) goto end;

  num_inserts += 1;
  if (num_inserts % INSERTS_PER_COMMIT == 0)
    OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  (void) OCIHandleFree((dvoid*) stmthp, OCI_HTYPE_STMT);

  return errcode;
}

static int OCTABLoad_TX(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTABLoad_oracleTX);
}

static void OCTABLoad_errorTX(TASession self, void **inout, int error_code,
                              char *error_message, size_t error_message_size)
{
  snprintf(error_message, error_message_size, "err: %d, msg: %s", 
           error_code, OCOracle_errorMessage(oracle));
}

static char *OCTABLoad_selectTX(TASession self)
{
#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 64
#endif
  static char tx_name[MAX_NAME_SIZE] = "OCTAB load";

  return tx_name;
}

static int OCTABLoad_commitTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize)
{
  OCITransCommit(svchp, errhp, (ub4) 0);
}

static void OCTABLoad_teardown(TASession self, void **inout)
{
  TATXStat stat = TASession_statByNameInPeriodInPhase(self, "OCTAB load",
                                                      TASession_MEASUREMENT,
                                                      TASession_TX);
  TALog log = TASession_log(self);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  OCOracle_execTX(oracle, inout, OCTABLoad_commitTX);
  OCOracle_release(oracle);

  /* shared memory */
  if (shmdt(loaded_account) == -1)
  {
    fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
    exit(1);
  }

  TALog_info(log, TATXStat_description(stat, desc, DESC_SIZE));
}

static void OCTABLoad_monitor(TASessionManager self)
{
#define PROGRESS_SIZE 50
  TATXStat summary = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                       "OCTAB load", TASession_MEASUREMENT, TASession_TX);
  struct timespec monitor_interval;
  int i = 0;

  monitor_interval.tv_sec = 0;
  monitor_interval.tv_nsec = 100000000; /* 100ms */
  nanosleep(&monitor_interval, NULL);

  printf("\rloading... ");

  for (i = 0; i < PROGRESS_SIZE; i++)
  {
    if (i < *loaded_account * PROGRESS_SIZE / total_account)
      printf("#");
    else
      printf(" ");
  }
  printf(" [%3d%%](%d/%d)", *loaded_account * 100 / total_account,
         *loaded_account / ACCOUNTS_PER_BRANCH, option.scale_factor);

  fflush(stdout);
  TATXStat_release(summary);
}

static void OCTABLoad_afterTeardown(TASessionManager self, void **inout)
{
  union semun sem_union;
  TATXStat summary = TASessionManager_summaryStatByNameInPeriodInPhase(self,
                       "OCTAB load", TASession_MEASUREMENT, TASession_TX);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  printf("\nError: %d\n", TATXStat_errorCount(summary));
  if (TATXStat_errorCount(summary) == 0)
    printf("All rows loaded successfully\n");

  printf("%s\n", TATXStat_description(summary, desc, DESC_SIZE));
  printf("\n");

  /* semaphore */
  if (semctl(semid, 0, IPC_RMID, sem_union) == -1)
  {
    fprintf(stderr, "semctl failed [%s]\n", strerror(errno));
    exit(1);
  }
  /* shared memory */
  if (shmdt(loaded_account) == -1)
  {
    fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
    exit(1);
  }
  if (shmctl(shmid, IPC_RMID, 0) == -1)
  {
    fprintf(stderr, "shmctl failed [%s]\n", strerror(errno));
    exit(1);
  }
}

int OCTABLoad_main(const OCTAOption *opt)
{
  TASession session_prototype = TASession_init();
  TASessionManager session_manager = NULL;
  int *account_id = NULL;
  int ret = 0;

  option = *opt;
  total_account = option.scale_factor * ACCOUNTS_PER_BRANCH;

  TASession_setSetup(session_prototype, OCTABLoad_setup);
  TASession_setTX(session_prototype, OCTABLoad_TX, "OCTAB load");
  TASession_setBeforeTX(session_prototype, OCTABLoad_beforeTX, "OCTAB load");
  TASession_setWhenErrorTX(session_prototype, OCTABLoad_errorTX, "OCTAB load");
  TASession_setSelectTX(session_prototype, OCTABLoad_selectTX);
  TASession_setTeardown(session_prototype, OCTABLoad_teardown);
  TALog_setLevel(TASession_log(session_prototype), TALog_WARN);

  session_manager =
    TASessionManager_initWithSessionPrototype(session_prototype,
                                              opt->num_sessions);
  TASessionManager_setBeforeSetup(session_manager, OCTABLoad_beforeSetup);
  TASessionManager_setAfterTeardown(session_manager, OCTABLoad_afterTeardown);
  TASessionManager_setMonitor(session_manager, OCTABLoad_monitor);

  account_id = malloc(sizeof(int));
  if (account_id == NULL)
  {
    fprintf(stdout, "Cannot malloc int\n");
    exit(1);
  }
  *account_id = 0;

  ret = TASessionManager_main(session_manager, (void **)&account_id);
  TASession_release(session_prototype);
  free(account_id);
  return ret;
}
