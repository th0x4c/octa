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
  char *optstring = "u:T:I:n:s:h";
  int ch;
  char *c;
  extern char *optarg;
  extern int optind;
  extern int optopt;
  extern int opterr;
  extern int optreset;

  if (argc == 0)
    OCTAOption_usage();

  while ((ch = getopt(argc, argv, optstring)) != -1)
  {
    switch (ch)
    {
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
    case '?':
    default:
      OCTAOption_usage();
    }
  }
  argc -= optind;
  argv += optind;

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
    "       octa -u <userid> -n <sessions> -s <scale_factor> -T <table_tablespace> -I <index_tablespace> setup\n"
    "       octa -u <userid> -n <sessions> -s <scale_factor> load\n"
    "       octa -u <userid> teardown\n"
    "\n"
    "Option:\n"
    "\t-u \tUserID (username/password@connect_identifier)\n"
    "\t-n \tNumber of sessions on loading\n"
    "\t-s \tScale factor (number of branches)\n"
    "\t-T \tTablespace name for tables\n"
    "\t-I \tTablespace name for indexes\n"
    "\t-h \tPrint Help (this message) and exit\n"
    "\n"
    "Command:\n"
    "\tsetup    \tSetup (create table, index, ... and load data)\n"
    "\tload     \tLoad data\n"
    "\tteardown \tTeardown (drop table and related objects)\n"
    "\n"
    "Example:\n"
    "\tocta -u scott/tiger@orcl -n 5 -s 10 -T USERS -I INDX setup\n"
    "\tocta -u scott/tiger@orcl teardown\n";

  fprintf(stderr, "%s", usage);
  exit(1);
}
