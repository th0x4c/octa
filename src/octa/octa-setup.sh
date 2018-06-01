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

Usage: octa-setup.sh [options]

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
    octa-setup.sh -B -u scott/tiger@orcl -n 5 -s 10 -T USERS -I INDX
    octa-setup.sh -C -u scott/tiger@orcl -n 5 -s 100 -p 10 -T USERS -I INDX
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
    while [ $i -lt $num_partition ]
    do
      partition_name=$(printf "p%0${length}d" $i)
      value=$(expr $num_table_rows \* $i / $num_partition + 1)
      echo "  PARTITION ${partition_name} VALUES LESS THAN (${value}) TABLESPACE ${tablespace_name},"
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
  echo "PARALLEL $PARALLEL_DEGREE"
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
  echo "PARALLEL $PARALLEL_DEGREE"
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
  echo "PARALLEL $PARALLEL_DEGREE"
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
  echo "PARALLEL $PARALLEL_DEGREE"
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
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "ADD CONSTRAINT teller_pk"
  echo "PRIMARY KEY ( teller_id )"
  echo "USING INDEX teller_pk"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE branch"
  echo "ADD CONSTRAINT branch_pk"
  echo "PRIMARY KEY ( branch_id )"
  echo "USING INDEX branch_pk"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE account"
  echo "ADD CONSTRAINT account_fk"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "ADD CONSTRAINT teller_fk"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk1"
  echo "FOREIGN KEY ( branch_id )"
  echo "REFERENCES branch ( branch_id )"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk2"
  echo "FOREIGN KEY ( teller_id )"
  echo "REFERENCES teller ( teller_id )"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "ADD CONSTRAINT history_fk3"
  echo "FOREIGN KEY ( account_id )"
  echo "REFERENCES account ( account_id )"
  echo "ENABLE NOVALIDATE"
  echo "/"
  echo "list"
}

