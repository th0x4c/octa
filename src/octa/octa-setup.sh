#!/bin/sh

MODE=""
USER_ID=""
PARALLEL_DEGREE=0
SCALE_FACTOR=0
NUM_PARTITION=0
TABLE_TABLESPACE=""
INDEX_TABLESPACE=""

ACCOUNTS_PER_BRANCH=100000
TELLERS_PER_BRANCH=10

OCTA=$(dirname $0)/octa

usage()
{
  cat <<EOF
Description:
    Setup octa (create table, index, ... and load data)

Usage: setup-octa.sh [options]

Option:
    -B  TPC-B like mode
    -C  TPC-C like mode
    -u  UserID (username/password@connect_identifier)
    -n  Parallel degree
    -s  Scale factor (number of branches/warehouses)
    -p  Number of partitions
    -T  Tablespace name for tables
    -I  Tablespace name for indexes
    -h  Print Help (this message) and exit

Example:
    setup-octa.sh -B -u scott/tiger@orcl -n 5 -s 10 -T USERS -I INDX
EOF
}

welcome()
{
  echo "----------------------------------------------------------------"
  echo "                   Setup for TPC-${MODE} like mode              "
  echo "----------------------------------------------------------------"
  echo "                         UserID : ${USER_ID}"
  echo "                Parallel degree : ${PARALLEL_DEGREE} "
  echo "                   Scale factor : ${SCALE_FACTOR}"
  echo "           Number of partitions : ${NUM_PARTITION}"
  echo "     Tablespace name for tables : ${TABLE_TABLESPACE}"
  echo "    Tablespace name for indexes : ${INDEX_TABLESPACE}"
  echo "----------------------------------------------------------------"
  echo "----------------------------------------------------------------"
  echo "              Start Time : $(date +%Y-%m-%dT%H:%M:%S)"
  echo "----------------------------------------------------------------"
}

bye()
{
  echo "----------------------------------------------------------------"
  echo "                End Time : $(date +%Y-%m-%dT%H:%M:%S)"
  echo "----------------------------------------------------------------"
}

table_partitioning_clause()
{
  local partition_key=$1
  local num_table_rows=$2
  local num_partition=$3
  local tablespace_name=$4
  local rows_per_partition=0
  local value=1
  local length=$(expr length $num_partition)
  local partition_name=""
  local i=1

  if [ $num_partition -gt 0 ]
  then
    rows_per_partition=$(expr $num_table_rows / $num_partition)
  fi

  if [ $rows_per_partition -gt 0 ]
  then
    echo "PARTITION BY RANGE($partition_key)"
    echo "("
    i=1
    value=$(expr 1 + $rows_per_partition)
    while [ $value -lt $num_table_rows  ]
    do
      partition_name=$(printf "p%0${length}d" $i)
      echo "  PARTITION ${partition_name} VALUES LESS THAN (${value}) TABLESPACE ${tablespace_name},"
      value=$(expr $value + $rows_per_partition)
      i=$(expr $i + 1)
    done
    partition_name=$(printf "p%0${length}d" $i)
    echo "  PARTITION ${partition_name} VALUES LESS THAN (MAXVALUE) TABLESPACE ${tablespace_name}"
    echo ")"
  fi
}

index_partitioning_clause()
{
  local num_table_rows=$1
  local num_partition=$2
  local rows_per_partition=0
  local i=1

  if [ $num_partition -gt 0 ]
  then
    rows_per_partition=$(expr $num_table_rows / $num_partition)
  fi

  if [ $rows_per_partition -gt 0 ]
  then
    echo "LOCAL"
  fi
}

create_table_tpcb()
{
  local num_branches=$SCALE_FACTOR
  local num_tellers=$(expr $SCALE_FACTOR \* $TELLERS_PER_BRANCH)
  local num_accounts=$(expr $SCALE_FACTOR \* $ACCOUNTS_PER_BRANCH)

  echo "CREATE TABLE account"
  echo "("
  echo "  account_id       NUMBER(10,0),"
  echo "  branch_id        NUMBER(10,0),"
  echo "  account_balance  NUMBER(10,0),"
  echo "  filler           VARCHAR2(97) "
  echo ")"
  echo "TABLESPACE $ACCOUNT_TABLESPACE"
  table_partitioning_clause "account_id" $num_accounts $NUM_PARTITION $ACCOUNT_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE teller"
  echo "("
  echo "  teller_id       NUMBER(10,0),"
  echo "  branch_id       NUMBER(10,0),"
  echo "  teller_balance  NUMBER(10,0),"
  echo "  filler          CHAR(97)"
  echo ")"
  echo "TABLESPACE $TELLER_TABLESPACE"
  table_partitioning_clause "teller_id" $num_tellers $NUM_PARTITION $TELLER_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE branch"
  echo "("
  echo "  branch_id       NUMBER(10,0),"
  echo "  branch_balance  NUMBER(10,0),"
  echo "  filler          CHAR(98)"
  echo ") "
  echo "TABLESPACE $BRANCH_TABLESPACE"
  table_partitioning_clause "branch_id" $num_branches $NUM_PARTITION $BRANCH_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE history"
  echo "("
  echo "  teller_id         NUMBER,"
  echo "  branch_id         NUMBER,"
  echo "  account_id        NUMBER,"
  echo "  amount            NUMBER,"
  echo "  timestamp         TIMESTAMP,"
  echo "  filler            CHAR(39)"
  echo ")"
  echo "TABLESPACE $HISTORY_TABLESPACE"
  table_partitioning_clause "teller_id" $num_tellers $NUM_PARTITION $TELLER_TABLESPACE
  echo "/"
  echo "list"
}

