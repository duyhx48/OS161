/* This is where all the proc-system calls shall be implemented */

#include <machine/trapframe.h>
#include <types.h>

int sys_fork(struct trapframe *tf, pid_t *retval);

int sys_execv(userptr_t prog, userptr_t args);

void sys__exit(int code);

int sys_getpid(pid_t *retval);

int sys_waitpid(pid_t pid, userptr_t returncode, int flags, pid_t *retval);

