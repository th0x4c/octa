/*
 * OCTACBench.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/05/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBench.h"

struct OCTACBenchInput
{
  OCTAOption option;
  OCOracle oracle;
  int session_id;
  OCTACBenchNewOrderInput input_new_order;
  OCTACBenchPaymentInput input_payment;
  OCTACBenchOrderStatusInput input_order_status;
  /* OCTACBenchDeliveryInput input_delivery; */
  /* OCTACBenchStockLevel input_stock_level; */
};
typedef struct OCTACBenchInput OCTACBenchInput;

static int tx_percentage[TXS];

static void OCTACBench_beforeSetup(TASessionManager self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  OCTAOption option = io->option;
  struct timeval start_time;
  char start_time_str[24] = "0000-00-00 00:00:00.000";
  int i = 0;

  for (i = 0; i < TXS; i++)
  {
    tx_percentage[i] = option.tx_percentage[i];
  }

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

static void OCTACBench_setup(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  TALog log = TALog_initWithFilename(LOGFILE);

  TASession_setLog(self, log);
  io->session_id = TASession_ID(self);
  io->oracle = OCOracle_init();
  OCOracle_connect(io->oracle, io->option.username, io->option.password,
                   io->option.tnsname);
  TASession_setPeriod(self, TASession_RAMPUP);
  TASession_setStatus(self, TASession_RUNNING);
}

/* New-Order */
static void OCTACBench_beforeTXNewOrder(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchNewOrder_beforeTX(&(io->input_new_order), TASession_ID(self),
                              io->option.scale_factor,
                              io->option.keying_time[IDX_NEW_ORDER]);
}

static int OCTACBench_TXNewOrder(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  OCTACBenchNewOrderInput *in = &(io->input_new_order);

  return OCOracle_execTX(io->oracle, (void **) &in,
                         OCTACBenchNewOrder_oracleTX);
}

static void OCTACBench_afterTXNewOrder(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchNewOrder_afterTX(io->option.think_time[IDX_NEW_ORDER]);
}

/* Payment */
static void OCTACBench_beforeTXPayment(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchPayment_beforeTX(&(io->input_payment), TASession_ID(self),
                             io->option.scale_factor,
                             io->option.keying_time[IDX_PAYMENT]);
}

static int OCTACBench_TXPayment(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  OCTACBenchPaymentInput *in = &(io->input_payment);

  return OCOracle_execTX(io->oracle, (void **) &in,
                         OCTACBenchPayment_oracleTX);
}

static void OCTACBench_afterTXPayment(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchPayment_afterTX(io->option.think_time[IDX_PAYMENT]);
}

/* Order-Status */
static void OCTACBench_beforeTXOrderStatus(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchOrderStatus_beforeTX(&(io->input_order_status), TASession_ID(self),
                                 io->option.scale_factor,
                                 io->option.keying_time[IDX_ORDER_STATUS]);
}

static int OCTACBench_TXOrderStatus(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  OCTACBenchOrderStatusInput *in = &(io->input_order_status);

  return OCOracle_execTX(io->oracle, (void **) &in,
                         OCTACBenchOrderStatus_oracleTX);
}

static void OCTACBench_afterTXOrderStatus(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCTACBenchOrderStatus_afterTX(io->option.think_time[IDX_ORDER_STATUS]);
}

/* Delivery */
/* static void OCTACBench_beforeTXDelivery(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */

/*   OCTACBenchDelivery_beforeTX(&(io->input_delivery), TASession_ID(self), */
/*                               io->option.scale_factor, */
/*                               io->option.keying_time[IDX_DELIVERY]); */
/* } */

/* static int OCTACBench_TXDelivery(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */
/*   OCTACBenchDeliveryInput *in = &(io->input_delivery); */

/*   return OCOracle_execTX(io->oracle, (void **) &in, */
/*                          OCTACBenchDelivery_oracleTX); */
/* } */

/* static void OCTACBench_afterTXDelivery(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */

/*   OCTACBenchDelivery_afterTX(io->option.think_time[IDX_DELIVERY]); */
/* } */

/* Stock-Level */
/* static void OCTACBench_beforeTXStockLevel(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */

/*   OCTACBenchStockLevel_beforeTX(&(io->input_stock_level), TASession_ID(self), */
/*                                 io->option.scale_factor, */
/*                                 io->option.keying_time[IDX_STOCK_LEVEL]); */
/* } */

/* static int OCTACBench_TXStockLevel(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */
/*   OCTACBenchStockLevelInput *in = &(io->input_stock_level); */

/*   return OCOracle_execTX(io->oracle, (void **) &in, */
/*                          OCTACBenchStockLevel_oracleTX); */
/* } */

/* static void OCTACBench_afterTXStockLevel(TASession self, void **inout) */
/* { */
/*   OCTACBenchInput *io = (OCTACBenchInput *)*inout; */

/*   OCTACBenchStockLevel_afterTX(io->option.think_time[IDX_STOCK_LEVEL]); */
/* } */

static int OCTACBench_rollbackTX(OCIEnv *envhp, OCIError *errhp,
                                 OCISvcCtx *svchp, void **inout, char *errmsg,
                                 size_t errmsgsize)
{
  OCITransRollback(svchp, errhp, OCI_DEFAULT);
}

static void OCTACBench_errorTX(TASession self, void **inout, int error_code,
                               char *error_message, size_t error_message_size)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;

  OCOracle_execTX(io->oracle, inout, OCTACBench_rollbackTX);
  snprintf(error_message, error_message_size, "err: %d, msg: %s", 
           error_code, OCOracle_errorMessage(io->oracle));
}

