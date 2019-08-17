dist_bin_SCRIPTS = octa-setup.sh
bin_PROGRAMS = octa
octa_SOURCES = OCTAMain.h              \
               OCTAMain.c              \
               OCTAOption.h            \
               OCTAOption.c            \
               OCTADDL.h               \
               OCTADDL.c               \
               OCTASetup.h             \
               OCTASetup.c             \
               OCTATeardown.h          \
               OCTATeardown.c          \
               OCTABCreateTable.h      \
               OCTABCreateIndex.h      \
               OCTABAddConstraint.h    \
               OCTABAnalyze.h          \
               OCTABDropTable.h        \
               OCTABSetup.h            \
               OCTABSetup.c            \
               OCTABLoad.h             \
               OCTABLoad.c             \
               OCTABTeardown.h         \
               OCTABTeardown.c         \
               OCTABBench.h            \
               OCTABBench.c            \
               OCTACCreateTable.h      \
               OCTACCreateIndex.h      \
               OCTACAddConstraint.h    \
               OCTACAnalyze.h          \
               OCTACDropTable.h        \
               OCTACSetup.h            \
               OCTACSetup.c            \
               OCTACConfig.h           \
               OCTACConfig.c           \
               OCTACLoad.h             \
               OCTACLoad.c             \
               OCTACLoadItem.h         \
               OCTACLoadItem.c         \
               OCTACLoadWarehouse.h    \
               OCTACLoadWarehouse.c    \
               OCTACLoadStock.h        \
               OCTACLoadStock.c        \
               OCTACLoadDistrict.h     \
               OCTACLoadDistrict.c     \
               OCTACLoadCustomer.h     \
               OCTACLoadCustomer.c     \
               OCTACLoadOrders.h       \
               OCTACLoadOrders.c       \
               OCTACTeardown.h         \
               OCTACTeardown.c         \
               OCTACBench.h            \
               OCTACBench.c            \
               OCTACBenchNewOrder.h    \
               OCTACBenchNewOrder.c    \
               OCTACBenchPayment.h     \
               OCTACBenchPayment.c     \
               OCTACBenchOrderStatus.h \
               OCTACBenchOrderStatus.c \
               OCTACBenchDelivery.h    \
               OCTACBenchDelivery.c    \
               OCTACBenchStockLevel.h  \
               OCTACBenchStockLevel.c  \
               OCTACDist.h             \
               OCTACDist.c
octa_LDADD = ../OC/libOC.a ../TA/libTA.a
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(ORACLE_HOME)/rdbms/public
AM_LDFLAGS = -L$(ORACLE_HOME)/lib/ -L$(ORACLE_HOME)/rdbms/lib/ \
             -lclntsh -ldl -lm
