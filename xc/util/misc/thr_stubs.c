/* $XConsortium: thr_stubs.c,v 1.1 93/12/06 16:24:21 kaleb Exp $ */
/*
 * Stub interface to thread routines that Solaris needs but shipped
 * broken/buggy versions in 5.2 and 5.3
 *
 * One workaround is to include this stub routine when you link.
 *
 * These routines don't need to have accurate interfaces -- they will 
 * never be called. They just need to be there in order to be resolved 
 * at link time by non-threaded programs.
 */

typedef int thread_t;

thread_t thr_self(void) { return 0; }
int thr_create(void) { return 0; }
int mutex_init(void) { return 0; }
int mutex_destroy(void) { return 0; }
int mutex_lock(void) { return 0; }
int mutex_unlock(void) { return 0; }
int cond_init(void) { return 0; }
int cond_destroy(void) { return 0; }
int cond_wait(void) { return 0; }
int cond_signal(void) { return 0; }
int cond_broadcast(void) { return 0; }
