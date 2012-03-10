/*
 * Forked from https://gist.github.com/236001
 * C unit testing framework
 * by yukioc
 *
 * JTN002 - MinUnit -- a minimal unit testing framework for C
 * http://www.jera.com/techinfo/jtns/jtn002.html
 */

#ifndef __munit_h__
#define __munit_h__
#include <stdio.h>
#define mu_failed(file,line,expr) printf( "%s:%u: failed assertion `%s'\n",file,line,expr)
#define mu_tested(test,passed) printf( "Test: %-10s ... %s\n",test,(passed)?"passed":"FAILED")
#define mu_assert(expr) do{mu_nassert++;if(!(expr)){++mu_nfail;mu_failed(__FILE__,__LINE__,#expr);}}while(0)
#define mu_run_test(test) do{int f=mu_nfail;++mu_ntest;test();mu_tested(#test,(f==mu_nfail));}while(0)
#define mu_show_failures() do{printf("### Failed %d of %d asserts (%d tests).\n",mu_nfail,mu_nassert,mu_ntest);}while(0)
extern int mu_nfail;
extern int mu_ntest;
extern int mu_nassert;
#endif /* __munit_h__ */

/*
 * int mu_nfail=0;
 * int mu_ntest=0;
 * int mu_nassert=0;
 * static void test_foo(){
 *   int foo=7;
 *   mu_assert(foo==7);
 *   mu_assert(foo==0||foo==7);
 * }
 * static void test_bar() {
 *   int bar=4,fail=4;
 *   mu_assert(bar==4);
 *   mu_assert(fail==5);
 * }
 * static void test_hoge() {
 *   int hoge=7;
 *   mu_assert(hoge==7);
 * }
 * static void test_fuga() {
 *   int fuga=7;
 *   mu_assert(fuga==7);
 * }
 *
 * int main(int argc, char **argv) {
 *   mu_run_test(test_foo);
 *   mu_run_test(test_bar);
 *   mu_run_test(test_hoge);
 *   mu_run_test(test_fuga);
 *   mu_show_failures();
 *   return mu_nfail!=0;
 * }
 */
