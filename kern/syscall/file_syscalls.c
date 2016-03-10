/* This is where all the file-system  calls shall be implemented */

#include <types.h>
#include <kern/file_syscall.h>
#include <kern/fcntl.h>
#include <kern/stat.h>

#include <current.h>
#include <lib.h>
#include <vfs.h>
#include <proc.h>
#include <limits.h>
#include <copyinout.h>

#include <synch.h>
#include <kern/errno.h>
#include <uio.h>
#include <vnode.h>


int sys_open(userptr_t filename,int flags,int mode, int* retval){
	// int retVal;


	//note filename is a user-pointer and we dont trust them
	//we will use copyinstr to copy string from  user space to kernel space

	// int copyinstr(const_userptr_t usersrc, char *dest, size_t len, size_t *got);
	
	// //creating a new fd structure
	// struct fd *newfile = NULL;

	// newfile = kmalloc(sizeof(struct fd*));

	// newfile->filename =  filename;//(char*)kmalloc(NAME_MAX*sizeof(char));
	// newfile->flags = flags;
	// newfile->offset = 0;
	// newfile->lk = kmalloc(sizeof(struct lock*));
	// newfile->refcount = 0;

	// retVal =  vfs_open(filename, flags, 0, &newfile->vn);

	// if(retVal){
	// 	int i=0;
	// 	while( i < OPEN_MAX && curproc->fd_table[i] != NULL)
	// 			i++;
	// 	if(i < OPEN_MAX){
	// 		curproc->fd_table[i] = newfile;
	// 		return i;	
	// 	}

	// }
	
	// return -1;
	int result;

	(void )mode;

	//char fname[NAME_MAX];

	char* fname;

	fname = kmalloc(PATH_MAX );
	
	result = copyinstr(filename, fname, PATH_MAX, NULL);

	if (result) { //if successful in copying the string filename to kernel space, result is 0 else the result is the error code which we return
		kfree(fname);
		return result; //returns the appropriate error code.
	}

	struct vnode *vn;
	struct fd *file;
	
	// ODO:checkflag  KASSERT( flags == O_);
	
	result = vfs_open(fname, flags, 0, &vn);
	if (result) {
		kfree(fname);
		return result;
	}

	file = kmalloc(sizeof(struct fd));
	if (file == NULL) {
		vfs_close(vn);
		return ENOMEM;
	}

	file->lk = lock_create("lock");
	if (file->lk == NULL) {
		vfs_close(vn);
		kfree(file);
		return ENOMEM;
	}

	file->vn = vn;
	file->offset = 0;
	file->refcount = 1;

	file->flags = flags;
	
	/* fetching the file descriptor */
	int i=0;
	while( i < OPEN_MAX && curproc->fd_table[i] != NULL)
		i++;
	
	if(i < OPEN_MAX){
			curproc->fd_table[i] = file;
			*retval = i;
	}
	else{
		lock_destroy(file->lk);
		vfs_close(file->vn);
		kfree(file);		
		return -1;
	}

	return 0;
}



int sys_close(int fd){
	//(void)fd;
	/*
	 	given  the fd index in ft_table retrieve the file descriptor
	 	check if the ref count is 1,
	 		if 1 make the fd null, and free the file
	 	else
			just decrement the count of the reference
	*/

		int res;
		struct fd* file;

		res = h_get_fd(fd,&file);

		if(res){
			return res; //if there was a bas file number detected
		}
		
		/* we check the ref count and make decisions accordingly */
		/* prior to which we need to aquire the lock on the file */

		lock_acquire(file->lk);

		if(file->refcount == 1) //meaning, this is the only reference , we can gc the file(call vfs_close)
		{	
			vfs_close(file->vn);
			lock_destroy(file->lk);
			kfree(file);

			//making the fd available again
			curproc->fd_table[fd] = NULL;
		}
		else if(file->refcount > 1)//just decrement the counter and release the lock
		{
			file->refcount--;
			lock_release(file->lk);
		}
		else return -1;

	return 0;
}

int sys_read(int fd, userptr_t buf, size_t buflen, int* retval){
	/*(void) fd;
	(void) buf;
	(void) buflen;

	*/
	struct fd* file;
	
	struct iovec iov;
	struct uio io;

	int res;

	/* fetching the fd structure, given a valid fd */
	res = h_get_fd(fd,&file);

	if(res){
		return res; //if there was a bas file number detected
	}

	//acquiring the lock
	lock_acquire(file->lk);

	iov.iov_ubase = buf;
	iov.iov_len = buflen;
    
    io.uio_iov = &iov;
	io.uio_iovcnt = 1;
	io.uio_offset = file->offset;
	io.uio_resid = buflen;
	io.uio_segflg = UIO_USERSPACE;
	io.uio_rw = UIO_READ;
	io.uio_space = curproc->p_addrspace;

	/*calling VOP read that does the actual read */
	res = VOP_READ(file->vn,&io);

	//res returns 0 if its success, else we release the lock and return res
	if(res)
	{
		lock_release(file->lk);
		return res;
	}

	//if success, we update the offset in the file based on what is 
	//obtained from the uio structure
	//setting the seek , max possible is buflen

	file->offset = io.uio_offset;

	lock_release(file->lk);

	*retval = buflen - io.uio_resid;

	return 0;
}

