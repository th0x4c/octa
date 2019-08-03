check_PROGRAMS = test_TABool           \
                 test_TALog            \
                 test_TADistribution   \
                 test_TATXStat         \
                 test_TASession        \
                 test_TASessionManager \
                 test_TANet
TESTS = test_TABool           \
        test_TALog            \
        test_TADistribution   \
        test_TATXStat         \
        test_TASession        \
        test_TASessionManager \
        test_TANet
test_TABool_SOURCES = ../munit.h test_TABool.c
test_TALog_SOURCES = ../munit.h test_TALog.c
test_TADistribution_SOURCES = ../munit.h test_TADistribution.c
test_TATXStat_SOURCES = ../munit.h test_TATXStat.c
test_TASession_SOURCES = ../munit.h test_TASession.c
test_TASessionManager_SOURCES = ../munit.h test_TASessionManager.c
test_TANet_SOURCES = ../munit.h test_TANet.c
LDADD = ../../src/TA/libTA.a
AM_CPPFLAGS = -I$(top_srcdir)/src
AUTOMAKE_OPTIONS = color-tests
