check_PROGRAMS = test_OCOracle
TESTS = test_OCOracle
test_OCOracle_SOURCES = ../munit.h test_OCOracle.c
LDADD = ../../src/OC/libOC.a
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(ORACLE_HOME)/rdbms/public
AM_LDFLAGS = -L$(ORACLE_HOME)/lib/ -L$(ORACLE_HOME)/rdbms/lib/ \
             -lclntsh -ldl -lm
AUTOMAKE_OPTIONS = color-tests
