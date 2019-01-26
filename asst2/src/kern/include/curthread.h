#ifndef _CURTHREAD_H_
#define _CURTHREAD_H_

/*
 * The current thread. This file is added for ASST2
 *
 * This is in its own header file (instead of thread.h) to reduce the
 * number of things that get recompiled when you change thread.h.
 */

struct thread;

extern struct thread *curthread;

#endif /* _CURTHREAD_H_ */
