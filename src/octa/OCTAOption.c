/*
 * OCTAOption.c
 * OCTA
 *
 * Created by Takashi Hashizume on 03/17/12.
 * Copyright 2012 Takashi Hashizume. All rights reserved.
 */

#include "OCTAOption.h"

void OCTAOption_getOption(int argc, char * const argv[], OCTAOption *option)
{
  char *optstring = "BCu:T:I:n:s:m:U:D:k:t:p:h";
  int ch;
  char *c;
  int tpcc_default_percentages[TXS] = DEFAULT_PERCENTAGES;
  int tpcc_default_keying_times[TXS] = DEFAULT_KEYING_TIMES;
  int tpcc_default_think_times[TXS] = DEFAULT_THINK_TIMES;
  int t_option = FALSE;
  int i = 0;
  extern char *optarg;
  extern int optind;
  extern int optopt;
  extern int opterr;
  extern int optreset;

  if (argc == 0)
    OCTAOption_usage();

  memset(option, 0, sizeof(*option));
  for (i = 0; i < TXS; i++)
  {
    option->keying_time[i].tv_sec = tpcc_default_keying_times[i] / 1000;
    option->keying_time[i].tv_usec =
      tpcc_default_keying_times[i] % 1000 * 1000;
    option->think_time[i].tv_sec = tpcc_default_think_times[i] / 1000;
    option->think_time[i].tv_usec =
      tpcc_default_think_times[i] % 1000 * 1000;
    option->tx_percentage[i] = tpcc_default_percentages[i];
  }

  while ((ch = getopt(argc, argv, optstring)) != -1)
  {
    switch (ch)
    {
    case 'B':
      option->mode = OCTA_TPCB;
      break;
    case 'C':
      option->mode = OCTA_TPCC;
      break;
    case 'u':
      c = strtok(optarg, "/");
      if (c == NULL)
      {
        fprintf(stderr, "Invalid user ID\n");
        OCTAOption_usage();
      }
      strncpy(option->username, c, MAX_NAME_SIZE);
      option->username[MAX_NAME_SIZE - 1] = '\0';

      c = strtok(NULL, "@");
      if (c == NULL)
      {
        fprintf(stderr, "Invalid user ID\n");
        OCTAOption_usage();
      }
      strncpy(option->password, c, MAX_NAME_SIZE);
      option->password[MAX_NAME_SIZE - 1] = '\0';

      c = strtok(NULL, " ");
      if (c == NULL)
        strncpy(option->tnsname, "", MAX_NAME_SIZE);
      else
        strncpy(option->tnsname, c, MAX_NAME_SIZE);
      option->tnsname[MAX_NAME_SIZE - 1] = '\0';
      break;
    case 'T':
      strncpy(option->table_tablespace, optarg, MAX_NAME_SIZE);
      break;
    case 'I':
      strncpy(option->index_tablespace, optarg, MAX_NAME_SIZE);
      break;
    case 'n':
      option->num_sessions = atoi(optarg);
      break;
    case 's':
      option->scale_factor = atoi(optarg);
      break;
    case 'm':
      option->measurement_interval.tv_sec = atoi(optarg);
      option->measurement_interval.tv_usec = 0;
      break;
    case 'U':
      option->rampup_time.tv_sec = atoi(optarg);
      option->rampup_time.tv_usec = 0;
      break;
    case 'D':
      option->rampdown_time.tv_sec = atoi(optarg);
      option->rampdown_time.tv_usec = 0;
      break;
    case 'k':
      c = strtok(optarg, ",");
      if (c == NULL)
      {
        option->keying_time[0].tv_sec = atoi(optarg) / 1000;
        option->keying_time[0].tv_usec = (atoi(optarg) % 1000) * 1000;
      }
      else
      {
        for (i = 0; i < TXS && c != NULL; i++)
        {
          option->keying_time[i].tv_sec = atoi(c) / 1000;
          option->keying_time[i].tv_usec = (atoi(c) % 1000) * 1000;
          c = strtok(NULL, ",");
        }
      }
      break;
    case 't':
      t_option = TRUE;
      c = strtok(optarg, ",");
      if (c == NULL)
      {
        option->think_time[0].tv_sec = atoi(optarg) / 1000;
        option->think_time[0].tv_usec = (atoi(optarg) % 1000) * 1000;
      }
      else
      {
        for (i = 0; i < TXS && c != NULL; i++)
        {
          option->think_time[i].tv_sec = atoi(c) / 1000;
          option->think_time[i].tv_usec = (atoi(c) % 1000) * 1000;
          c = strtok(NULL, ",");
        }
      }
      break;
    case 'p':
      c = strtok(optarg, ",");
      if (c == NULL)
        option->tx_percentage[0] = atoi(optarg);
      else
      {
        for (i = 0; i < TXS && c != NULL; i++)
        {
          option->tx_percentage[i] = atoi(c);
          c = strtok(NULL, ",");
        }
      }
      break;
    case '?':
    default:
      OCTAOption_usage();
    }
  }
  argc -= optind;
  argv += optind;

  if (option->mode != OCTA_TPCB && option->mode != OCTA_TPCC)
    OCTAOption_usage();

  if (strlen(option->username) == 0 || strlen(option->password) == 0 ||
      argc == 0)
    OCTAOption_usage();

  if (strcmp(argv[0], "setup") == 0)
  {
    option->command = OCTA_SETUP;
    if (option->num_sessions == 0 || option->scale_factor == 0 ||
        strlen(option->table_tablespace) == 0 ||
        strlen(option->index_tablespace) == 0 )
      OCTAOption_usage();

    return;
  }

  if (strcmp(argv[0], "load") == 0)
  {
    option->command = OCTA_LOAD;
    if (option->num_sessions == 0 || option->scale_factor == 0)
      OCTAOption_usage();

    return;
  }

  if (strcmp(argv[0], "bench") == 0)
  {
    option->command = OCTA_BENCH;
    if (option->num_sessions == 0 || option->scale_factor == 0 ||
        !timerisset(&(option->measurement_interval)))
      OCTAOption_usage();

    if (option->mode == OCTA_TPCB && t_option == FALSE)
    {
      option->think_time[0].tv_sec = 0;
      option->think_time[0].tv_usec = 0;
    }

    return;
  }

  if (strcmp(argv[0], "teardown") == 0)
  {
    option->command = OCTA_TEARDOWN;
    return;
  }

  fprintf(stderr, "Invalid command: %s\n", argv[0]);
  OCTAOption_usage();
}