validate_constraint_tpcb()
{
  echo "ALTER TABLE account"
  echo "MODIFY CONSTRAINT account_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "MODIFY CONSTRAINT teller_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE branch"
  echo "MODIFY CONSTRAINT branch_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE account"
  echo "MODIFY CONSTRAINT account_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE teller"
  echo "MODIFY CONSTRAINT teller_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "MODIFY CONSTRAINT history_fk1 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "MODIFY CONSTRAINT history_fk2 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "MODIFY CONSTRAINT history_fk3 VALIDATE"
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

create_table_tpcc()
{
  local num_warehouses=$SCALE_FACTOR

  echo "CREATE TABLE warehouse"
  echo "("
  echo "  w_id       NUMBER,       -- 2*W unique IDs"
  echo "  w_name     VARCHAR2(10),"
  echo "  w_street_1 VARCHAR2(20),"
  echo "  w_street_2 VARCHAR2(20),"
  echo "  w_city     VARCHAR2(20),"
  echo "  w_state    CHAR(2),"
  echo "  w_zip      CHAR(9),"
  echo "  w_tax      NUMBER(4, 4),"
  echo "  w_ytd      NUMBER(12, 2)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE district"
  echo "("
  echo "  d_id        NUMBER(2, 0), -- 20 unique IDs"
  echo "  d_w_id      NUMBER,       -- 2*W unique IDs"
  echo "  d_name      VARCHAR2(10),"
  echo "  d_street_1  VARCHAR2(20),"
  echo "  d_street_2  VARCHAR2(20),"
  echo "  d_city      VARCHAR2(20),"
  echo "  d_state     CHAR(2),"
  echo "  d_zip       CHAR(9),"
  echo "  d_tax       NUMBER(4, 4),"
  echo "  d_ytd       NUMBER(12, 2),"
  echo "  d_next_o_id NUMBER(8, 0)  -- 10,000,000 unique IDs"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "d_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE customer"
  echo "("
  echo "  c_id           NUMBER(5, 0), -- 96,000 unique IDs"
  echo "  c_d_id         NUMBER(2, 0), -- 20 unique IDs"
  echo "  c_w_id         NUMBER,       -- 2*W unique IDs"
  echo "  c_first        VARCHAR2(16),"
  echo "  c_middle       CHAR(2),"
  echo "  c_last         VARCHAR2(16),"
  echo "  c_street_1     VARCHAR2(20),"
  echo "  c_street_2     VARCHAR2(20),"
  echo "  c_city         VARCHAR2(20),"
  echo "  c_state        CHAR(2),"
  echo "  c_zip          CHAR(9),"
  echo "  c_phone        CHAR(16),"
  echo "  c_since        DATE,"
  echo "  c_credit       CHAR(2),"
  echo "  c_credit_lim   NUMBER(12, 2),"
  echo "  c_discount     NUMBER(4, 4),"
  echo "  c_balance      NUMBER(12, 2),"
  echo "  c_ytd_payment  NUMBER(12, 2),"
  echo "  c_payment_cnt  NUMBER(4, 0),"
  echo "  c_delivery_cnt NUMBER(4, 0),"
  echo "  c_data         VARCHAR2(500)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "c_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE history"
  echo "("
  echo "  h_c_id   NUMBER(5, 0), -- 96,000 unique IDs"
  echo "  h_c_d_id NUMBER(2, 0), -- 20 unique IDs"
  echo "  h_c_w_id NUMBER,       -- 2*W unique IDs"
  echo "  h_d_id   NUMBER(2, 0), -- 20 unique IDs"
  echo "  h_w_id   NUMBER,       -- 2*W unique IDs"
  echo "  h_date   DATE,"
  echo "  h_amount NUMBER(6, 2),"
  echo "  h_data   VARCHAR2(24)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "h_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE new_order"
  echo "("
  echo "  no_o_id NUMBER(8, 0), -- 10,000,000 unique IDs"
  echo "  no_d_id NUMBER(2, 0), -- 20 unique IDs"
  echo "  no_w_id NUMBER        -- 2*W unique IDs"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "no_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE orders"
  echo "("
  echo "  o_id         NUMBER(8, 0), -- 10,000,000 unique IDs"
  echo "  o_d_id       NUMBER(2, 0), -- 20 unique IDs"
  echo "  o_w_id       NUMBER,       -- 2*W unique IDs"
  echo "  o_c_id       NUMBER(5, 0), -- 96,000 unique IDs"
  echo "  o_entry_d    DATE,"
  echo "  o_carrier_id NUMBER(2, 0), -- 10 unique IDs, or null"
  echo "  o_ol_cnt     NUMBER(2, 0),"
  echo "  o_all_local  NUMBER(1, 0)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "o_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE order_line"
  echo "("
  echo "  ol_o_id        NUMBER(8, 0), -- 10,000,000 unique IDs"
  echo "  ol_d_id        NUMBER(2, 0), -- 20 unique IDs"
  echo "  ol_w_id        NUMBER,       -- 2*W unique IDs"
  echo "  ol_number      NUMBER(2, 0), -- 15 unique IDs"
  echo "  ol_i_id        NUMBER(6, 0), -- 200,000 unique IDs"
  echo "  ol_supply_w_id NUMBER,       -- 2*W unique IDs"
  echo "  ol_delivery_d  DATE,         -- date and time, or null"
  echo "  ol_quantity    NUMBER(2, 0),"
  echo "  ol_amount      NUMBER(6, 2),"
  echo "  ol_dist_info   CHAR(24)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "ol_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"

  echo "CREATE TABLE item"
  echo "("
  echo "  i_id    NUMBER(6, 0), -- 200,000 unique IDs"
  echo "  i_im_id NUMBER(6, 0), -- 200,000 unique IDs"
  echo "  i_name  VARCHAR2(24),"
  echo "  i_price NUMBER(5, 2),"
  echo "  i_data  VARCHAR2(50)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE TABLE stock"
  echo "("
  echo "  s_i_id       NUMBER(6, 0), -- 200,000 unique IDs"
  echo "  s_w_id       NUMBER,       -- 2*W unique IDs"
  echo "  s_quantity   NUMBER(4, 0),"
  echo "  s_dist_01    CHAR(24),"
  echo "  s_dist_02    CHAR(24),"
  echo "  s_dist_03    CHAR(24),"
  echo "  s_dist_04    CHAR(24),"
  echo "  s_dist_05    CHAR(24),"
  echo "  s_dist_06    CHAR(24),"
  echo "  s_dist_07    CHAR(24),"
  echo "  s_dist_08    CHAR(24),"
  echo "  s_dist_09    CHAR(24),"
  echo "  s_dist_10    CHAR(24),"
  echo "  s_ytd        NUMBER(8, 0),"
  echo "  s_order_cnt  NUMBER,"
  echo "  s_remote_cnt NUMBER,"
  echo "  s_data       VARCHAR2(50)"
  echo ")"
  echo "TABLESPACE $TABLE_TABLESPACE"
  echo "PARALLEL $PARALLEL_DEGREE"
  table_partitioning_clause "s_w_id" $num_warehouses $NUM_PARTITION $TABLE_TABLESPACE
  echo "/"
  echo "list"
}

create_index_tpcc()
{
  local num_warehouses=$SCALE_FACTOR

  echo "CREATE UNIQUE INDEX warehouse_pk"
  echo "  ON warehouse ( w_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX district_pk"
  echo "  ON district ( d_w_id, d_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX customer_pk"
  echo "  ON customer ( c_w_id, c_d_id, c_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX new_order_pk"
  echo "  ON new_order ( no_w_id, no_d_id, no_o_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX orders_pk"
  echo "  ON orders ( o_w_id, o_d_id, o_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX order_line_pk"
  echo "  ON order_line ( ol_w_id, ol_d_id, ol_o_id, ol_number )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX item_pk"
  echo "  ON item ( i_id )"
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX stock_pk"
  echo "  ON stock ( s_w_id, s_i_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  # Additional index
  echo "CREATE UNIQUE INDEX customer_idx"
  echo "  ON customer ( c_last, c_w_id, c_d_id, c_first, c_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"

  echo "CREATE UNIQUE INDEX orders_idx"
  echo "  ON orders ( o_c_id, o_d_id, o_w_id, o_id )"
  index_partitioning_clause $num_warehouses $NUM_PARTITION
  echo "  TABLESPACE $INDEX_TABLESPACE"
  echo "  PARALLEL $PARALLEL_DEGREE"
  echo "/"
  echo "list"
}

add_constraint_tpcc()
{
  # Primary key
  echo "ALTER TABLE warehouse ADD CONSTRAINT warehouse_pk"
  echo "  PRIMARY KEY ( w_id )"
  echo "  USING INDEX warehouse_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE district ADD CONSTRAINT district_pk"
  echo "  PRIMARY KEY ( d_w_id, d_id )"
  echo "  USING INDEX district_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE customer ADD CONSTRAINT customer_pk"
  echo "  PRIMARY KEY ( c_w_id, c_d_id, c_id )"
  echo "  USING INDEX customer_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE new_order ADD CONSTRAINT new_order_pk"
  echo "  PRIMARY KEY ( no_w_id, no_d_id, no_o_id )"
  echo "  USING INDEX new_order_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE orders ADD CONSTRAINT orders_pk"
  echo "  PRIMARY KEY ( o_w_id, o_d_id, o_id )"
  echo "  USING INDEX orders_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line ADD CONSTRAINT order_line_pk"
  echo "  PRIMARY KEY ( ol_w_id, ol_d_id, ol_o_id, ol_number )"
  echo "  USING INDEX order_line_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE item ADD CONSTRAINT item_pk"
  echo "  PRIMARY KEY ( i_id )"
  echo "  USING INDEX item_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock ADD CONSTRAINT stock_pk"
  echo "  PRIMARY KEY ( s_w_id, s_i_id )"
  echo "  USING INDEX stock_pk"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  # Foreign key
  echo "ALTER TABLE district ADD CONSTRAINT district_fk"
  echo "  FOREIGN KEY ( d_w_id )"
  echo "  REFERENCES warehouse ( w_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE customer ADD CONSTRAINT customer_fk"
  echo "  FOREIGN KEY ( c_w_id, c_d_id )"
  echo "  REFERENCES district ( d_w_id, d_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history ADD CONSTRAINT history_fk1"
  echo "  FOREIGN KEY ( h_c_w_id, h_c_d_id, h_c_id )"
  echo "  REFERENCES customer ( c_w_id, c_d_id, c_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history ADD CONSTRAINT history_fk2"
  echo "  FOREIGN KEY ( h_w_id, h_d_id )"
  echo "  REFERENCES district ( d_w_id, d_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE new_order ADD CONSTRAINT new_order_fk"
  echo "  FOREIGN KEY ( no_w_id, no_d_id, no_o_id )"
  echo "  REFERENCES orders ( o_w_id, o_d_id, o_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE orders ADD CONSTRAINT orders_fk"
  echo "  FOREIGN KEY ( o_w_id, o_d_id, o_c_id )"
  echo "  REFERENCES customer ( c_w_id, c_d_id, c_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line ADD CONSTRAINT order_line_fk1"
  echo "  FOREIGN KEY ( ol_w_id, ol_d_id, ol_o_id )"
  echo "  REFERENCES orders ( o_w_id, o_d_id, o_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line ADD CONSTRAINT order_line_fk2"
  echo "  FOREIGN KEY ( ol_supply_w_id, ol_i_id )"
  echo "  REFERENCES stock ( s_w_id, s_i_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock ADD CONSTRAINT stock_fk1"
  echo "  FOREIGN KEY ( s_w_id )"
  echo "  REFERENCES warehouse ( w_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock ADD CONSTRAINT stock_fk2"
  echo "  FOREIGN KEY ( s_i_id )"
  echo "  REFERENCES item ( i_id )"
  echo "  ENABLE NOVALIDATE"
  echo "/"
  echo "list"
}

validate_constraint_tpcc()
{
  # Primary key
  echo "ALTER TABLE warehouse"
  echo "MODIFY CONSTRAINT warehouse_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE district"
  echo "MODIFY CONSTRAINT district_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE customer"
  echo "MODIFY CONSTRAINT customer_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE new_order"
  echo "MODIFY CONSTRAINT new_order_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE orders"
  echo "MODIFY CONSTRAINT orders_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line"
  echo "MODIFY CONSTRAINT order_line_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE item"
  echo "MODIFY CONSTRAINT item_pk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock"
  echo "MODIFY CONSTRAINT stock_pk VALIDATE"
  echo "/"
  echo "list"

  # Foreign key
  echo "ALTER TABLE district"
  echo "MODIFY CONSTRAINT district_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE customer"
  echo "MODIFY CONSTRAINT customer_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "MODIFY CONSTRAINT history_fk1 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE history"
  echo "MODIFY CONSTRAINT history_fk2 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE new_order"
  echo "MODIFY CONSTRAINT new_order_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE orders"
  echo "MODIFY CONSTRAINT orders_fk VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line"
  echo "MODIFY CONSTRAINT order_line_fk1 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE order_line"
  echo "MODIFY CONSTRAINT order_line_fk2 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock"
  echo "MODIFY CONSTRAINT stock_fk1 VALIDATE"
  echo "/"
  echo "list"

  echo "ALTER TABLE stock"
  echo "MODIFY CONSTRAINT stock_fk2 VALIDATE"
  echo "/"
  echo "list"
}

analyze_tpcc()
{
  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'WAREHOUSE',"
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
  echo "                                tabname => 'DISTRICT',"
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
  echo "                                tabname => 'CUSTOMER',"
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

  echo "DECLARE"
  echo "  username USER_USERS.USERNAME%TYPE;"
  echo "BEGIN"
  echo "  SELECT USERNAME into username FROM USER_USERS;"
  echo "  DBMS_STATS.GATHER_TABLE_STATS(ownname => username,"
  echo "                                tabname => 'NEW_ORDER',"
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
  echo "                                tabname => 'ORDERS',"
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
  echo "                                tabname => 'ORDER_LINE',"
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
  echo "                                tabname => 'ITEM',"
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
  echo "                                tabname => 'STOCK',"
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

lower_mode=$(echo $MODE | tr "A-Z" "a-z")

echo "Create Table"
create_table_tpc${lower_mode} | sqlplus -S ${USER_ID}

$OCTA -u ${USER_ID} -${MODE} -n $PARALLEL_DEGREE -s $SCALE_FACTOR load

echo "Create Index"
create_index_tpc${lower_mode} | sqlplus -S ${USER_ID}

echo "Add Constraint"
add_constraint_tpc${lower_mode} | sqlplus -S ${USER_ID}

echo "Validate Constraint"
validate_constraint_tpc${lower_mode} | sqlplus -S ${USER_ID}

echo "Analyze"
analyze_tpc${lower_mode} | sqlplus -S ${USER_ID}

bye
