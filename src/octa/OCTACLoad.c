/*
 * OCTACLoad.c
 * OCTA
 *
 * Created by Takashi Hashizume on 04/01/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACLoad.h"

#ifndef HAVE_UNION_SEMUN
union semun {
  int             val;            /* value for SETVAL */
  struct semid_ds *buf;           /* buffer for IPC_STAT & IPC_SET */
  unsigned short  *array;         /* array for GETALL & SETALL */
};
#endif

struct OCTACLoadCount
{
  long item;
  long warehouse;
  long stock;
  long district;
  long customer;
  long orders;
};
typedef struct OCTACLoadCount OCTACLoadCount;

/* All sessions share the following variables */
static OCTAOption option;
static int semid;
static int shmid;
static OCTACLoadCount total_count;
static OCTACLoadCount *loaded_count;
static long *permutation;

/* Each session has its own vairable */
static OCOracle oracle;
static OCTACLoadCount loading_count;
static long o_c_id;

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

static void OCTACLoad_beforeSetup(TASessionManager self, void **inout)
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
  shmid = shmget(IPC_PRIVATE,
                 sizeof(OCTACLoadCount) + (sizeof(long) * CUST_PER_DIST),
                 0666 | IPC_CREAT);
  if (shmid < 0)
  {
    fprintf(stderr, "shmget failed [%s]\n", strerror(errno));
    exit(1);
  }
  loaded_count = (OCTACLoadCount *) shmat(shmid, 0, 0);
  if (loaded_count == (void *)-1)
  {
    fprintf(stderr, "shmat failed [%s]\n", strerror(errno));
    exit(1);
  }
  loaded_count->item = 0;
  loaded_count->warehouse = 0;
  loaded_count->stock = 0;
  loaded_count->district = 0;
  loaded_count->orders = 0;
  permutation = (long *) (loaded_count + 1);
  OCTACConfig_initPermutation(permutation, CUST_PER_DIST);
}

static void OCTACLoad_setup(TASession self, void **inout)
{
  oracle = OCOracle_init();
  OCOracle_connect(oracle, option.username, option.password,
                   option.tnsname);
  TASession_setStatus(self, TASession_RUNNING);
}

/* Item */
static void OCTACLoad_beforeTXItem(TASession self, void **inout)
{
  long i_id = loading_count.item;

  if (i_id == 1)
  {
    printf("Loading Item\n");
    fflush(stdout);
  }

  OCTACLoadItem_beforeTX(i_id, inout);
}

static int OCTACLoad_TXItem(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadItem_oracleTX);
}

static void OCTACLoad_afterTXItem(TASession self, void **inout)
{
  OCTACLoadItem_afterTX(inout);

  if (loading_count.item == total_count.item)
  {
    printf("Item Done.\n");
    fflush(stdout);
  }
}

/* Warehouse */
static void OCTACLoad_beforeTXWarehouse(TASession self, void **inout)
{
  long w_id = loading_count.warehouse;

  if (w_id == 1)
  {
    printf("Loading Warehouse\n");
    fflush(stdout);
  }

  OCTACLoadWarehouse_beforeTX(w_id, inout);
}

static int OCTACLoad_TXWarehouse(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadWarehouse_oracleTX);
}

static void OCTACLoad_afterTXWarehouse(TASession self, void **inout)
{
  OCTACLoadWarehouse_afterTX(inout);

  if (loading_count.warehouse == total_count.warehouse)
  {
    printf("Warehouse Done.\n");
    fflush(stdout);
  }
}

/* Stock */
static void OCTACLoad_beforeTXStock(TASession self, void **inout)
{
  long s_i_id = ((loading_count.stock - 1) % MAXITEMS) + 1;
  long s_w_id = ((loading_count.stock - 1) / MAXITEMS) + 1;

  if (s_i_id == 1)
  {
    printf("Loading Stock Wid=%ld\n", s_w_id);
    fflush(stdout);
  }

  OCTACLoadStock_beforeTX(s_i_id, s_w_id, inout);
}