int sys_write(int fd, userptr_t buf, size_t buflen,int* retval){
/*  (void) fd;
	(void) buf;
	(void) buflen; */

	kprintf("%s",(char*)buf);

	struct fd* file;
	
	struct iovec iov;
	struct uio io;

	int res;

	/* fetching the fd structure, given a valid fd */
	res = h_get_fd(fd,&file);

	if(res){
		return res; //if there was a bas file number detected
	}

	//acquiring the lock
	lock_acquire(file->lk);

	iov.iov_ubase = buf;
	iov.iov_len = buflen;
    
    io.uio_iov = &iov;
	io.uio_iovcnt = 1;
	io.uio_offset = file->offset;
	io.uio_resid = buflen;
	io.uio_segflg = UIO_USERSPACE;
	io.uio_rw = UIO_WRITE;
	io.uio_space = curproc->p_addrspace;

	/*calling VOP read that does the actual read */
	res = VOP_WRITE(file->vn,&io);

	//res returns 0 if its success, else we release the lock and return res
	if(res)
	{
		lock_release(file->lk);
		return res;
	}

	//if success, we update the offset in the file based on what is 
	//obtained from the uio structure
	//setting the seek , max possible is buflen

	file->offset = io.uio_offset;

	lock_release(file->lk);

	*retval = buflen - io.uio_resid;


	return 0;
	
}

int sys_lseek(int fd, off_t pos, int whence, off_t* retval){
	/*(void) fd;
	(void) pos;
	(void) whence;  
	(void) retval;*/

	struct fd* file;
	struct stat fstat;

	int res;

	//  fetching the fd structure, given a valid fd 
	res = h_get_fd(fd,&file);

	if(res)
		return res;

	//acquiring the lock
	lock_acquire(file->lk);


	//we next need to check if the file is seekable
	res =  VOP_ISSEEKABLE(file->vn);

	if(!res){ //returns 0 , if the file is not seekable
		lock_release(file->lk);
		return res;
	}

	/*

	we set the whence based on its type

	SEEK_SET      0       // Seek relative to beginning of file 
	SEEK_CUR      1       // Seek relative to current position in file 
	SEEK_END      2       // Seek relative to end of file 

	*/
	switch(whence){
		case 0:
			*retval = pos;
			break;
		case 1:
			*retval = file->offset + pos;
			break;
		case 2:
			res = VOP_STAT(file->vn, &fstat);
			if(res){
				lock_release(file->lk);
				return res;
			}

			*retval = fstat.st_size + pos;
			
			break;
		default:
			lock_release(file->lk);
			return EINVAL;
	}


	/* closing off by setting the file offset to the ret value */

	file->offset = *retval;

	lock_release(file->lk);

	return 0;
}

int sys_dup2(int oldfd, int newfd){
	(void) oldfd;
	(void) newfd;

	/* duplicate fd and store it in the newfd*/
	int res;
	struct fd* file;

	if(newfd < 0 || newfd > OPEN_MAX -1)
		return EBADF;

	res = h_get_fd(oldfd,&file);

	if(res){
		return res;
	}

	//checking if oldfd and  newfd are already same
	if(oldfd == newfd) return 0;

	//next if newfd is not null, we try to close it 
	if(curproc->fd_table[newfd] != NULL)
	{
		res = sys_close(newfd);
		if(res)
			return res;
	}

	//if the program reaches this stage, it means
	//we have a valid oldfd and a  null newfd
	//all we need to do is 
	// 1. increment the ref count of file
	// 2. assign the fd struct of oldfd to newfd

	lock_acquire(file->lk);
		file->refcount++;
	lock_release(file->lk);

	curproc->fd_table[newfd] = file;

	return 0;
}	

int sys_chdir(userptr_t pathname){
	/* (void) pathname;*/

	//we handle the userptr pathname using copyinstr
	//next we simply call

	char pname[PATH_MAX];
	int res;

	res = copyinstr(pathname, pname, sizeof(pname), NULL);

	if (res) { //if successful in copying the string filename to kernel space, result is 0 else the result is the error code which we return
		return res; //returns the appropriate error code.

	}
	
	return vfs_chdir(pname);
}

int sys___getcwd(userptr_t buf, size_t buflen,int* retval ){
	/*(void) buf;
	(void) buflen;
	(void) retval;*/

	int res;
	struct iovec iov;
	struct uio io;

	iov.iov_ubase = buf;
	iov.iov_len = buflen;
    
    io.uio_iov = &iov;
	io.uio_iovcnt = 1;
	io.uio_offset = 0;
	io.uio_resid = buflen;
	io.uio_segflg = UIO_USERSPACE;
	io.uio_rw = UIO_READ;
	io.uio_space = curproc->p_addrspace;

	res = vfs_getcwd(&io);
	
	if (res) {
		return res;
	}

	*retval = buflen - io.uio_resid;

	return 0;
}


int h_get_fd(int fd,struct fd ** file){
	//this fucnction check that the fd index requested is valid and is no null.
	if(fd < 0  || fd > OPEN_MAX-1)
		return EBADF;    //return bad file number error code

	//new that the file descriptor is within range we need to check ,if its not null
	*file = curproc->fd_table[fd];

	if(*file == NULL) return EBADF;

	return 0;
}