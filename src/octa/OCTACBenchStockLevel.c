/*
 * OCTACBenchStockLevel.c
 * OCTA
 *
 * Created by Takashi Hashizume on 05/19/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTACBenchStockLevel.h"

void OCTACBenchStockLevel_beforeTX(OCTACBenchStockLevelInput *input,
                                   int session_id, long scale_factor,
                                   struct timeval keying_time)
{
  long w_id = OCTACConfig_homeWID(scale_factor, session_id);
  long d_id = (session_id - 1) / scale_factor + 1;

  snprintf(input->w_id, sizeof(input->w_id), "%ld", w_id);
  snprintf(input->d_id, sizeof(input->d_id), "%ld", d_id);
  snprintf(input->threshold, sizeof(input->threshold), "%ld",
           TARandom_number(10, 20));

  OCTACConfig_sleepKeyingTime(keying_time);
}

int OCTACBenchStockLevel_oracleTX(OCIEnv *envhp, OCIError *errhp,
                                  OCISvcCtx *svchp, void **inout,
                                  char *errmsg, size_t errmsgsize)
{
  OCTACBenchStockLevelInOut *io = (OCTACBenchStockLevelInOut *)*inout;
  OCTACBenchStockLevelInput *in = &(io->input);
  OCTACBenchStockLevelOutput *out = &(io->output);
  static OCSQL sql1 = NULL;
  static OCSQL sql2 = NULL;
  sword errcode = 0;

  memset(out, 0, sizeof(*out));

  if (sql1 == NULL)
    sql1 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT d_next_o_id FROM district "
                             "WHERE d_w_id = :1 AND d_id = :2");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql1, errhp, svchp,
                                     in->w_id,
                                     in->d_id));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql1, errhp,
                                       out->o_id));
  if (errcode != 0) goto end;

  if (sql2 == NULL)
    sql2 = OCSQL_initWithSQL(envhp, errhp,
                             "SELECT COUNT(DISTINCT (s_i_id)) "
                             "FROM order_line, stock "
                             "WHERE ol_w_id = :1 AND ol_d_id = :2 "
                             "  AND ol_o_id < :3 AND ol_o_id >= :4 - 20 "
                             "  AND s_w_id = :5 AND s_i_id = ol_i_id "
                             "  AND s_quantity < :6");
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_execute(sql2, errhp, svchp,
                                     in->w_id,
                                     in->d_id,
                                     out->o_id,
                                     out->o_id,
                                     in->w_id,
                                     in->threshold));
  if (errcode != 0) goto end;
  errcode = OCOCIERROR(errhp, errmsg, errmsgsize,
                       OCSQL_fetchInto(sql2, errhp,
                                       out->stock_count));
  if (errcode != 0) goto end;

  OCITransCommit(svchp, errhp, (ub4) 0);

 end:
  /* if (sql1 != NULL) */
  /* { */
  /*   OCSQL_release(sql1); */
  /*   sql1 = NULL; */
  /* } */
  /* if (sql2 != NULL) */
  /* { */
  /*   OCSQL_release(sql2); */
  /*   sql2 = NULL; */
  /* } */

  return errcode;
}

void OCTACBenchStockLevel_afterTX(OCTACBenchStockLevelInOut *inout,
                                  struct timeval think_time)
{
  OCTACConfig_sleepThinkTime(think_time);
}
