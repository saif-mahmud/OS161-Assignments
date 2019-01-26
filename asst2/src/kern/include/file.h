/**
* This file is added for assignment 2
**/
#ifndef __FILE_H
#define __FILE_H

#include <types.h>
#include <synch.h>
#include <vnode.h>
/* Some limits  */

/* This is the maximum number of files that can be opened per
 * process */
#define MAX_PROCESS_OPEN_FILES  16

/* This is the maximum number of files that can be open in the system
 * at any one time */
#define MAX_SYSTEM_OPEN_FILES   64 
struct vnode;
struct lock;
/*
 * Put your function declarations and data types here ...
 */

struct File {
	struct vnode *v_ptr;
    int open_flags;
	int references;
	struct lock *flock;
    off_t offset;
};

//void open_std(void);
//int filetable_init(struct proc *newproc);
int sys_open(userptr_t filename, int flags, int *ret);
int sys_read(int filehandler, userptr_t buf, size_t size, int *ret);
int sys_write(int filehandler, userptr_t buf, size_t size, int *ret);
int sys_close(int filehandler);
int sys_dup2(int oldfd, int newfd);
int sys_lseek(int fd, off_t pos, userptr_t whence_ptr, off_t *ret);
//static int open2(struct proc *newproc,char *filename, int flags, int descriptor);
/* Place your data-structures here ... */

#endif