static char *OCTACBench_selectTX(TASession self)
{
  static char *tx_names[TXS] =
                {"New-Order", "Payment", "Order-Status", "Delivery",
                 "Stock-Level"};

  /* return tx_names[TARandom_indexInRatio(tx_percentage, TXS)]; */
  return tx_names[TARandom_indexInRatio(tx_percentage, 3)];
}

static void OCTACBench_teardown(TASession self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  TATXStat stat = TASession_statByNameInPeriodInPhase(self, "New-Order",
                                                      TASession_MEASUREMENT,
                                                      TASession_TX);
  TALog log = TASession_log(self);
#define DESC_SIZE 512
  char desc[DESC_SIZE] = "";

  OCOracle_release(io->oracle);
  TALog_info(log, TATXStat_description(stat, desc, DESC_SIZE));
}

static void OCTACBench_monitor(TASessionManager self)
{
#define MONITOR_INTERVAL 4
#define PAGESIZE 20
  struct timespec monitor_interval;

  monitor_interval.tv_sec = MONITOR_INTERVAL;
  monitor_interval.tv_nsec = 0;
  nanosleep(&monitor_interval, NULL);

  TASessionManager_printMonitoredTX(self, "New-Order", PAGESIZE);
}

static void OCTACBench_afterTeardown(TASessionManager self, void **inout)
{
  OCTACBenchInput *io = (OCTACBenchInput *)*inout;
  TATXStat summary_stat =
             TASessionManager_summaryStatByNameInPeriodInPhase(self,
               "New-Order", TASession_MEASUREMENT, TASession_TX);
  struct timeval end_timeval;
  char end_time_str[24] = "0000-00-00 00:00:00.000";
  static char *tx_names[TXS] =
                {"New-Order", "Payment", "Order-Status", "Delivery",
                 "Stock-Level"};

  TASessionManager_printMonitoredTX(self, "New-Order", PAGESIZE);
  TADistribution_print(TATXStat_distribution(summary_stat));
  /* TASessionManager_printNumericalQuantitiesSummary(self, tx_names, TXS); */
  TASessionManager_printNumericalQuantitiesSummary(self, tx_names, 3);

  timerclear(&end_timeval);
  gettimeofday(&end_timeval, (struct timezone *)0);
  timeval2str(end_time_str, end_timeval);
  printf("----------------------------------------------------------------\n");
  printf("               End Time : %s\n", end_time_str);
  printf("----------------------------------------------------------------\n");
}