static int OCTACLoad_TXStock(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadStock_oracleTX);
}

static void OCTACLoad_afterTXStock(TASession self, void **inout)
{
  OCTACLoadStock_afterTX(inout);

  if (loading_count.stock == total_count.stock)
  {
    printf("Stock Done.\n");
    fflush(stdout);
  }
}

/* District */
static void OCTACLoad_beforeTXDistrict(TASession self, void **inout)
{
  long d_id = ((loading_count.district - 1) % DIST_PER_WARE) + 1;
  long d_w_id = ((loading_count.district - 1) / DIST_PER_WARE) + 1;

  if (loading_count.district == 1)
  {
    printf("Loading District\n");
    fflush(stdout);
  }

  OCTACLoadDistrict_beforeTX(d_id, d_w_id, inout);
}

static int OCTACLoad_TXDistrict(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadDistrict_oracleTX);
}

static void OCTACLoad_afterTXDistrict(TASession self, void **inout)
{
  OCTACLoadDistrict_afterTX(inout);

  if (loading_count.district == total_count.district)
  {
    printf("District Done.\n");
    fflush(stdout);
  }
}

/* Customer */
static void OCTACLoad_beforeTXCustomer(TASession self, void **inout)
{
  long c_id = ((loading_count.customer - 1) % CUST_PER_DIST) + 1;
  long c_d_id = ((loading_count.customer - 1) / CUST_PER_DIST) %
                DIST_PER_WARE + 1;
  long c_w_id = ((loading_count.customer - 1) /
                 (DIST_PER_WARE * CUST_PER_DIST)) + 1;

  if (c_id == 1)
  {
    printf("Loading Customer for DID=%ld, WID=%ld\n", c_d_id, c_w_id);
    fflush(stdout);
  }

  OCTACLoadCustomer_beforeTX(c_id, c_d_id, c_w_id, inout);
}

static int OCTACLoad_TXCustomer(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadCustomer_oracleTX);
}

static void OCTACLoad_afterTXCustomer(TASession self, void **inout)
{
  OCTACLoadCustomer_afterTX(inout);

  if (loading_count.customer == total_count.customer)
  {
    printf("Customer Done.\n");
    fflush(stdout);
  }
}

/* Orders */
static void OCTACLoad_beforeTXOrders(TASession self, void **inout)
{
  long o_id = ((loading_count.orders - 1) % ORD_PER_DIST) + 1;
  long o_d_id = ((loading_count.orders - 1) / ORD_PER_DIST) % DIST_PER_WARE +
                1;
  long o_w_id = ((loading_count.orders - 1) / (DIST_PER_WARE * ORD_PER_DIST)) +
                1;

  if (o_id == 1)
  {
    printf("Loading Orders for D=%ld, W=%ld\n", o_d_id, o_w_id);
    fflush(stdout);
  }

  OCTACLoadOrders_beforeTX(o_id, o_d_id, o_w_id, o_c_id, inout);
}

static int OCTACLoad_TXOrders(TASession self, void **inout)
{
  return OCOracle_execTX(oracle, inout, OCTACLoadOrders_oracleTX);
}

static void OCTACLoad_afterTXOrders(TASession self, void **inout)
{
  OCTACLoadOrders_afterTX(inout);

  if (loading_count.orders == total_count.orders)
  {
    printf("Orders Done.\n");
    fflush(stdout);
  }
}

static int OCTACLoad_commitTX(OCIEnv *envhp, OCIError *errhp, OCISvcCtx *svchp,
                              void **inout, char *errmsg, size_t errmsgsize)
{
  OCITransCommit(svchp, errhp, (ub4) 0);
}

static int OCTACLoad_rollbackTX(OCIEnv *envhp, OCIError *errhp,
                                OCISvcCtx *svchp, void **inout, char *errmsg,
                                size_t errmsgsize)
{
  OCITransRollback(svchp, errhp, OCI_DEFAULT);
}

