/********************************

Saif Mahmud (SH-54)
Tauhid Tanjim (SH-58)

*********************************/
#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <limits.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <proc.h>
#include <syscall.h>
#include <copyinout.h>
#include <machine/trapframe.h>
/*
 * Add your file-related functions here ...
 */

// TODO: Take in 3rd argument register and give to vfs_open (mode_t)
// Although this is not even implemented in OS161 so whatever...

static int open(char *filename, int flags, int descriptor);
static int close(int filehandler, struct proc *proc);

static int NUM_OPEN_FILES = 0;

int sys_open(userptr_t filename, int flags, int *ret) {
	size_t length;

	int fd, result, error_log;
	
	if (filename == NULL) {
		return EFAULT;
	}

	char *kfilename = kmalloc((PATH_MAX)*sizeof(char));
	
	if (kfilename == NULL) {
		return ENFILE;
	}

	result = copyinstr(filename, kfilename, PATH_MAX, &length);
	
	if (result) {
		kfree(kfilename);
		return result;
	}

	for (fd = 3; fd < MAX_PROCESS_OPEN_FILES; fd++) {
		if (curproc->file_table[i] == NULL) {
			break;
		}
	}
	
	if (fd == MAX_PROCESS_OPEN_FILES) {
		kfree(kfilename);
		return EMFILE;
	}
	
	if (NUM_OPEN_FILES >= MAX_SYSTEM_OPEN_FILES) {
		kfree(kfilename);
		return ENFILE;
	}

	error_log = open(kfilename, flags, i);
	if(error_log){
		kfree(kfilename);
		return error_log;
	}

	*ret = fd;
	NUM_OPEN_FILES++;
	
	return 0;
}

int sys_close(int filehandler) {
	
	return close(filehandler, curproc);

}

int sys_read(int filehandler, userptr_t buf, size_t size, int *ret) {
	struct iovec iov;
	struct uio read_uio;

	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->file_table[filehandler]) {
		return EBADF;
	}

	struct File *file = curproc->file_table[filehandler];

	int mode = file->open_flags & O_ACCMODE;
	
	if (mode == O_WRONLY) {
		return EBADF;
	}
	
	lock_acquire(file->flock);
	off_t old_offset = file->offset;

	uio_uinit(&iov, &read_uio, buf, size, file->offset, UIO_READ);

	int result = VOP_READ(file->v_ptr, &read_uio);

	if (result) {
		lock_release(file->flock);
		return result;
	}

	file->offset = read_uio.uio_offset;
	*ret = file->offset - old_offset;
	lock_release(file->flock);

	return 0;
}

int sys_write(int filehandler, userptr_t buf, size_t size, int *ret) {
	struct iovec file_iovec;
	struct uio write_uio;

	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->file_table[filehandler]) {		
		return EBADF;
	}

	struct File *file = curproc->file_table[filehandler];

	int mode = file->open_flags & O_ACCMODE;
	if (mode == O_RDONLY) {
		return EBADF;
	}
	
	lock_acquire(file->flock);
	
	off_t old_offset = file->offset;
	uio_uinit(&file_iovec, &write_uio, buf, size, file->offset, UIO_WRITE);
	
	int result = VOP_WRITE(file->v_ptr, &write_uio);
	if (result) {
		lock_release(file->flock);
		return result;
	}
	
	file->offset = write_uio.uio_offset;
	*ret = file->offset - old_offset;
	
	lock_release(file->flock);
	
	return 0;
}

int sys_dup2(int oldfd, int newfd) {

	if(oldfd < 0 || oldfd >= MAX_PROCESS_OPEN_FILES || newfd < 0 || newfd >= MAX_PROCESS_OPEN_FILES || !curproc->file_table[oldfd]) {
		return EBADF;
	}

	if(oldfd == newfd){
		return 0;
	}

	if(curproc->file_table[newfd]){
		int result = sys_close(newfd);
		if(result){
			return result;
		}
	}

	struct File *file = curproc->file_table[newfd] = curproc->file_table[oldfd];
	
	lock_acquire(file->flock);
	file->references++;
	lock_release(file->flock);
	
	return 0;
}

int sys_lseek(int fd, off_t pos, userptr_t whence_ptr, off_t *ret) {
	struct File *file;
	struct stat stats;

	int lseek_location;
	
	if(fd < 0 || fd > MAX_PROCESS_OPEN_FILES || !(file = curproc->file_table[fd])){
		return EBADF;
	}

	if(!VOP_ISSEEKABLE(file->v_ptr)){
		return ESPIPE;
	}

	int result = VOP_STAT(file->v_ptr, &stats);
	if(result){
		return result;
	}

	result = copyin(whence_ptr, &lseek_location, sizeof(int));
	if(result) {
		return result;
	}
	
	switch(lseek_location){
		case SEEK_SET:
			if(pos < 0){
				return EINVAL;
			}
			
			lock_acquire(file->flock);
			*ret = file->offset = pos;
			lock_release(file->flock);
			
			break;

		case SEEK_CUR:
			lock_acquire(file->flock);
			
			if(file->offset + pos < 0){
				lock_release(file->flock);
				return EINVAL;
			}
			*ret = file->offset += pos;

			lock_release(file->flock);
			
			break;
		
		case SEEK_END:
			if(stats.st_size + pos < 0){
				return EINVAL;
			}
			
			lock_acquire(file->flock);
			*ret = file->offset = stats.st_size + pos; 
			lock_release(file->flock);
			
			break;

		default:
			return EINVAL;
	}
	
	return 0;
}

static int open(char *filename, int flags, int descriptor){

	struct File *file = kmalloc(sizeof(struct File*));
	
	int result;
	struct vnode *vn;
	
	if(!file){
		return ENFILE;
	}
 
	result = vfs_open(filename, flags, 0, &vn);

	if (result) {
		kfree(file);
		return result;
	}

	file->flock = lock_create("lock create");

	if(!file->flock) {
		vfs_close(file->v_ptr);
		kfree(file);

		return ENFILE;
	}

	file->offset = 0;
	file->open_flags = flags;
	file->references = 1;
	file->v_ptr=vn;
	
	curproc->file_table[descriptor] = file;

	return 0;
}

static int close(int filehandler, struct proc *proc) {

	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !proc->file_table[filehandler]) {
		return EBADF;
	}

	struct File *file = proc->file_table[filehandler];

	lock_acquire(file->flock);
	
	proc->file_table[filehandler] = NULL;
	file->references --;
	
	if(file->references<=0) {
		
		lock_release(file->flock);
		vfs_close(file->v_ptr);
		lock_destroy(file->flock);
		
		kfree(file);
	}
	else {
		lock_release(file->flock);
	}

	NUM_OPEN_FILES--;
	
	return 0;
}