void OCTAOption_usage()
{
  static char* usage =
    "Usage: octa <option> <command>\n"
    "\n"
    "       octa -B|-C -u <userid> -n <sessions> -s <scale_factor> -T <table_tablespace> -I <index_tablespace> setup\n"
    "       octa -B|-C -u <userid> -n <sessions> -s <scale_factor> load\n"
    "       octa -B -u <userid> -n <sessions> -s <scale_factor> -m <measurement_interval> [-U <rampup_time>] [-D <rampdown_time>] [-t <think_time>] bench\n"
    "       octa -C -u <userid> -n <sessions> -s <scale_factor> -m <measurement_interval> [-U <rampup_time>] [-D <rampdown_time>] [-t <think_time>] [-k <keying_time>] [-p <percentage>] bench\n"
    "       octa -B|-C -u <userid> teardown\n"
    "\n"
    "Option:\n"
    "\t-B \tTPC-B like mode\n"
    "\t-C \tTPC-C like mode\n"
    "\t-u \tUserID (username/password@connect_identifier)\n"
    "\t-n \tNumber of sessions\n"
    "\t-s \tScale factor (number of branches/warehouses)\n"
    "\t-m \tMeasurement interval (in sec)\n"
    "\t-U \tRamp-up time (in sec)\n"
    "\t-D \tRamp-down time (in sec)\n"
    "\t-t \tThink time (in msec)\n"
    "\t-k \tKeying time (in msec)\n"
    "\t-p \tPercentage of transactions\n"
    "\t-T \tTablespace name for tables\n"
    "\t-I \tTablespace name for indexes\n"
    "\t-h \tPrint Help (this message) and exit\n"
    "\n"
    "Command:\n"
    "\tsetup    \tSetup (create table, index, ... and load data)\n"
    "\tload     \tLoad data\n"
    "\tbench    \tBenchmark loosely based on TPC-B/TPC-C\n"
    "\tteardown \tTeardown (drop table and related objects)\n"
    "\n"
    "Example:\n"
    "\tocta -u scott/tiger@orcl -B -n 5 -s 10 -T USERS -I INDX setup\n"
    "\tocta -u scott/tiger@orcl -B -n 5 -s 10 -m 600 -U 60 -D 60 -t 1 bench\n"
    "\tocta -u scott/tiger@orcl -B teardown\n"
    "\tocta -u scott/tiger@orcl -C -n 5 -s 10 -T USERS -I INDX setup\n"
    "\tocta -u scott/tiger@orcl -C -n 5 -s 10 -m 7200 -U 1200 -D 600 -t 18000,3000,2000,2000,2000 -k 12000,12000,10000,5000,5000 -p 45,43,4,4,4 bench\n"
    "\tocta -u scott/tiger@orcl -C teardown\n";

  fprintf(stderr, "%s", usage);
  exit(1);
}