static void OCTACLoad_errorTX(TASession self, void **inout, int error_code,
                              char *error_message, size_t error_message_size)
{
  OCOracle_execTX(oracle, inout, OCTACLoad_rollbackTX);
  snprintf(error_message, error_message_size, "err: %d, msg: %s",
           error_code, OCOracle_errorMessage(oracle));
}

static char *OCTACLoad_selectTX(TASession self)
{
#ifndef MAX_NAME_SIZE
#define MAX_NAME_SIZE 64
#endif
  static char tx_name[6][MAX_NAME_SIZE] =
                {"OCTAC load item", "OCTAC load warehouse", "OCTAC load stock",
                 "OCTAC load district", "OCTAC load customer",
                 "OCTAC load orders"};
  char *ret = "OCTAC load item";

  SEMAPHORE_P;
  if (loaded_count->item < total_count.item)
  {
    loaded_count->item += 1;
    loading_count.item = loaded_count->item;
    ret = tx_name[0];
  }
  else if (loaded_count->warehouse < total_count.warehouse)
  {
    loaded_count->warehouse += 1;
    loading_count.warehouse = loaded_count->warehouse;
    ret = tx_name[1];
  }
  else if (loaded_count->stock < total_count.stock)
  {
    loaded_count->stock += 1;
    loading_count.stock = loaded_count->stock;
    ret = tx_name[2];
  }
  else if (loaded_count->district < total_count.district)
  {
    loaded_count->district += 1;
    loading_count.district = loaded_count->district;
    ret = tx_name[3];
  }
  else if (loaded_count->customer < total_count.customer)
  {
    loaded_count->customer += 1;
    loading_count.customer = loaded_count->customer;
    ret = tx_name[4];
  }
  else if (loaded_count->orders < total_count.orders)
  {
    loaded_count->orders += 1;
    loading_count.orders = loaded_count->orders;
    if (loading_count.orders % ORD_PER_DIST == 1)
      OCTACConfig_initPermutation(permutation, CUST_PER_DIST);

    o_c_id = permutation[(loading_count.orders - 1) % ORD_PER_DIST];
    ret = tx_name[5];
  }
  else
  {
    TASession_setStatus(self, TASession_STOP);
  }
  SEMAPHORE_V;

  return ret;
}

static void OCTACLoad_teardown(TASession self, void **inout)
{
  OCOracle_execTX(oracle, inout, OCTACLoad_commitTX);
  OCOracle_release(oracle);

  /* shared memory */
  if (shmdt(loaded_count) == -1)
  {
    fprintf(stderr, "shmdt failed [%s]\n", strerror(errno));
    exit(1);
  }
}