int OCTACBench_main(const OCTAOption *opt)
{
  TASession session_prototype = TASession_init();
  TASessionManager session_manager = NULL;
  OCTACBenchInput *io = NULL;
  int ret = 0;

  io = malloc(sizeof(OCTACBenchInput));
  if (io == NULL)
  {
    fprintf(stdout, "Cannot malloc OCTACBenchInput\n");
    exit(1);
  }
  memset(io, 0, sizeof(*io));
  io->option = *opt;
  io->oracle = NULL;

  TASession_setSetup(session_prototype, OCTACBench_setup);

  /* New-Order */
  TASession_setTX(session_prototype, OCTACBench_TXNewOrder, "New-Order");
  TASession_setBeforeTX(session_prototype, OCTACBench_beforeTXNewOrder,
                        "New-Order");
  TASession_setWhenErrorTX(session_prototype, OCTACBench_errorTX,
                           "New-Order");
  TASession_setAfterTX(session_prototype, OCTACBench_afterTXNewOrder,
                       "New-Order");

  /* Payment */
  TASession_setTX(session_prototype, OCTACBench_TXPayment, "Payment");
  TASession_setBeforeTX(session_prototype, OCTACBench_beforeTXPayment,
                        "Payment");
  TASession_setWhenErrorTX(session_prototype, OCTACBench_errorTX,
                           "Payment");
  TASession_setAfterTX(session_prototype, OCTACBench_afterTXPayment,
                       "Payment");

  /* Order-Status */
  TASession_setTX(session_prototype, OCTACBench_TXOrderStatus, "Order-Status");
  TASession_setBeforeTX(session_prototype, OCTACBench_beforeTXOrderStatus,
                        "Order-Status");
  TASession_setWhenErrorTX(session_prototype, OCTACBench_errorTX,
                           "Order-Status");
  TASession_setAfterTX(session_prototype, OCTACBench_afterTXOrderStatus,
                       "Order-Status");

  /* Delivery */
  /* TASession_setTX(session_prototype, OCTACBench_TXDelivery, "Delivery"); */
  /* TASession_setBeforeTX(session_prototype, OCTACBench_beforeTXDelivery, */
  /*                       "Delivery"); */
  /* TASession_setWhenErrorTX(session_prototype, OCTACBench_errorTX, */
  /*                          "Delivery"); */
  /* TASession_setAfterTX(session_prototype, OCTACBench_afterTXDelivery, */
  /*                      "Delivery"); */

  /* Stock-Level */
  /* TASession_setTX(session_prototype, OCTACBench_TXStockLevel, "Stock-Level"); */
  /* TASession_setBeforeTX(session_prototype, OCTACBench_beforeTXStockLevel, */
  /*                       "Stock-Level"); */
  /* TASession_setWhenErrorTX(session_prototype, OCTACBench_errorTX, */
  /*                          "Stock-Level"); */
  /* TASession_setAfterTX(session_prototype, OCTACBench_afterTXStockLevel, */
  /*                      "Stock-Level"); */

  TASession_setSelectTX(session_prototype, OCTACBench_selectTX);
  TASession_setTeardown(session_prototype, OCTACBench_teardown);

  session_manager =
    TASessionManager_initWithSessionPrototype(session_prototype,
                                              opt->num_sessions);
  TASessionManager_setBeforeSetup(session_manager, OCTACBench_beforeSetup);
  TASessionManager_setAfterTeardown(session_manager, OCTACBench_afterTeardown);
  TASessionManager_setMonitor(session_manager, OCTACBench_monitor);

  ret = TASessionManager_main(session_manager, (void **)&io);
  TASession_release(session_prototype);
  free(io);
  return ret;
}