create_index_tpcb()
{
  local num_branches=$SCALE_FACTOR
  local num_tellers=$(expr $SCALE_FACTOR \* $TELLERS_PER_BRANCH)
  local num_accounts=$(expr $SCALE_FACTOR \* $ACCOUNTS_PER_BRANCH)

  echo "CREATE UNIQUE INDEX account_pk"
  echo "ON account ( account_id )"
  index_partitioning_clause $num_accounts $NUM_PARTITION
  echo "TABLESPACE $ACCOUNT_INDEX_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX teller_pk"
  echo "ON teller ( teller_id )"
  index_partitioning_clause $num_tellers $NUM_PARTITION
  echo "TABLESPACE $TELLER_INDEX_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX branch_pk"
  echo "ON branch ( branch_id )"
  index_partitioning_clause $num_branches $NUM_PARTITION
  echo "TABLESPACE $BRANCH_INDEX_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"
}

add_constraint_tpcb()
{
  echo "ALTER TABLE account"
  echo "ADD CONSTRAINT account_pk"
  echo "PRIMARY KEY ( account_id )"
  echo "USING INDEX account_pk"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "ADD CONSTRAINT teller_pk"
  echo "PRIMARY KEY ( teller_id )"
  echo "USING INDEX teller_pk"
  echo "/"
  echo "list"

  echo "ALTER TABLE branch"
  echo "ADD CONSTRAINT branch_pk"
  echo "PRIMARY KEY ( branch_id )"
  echo "USING INDEX branch_pk"
  echo "/"
  echo "list"

  echo "ALTER TABLE account"
  echo "ADD CONSTRAINT account_fk"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "ADD CONSTRAINT teller_fk"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk1"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk2"
  echo "FOREIGN KEY ( teller_id )"
  echo "REFERENCES teller ( teller_id )"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk3"
  echo "FOREIGN KEY ( account_id )"
  echo "REFERENCES account ( account_id )"
  echo "/"
  echo "list"
}

analyze_tpcb()
{
  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'ACCOUNT',"
  echo "                                degree  => ${PARALLEL_DEGREE},"
  echo "                                cascade => true);"
  echo "END;"
  echo "/"
  echo "list"

  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'TELLER',"
  echo "                                degree  => ${PARALLEL_DEGREE},"
  echo "                                cascade => true);"
  echo "END;"
  echo "/"
  echo "list"

  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'BRANCH',"
  echo "                                degree  => ${PARALLEL_DEGREE},"
  echo "                                cascade => true);"
  echo "END;"
  echo "/"
  echo "list"

  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'HISTORY',"
  echo "                                degree  => ${PARALLEL_DEGREE},"
  echo "                                cascade => true);"
  echo "END;"
  echo "/"
  echo "list"
}

while [ $# -gt 0 ]
do
  case $1 in
    -B )
      MODE="B"
      shift
      ;;
    -C )
      MODE="C"
      shift
      ;;
    -u )
      shift
      if [ -n "$1" ]
      then
        USER_ID=$1
        shift
      fi
      ;;
    -n )
      shift
      if [ -n "$1" ]
      then
        PARALLEL_DEGREE=$1
        shift
      fi
      ;;
    -s )
      shift
      if [ -n "$1" ]
      then
        SCALE_FACTOR=$1
        shift
      fi
      ;;
    -p )
      shift
      if [ -n "$1" ]
      then
        NUM_PARTITION=$1
        shift
      fi
      ;;
    -T )
      shift
      if [ -n "$1" ]
      then
        TABLE_TABLESPACE=$1
        shift
      fi
      ;;
    -I )
      shift
      if [ -n "$1" ]
      then
        INDEX_TABLESPACE=$1
        shift
      fi
      ;;
    -h|--help )
      usage
      exit
      ;;
    * )
      echo "invalid option: $1"
      usage
      exit 1
      break
      ;;
  esac
done

if [ -z "$MODE" -o -z "$USER_ID" ]
then
  usage
  exit
fi

if [ "$MODE" != "B" -a "$MODE" != "C" ]
then
  usage
  exit
fi

sqlplus -S -L $USER_ID <<EOF
exit
EOF
if [ $? -ne 0 ]
then
  usage
  exit
fi

if [ $PARALLEL_DEGREE -eq 0 -o $SCALE_FACTOR -eq 0 ]
then
  usage
  exit
fi

if [ -z "$TABLE_TABLESPACE" -o -z "$INDEX_TABLESPACE" ]
then
  usage
  exit
fi

ACCOUNT_TABLESPACE=$TABLE_TABLESPACE
TELLER_TABLESPACE=$TABLE_TABLESPACE
BRANCH_TABLESPACE=$TABLE_TABLESPACE
HISTORY_TABLESPACE=$TABLE_TABLESPACE

ACCOUNT_INDEX_TABLESPACE=$INDEX_TABLESPACE
TELLER_INDEX_TABLESPACE=$INDEX_TABLESPACE
BRANCH_INDEX_TABLESPACE=$INDEX_TABLESPACE

welcome

echo "Create Table"
create_table_tpcb | sqlplus -S ${USER_ID}

$OCTA -u ${USER_ID} -${MODE} -n $PARALLEL_DEGREE -s $SCALE_FACTOR load

echo "Create Index"
create_index_tpcb | sqlplus -S ${USER_ID}

echo "Add Constraint"
add_constraint_tpcb | sqlplus -S ${USER_ID}

echo "Analyze"
analyze_tpcb | sqlplus -S ${USER_ID}

bye