static void OCTACLoad_afterTeardown(TASessionManager self, void **inout)
{
  union semun sem_union;
  TATXStat summary_item =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load item", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_warehouse =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load warehouse", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_stock =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load stock", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_district =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load district", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_customer =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load customer", TASession_MEASUREMENT, TASession_TX);
  TATXStat summary_orders =
    TASessionManager_summaryStatByNameInPeriodInPhase(self,
      "OCTAC load orders", TASession_MEASUREMENT, TASession_TX);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";
  unsigned int error_count = 0;

  error_count = TATXStat_errorCount(summary_item)      +
                TATXStat_errorCount(summary_warehouse) +
                TATXStat_errorCount(summary_stock)     +
                TATXStat_errorCount(summary_district)  +
                TATXStat_errorCount(summary_customer)  +
                TATXStat_errorCount(summary_orders);

  printf("\nError: %d\n", error_count);

  if (error_count == 0)
    printf("...DATA LOADING COMPLETED SUCCESSFULLY.\n");

  printf("%s\n", TATXStat_description(summary_item, desc, DESC_SIZE));
  printf("%s\n", TATXStat_description(summary_warehouse, desc, DESC_SIZE));
  printf("%s\n", TATXStat_description(summary_stock, desc, DESC_SIZE));
  printf("%s\n", TATXStat_description(summary_district, desc, DESC_SIZE));
  printf("%s\n", TATXStat_description(summary_customer, desc, DESC_SIZE));
  printf("%s\n", TATXStat_description(summary_orders, desc, DESC_SIZE));
  printf("\n");

  TATXStat_release(summary_item);
  TATXStat_release(summary_warehouse);
  TATXStat_release(summary_stock);
  TATXStat_release(summary_district);
  TATXStat_release(summary_customer);
  TATXStat_release(summary_orders);

  /* semaphore */
  if (semctl(semid, 0, IPC_RMID, sem_union) == -1)
  {
    fprintf(stderr, "semctl failed [%s]\n", strerror(errno));
    exit(1);
  }
  /* shared memory */
  if (shmdt(loaded_count) == -1)
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

int OCTACLoad_main(const OCTAOption *opt)
{
  TASession session_prototype = TASession_init();
  TASessionManager session_manager = NULL;
  void *inout = NULL;
  int ret = 0;

  option = *opt;
  total_count.item = MAXITEMS;
  total_count.warehouse = option.scale_factor;
  total_count.stock = total_count.warehouse * MAXITEMS;
  total_count.district = total_count.warehouse * DIST_PER_WARE;
  total_count.customer = total_count.warehouse * DIST_PER_WARE * CUST_PER_DIST;
  total_count.orders = total_count.warehouse * DIST_PER_WARE * ORD_PER_DIST;

  TASession_setSetup(session_prototype, OCTACLoad_setup);
  /* Item */
  TASession_setTX(session_prototype, OCTACLoad_TXItem, "OCTAC load item");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXItem,
                        "OCTAC load item");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load item");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXItem,
                        "OCTAC load item");
  /* Warehouse */
  TASession_setTX(session_prototype, OCTACLoad_TXWarehouse,
                  "OCTAC load warehouse");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXWarehouse,
                        "OCTAC load warehouse");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load warehouse");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXWarehouse,
                        "OCTAC load warehouse");
  /* Stock */
  TASession_setTX(session_prototype, OCTACLoad_TXStock,
                  "OCTAC load stock");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXStock,
                        "OCTAC load stock");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load stock");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXStock,
                        "OCTAC load stock");
  /* District */
  TASession_setTX(session_prototype, OCTACLoad_TXDistrict,
                  "OCTAC load district");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXDistrict,
                        "OCTAC load district");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load district");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXDistrict,
                        "OCTAC load district");
  /* Customer */
  TASession_setTX(session_prototype, OCTACLoad_TXCustomer,
                  "OCTAC load customer");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXCustomer,
                        "OCTAC load customer");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load customer");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXCustomer,
                        "OCTAC load customer");
  /* Orders */
  TASession_setTX(session_prototype, OCTACLoad_TXOrders,
                  "OCTAC load orders");
  TASession_setBeforeTX(session_prototype, OCTACLoad_beforeTXOrders,
                        "OCTAC load orders");
  TASession_setWhenErrorTX(session_prototype, OCTACLoad_errorTX,
                           "OCTAC load orders");
  TASession_setAfterTX(session_prototype, OCTACLoad_afterTXOrders,
                        "OCTAC load orders");

  TASession_setSelectTX(session_prototype, OCTACLoad_selectTX);
  TASession_setTeardown(session_prototype, OCTACLoad_teardown);
  TALog_setLevel(TASession_log(session_prototype), TALog_WARN);

  session_manager =
    TASessionManager_initWithSessionPrototype(session_prototype,
                                              opt->num_sessions);
  TASessionManager_setBeforeSetup(session_manager, OCTACLoad_beforeSetup);
  TASessionManager_setAfterTeardown(session_manager, OCTACLoad_afterTeardown);

  ret = TASessionManager_main(session_manager, (void **)&inout);
  TASession_release(session_prototype);
  return ret;
}
