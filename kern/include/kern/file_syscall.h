#ifndef FILE_SYSCALL_H_
#define FILE_SYSCALL_H_

#include <types.h>
#include <limits.h>
#include <proc.h>

/* Implementing structure for the file descriptor */



/* file table structure*/
//struct fd* fl_table[PID_MAX - PID_MIN];


/* implementing the signatures of all the system calls */

int sys_open(userptr_t filename, int flags,  int mode, int* retval);

int sys_close(int fd);

int sys_read(int fd, userptr_t buf, size_t buflen, int* retval);

int sys_write(int fd, userptr_t buf, size_t buflen, int* retval);

int sys_lseek(int fd, off_t pos, int whence, off_t* retval);

int sys_dup2(int oldfd, int newfd);

int sys_chdir(userptr_t pathname);

int sys___getcwd(userptr_t buf, size_t buflen, int* retval);

/* helper functions */	
int h_get_fd(int fd,struct fd ** file);

#endif