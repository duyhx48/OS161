#include <types.h>
#include <kern/proc_syscall.h>
#include <machine/trapframe.h>


int sys_fork(struct trapframe *tf, pid_t *retval){
	(void) tf;
	(void) retval;

	return 0;
}

int sys_execv(userptr_t prog, userptr_t args){
	(void) prog;
	(void) args;

	return 0;
}

void sys__exit(int code){
	(void)code;
}

int sys_getpid(pid_t *retval){
	(void) retval;

	return 0;
}

int sys_waitpid(pid_t pid, userptr_t returncode, int flags, pid_t *retval){
	(void) pid;
	(void) returncode;
	(void) flags;
	(void) retval;
	return 0;
}