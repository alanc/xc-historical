/* $XConsortium: thr_stubs.c,v 1.0 93/10/25 15:35:10 kaleb Exp $ */
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

void thr_self(void) {}
void thr_create(void) {}
void mutex_init(void) {}
void mutex_destroy(void) {}
void mutex_lock(void) {}
void mutex_unlock(void) {}
void cond_init(void) {}
void cond_destroy(void) {}
void cond_wait(void) {}
void cond_signal(void) {}
void cond_broadcast(void) {}
