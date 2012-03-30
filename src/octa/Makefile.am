bin_PROGRAMS = octa
octa_SOURCES = OCTAMain.h           \
               OCTAMain.c           \
               OCTAOption.h         \
               OCTAOption.c         \
               OCTADDL.h            \
               OCTADDL.c            \
               OCTASetup.h          \
               OCTASetup.c          \
               OCTATeardown.h       \
               OCTATeardown.c       \
               OCTABCreateTable.h   \
               OCTABCreateIndex.h   \
               OCTABAddConstraint.h \
               OCTABAnalyze.h       \
               OCTABDropTable.h     \
               OCTABSetup.h         \
               OCTABSetup.c         \
               OCTABLoad.h          \
               OCTABLoad.c          \
               OCTABTeardown.h      \
               OCTABTeardown.c      \
               OCTABBench.h         \
               OCTABBench.c
octa_LDADD = ../OC/libOC.a ../TA/libTA.a 
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(ORACLE_HOME)/rdbms/public
AM_LDFLAGS = -L$(ORACLE_HOME)/lib/ -L$(ORACLE_HOME)/rdbms/lib/ \
             -lclntsh -ldl -lm