void OCTAOption_print(OCTAOption option)
{
  int total = 0;
  int i = 0;

  for (i = 0; i < TXS; i++)
  {
    total += option.tx_percentage[i];
  }

  printf("----------------------------------------------------------------\n");
  printf("        OCTA (OCI Transaction Application) %s\n", VERSION);
  printf("----------------------------------------------------------------\n");
  printf("                           Mode : %s like\n",
         option.mode == OCTA_TPCB ? "TPC-B" : "TPC-C");
  printf("              Database username : %s\n", option.username);
  printf("              Database password : %s\n", option.password);
  printf("  Connect identifier (tnsnames) : %s\n", option.tnsname);
  printf("             Number of sessions : %d\n", option.num_sessions);
  printf("                   Scale factor : %d\n", option.scale_factor);
  printf("  Measurement interval (in sec) : %8.3f\n",
         timeval2sec(option.measurement_interval));
  printf("          Ramp-up time (in sec) : %8.3f\n",
         timeval2sec(option.rampup_time));
  printf("        Ramp-down time (in sec) : %8.3f\n",
         timeval2sec(option.rampdown_time));
  printf("            Think time (in sec) : ");
  switch (option.mode)
  {
  case OCTA_TPCB:
    printf("%8.3f\n", timeval2sec(option.think_time[0]));
    break;
  case OCTA_TPCC:
    printf("%8.3f, %8.3f, %8.3f, %8.3f, %8.3f\n",
           timeval2sec(option.think_time[IDX_NEW_ORDER]),
           timeval2sec(option.think_time[IDX_PAYMENT]),
           timeval2sec(option.think_time[IDX_ORDER_STATUS]),
           timeval2sec(option.think_time[IDX_DELIVERY]),
           timeval2sec(option.think_time[IDX_STOCK_LEVEL]));
    printf("           Keying time (in sec) : "
           "%8.3f, %8.3f, %8.3f, %8.3f, %8.3f\n",
           timeval2sec(option.keying_time[IDX_NEW_ORDER]),
           timeval2sec(option.keying_time[IDX_PAYMENT]),
           timeval2sec(option.keying_time[IDX_ORDER_STATUS]),
           timeval2sec(option.keying_time[IDX_DELIVERY]),
           timeval2sec(option.keying_time[IDX_STOCK_LEVEL]));
    printf("     Percentage of transactions : "
           "%6.2f, %6.2f, %6.2f, %6.2f, %6.2f\n",
           (double) option.tx_percentage[IDX_NEW_ORDER] * 100 / total,
           (double) option.tx_percentage[IDX_PAYMENT] * 100 / total,
           (double) option.tx_percentage[IDX_ORDER_STATUS] * 100 / total,
           (double) option.tx_percentage[IDX_DELIVERY] * 100 / total,
           (double) option.tx_percentage[IDX_STOCK_LEVEL] * 100 / total);
    break;
  default:
    break;
  }
  printf("----------------------------------------------------------------\n");
}
