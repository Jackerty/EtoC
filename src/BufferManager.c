/**********************************************
* See BufferManager.h for module description. *
**********************************************/
#define _GNU_SOURCE
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"BufferManager.h"
#ifdef __linux__
	#include<linux/version.h>
#endif /* __linux__ */

/**********************************************
* Preprocessing on different API avaible or   *
* if everything else fails then use blocking  *
* IO.                                         *
**********************************************/
#if LINUX_VERSION_CODE>=KERNEL_VERSION(5,1,0)
/**********************************************
* Linux version has to be more then 5.1.0 to  *
* use linux io_uring as asynchronous IO.      *
**********************************************/
#include<signal.h>
#include<sys/syscall.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<linux/io_uring.h>
#include<errno.h>

	/**********************************************
	* Since we are using linux system calls       *
	* diectly let's make easy to use static       *
	* inlines of these functions. Look liburing's *
	* syscall.c for descriptions.                 *
	**********************************************/
	static inline int ioUringSetup(unsigned int entries,struct io_uring_params *p){
		return syscall(__NR_io_uring_setup,entries,p);
	}
	static inline int ioUringEnter(int fd,unsigned int numsubmit,unsigned int mincompl,unsigned int flags,sigset_t *sig){ 
		// Last parameter has something to do with signals.
		return syscall(__NR_io_uring_enter,fd,numsubmit,mincompl,flags,sig,_NSIG/8);
	}
/**********************************************
* File descriptor of io_uring and uring       *
* parameters.                                 *
**********************************************/
int IoUringFd;
void *Sq,*Cq;
struct io_uring_sqe *Sqe;
size_t SqLength;
size_t SqeLength;
size_t CqLength;
struct io_uring_cqe *Cqes;
uint32_t *SqHead;
uint32_t *SqTail;
uint32_t *SqMask;
uint32_t *CqHead;
uint32_t *CqTail;
uint32_t *CqMask;

	/**********************
	* See BufferManager.h *
	**********************/
	int initIoBufferManager(){	
		
		// First let's do equal job to liburing's
		// io_uring_queue_init.
		struct io_uring_params param;
		memset(&param,0,sizeof(param));
		param.flags=IORING_SETUP_SQPOLL;		
		IoUringFd=ioUringSetup(64,&param);
		if(IoUringFd>=0){
			
			SqLength=param.sq_off.array+param.sq_entries*sizeof(uint32_t);
			Sq=mmap(0,SqLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_SQ_RING);
			if(Sq==MAP_FAILED){
				
				SqHead=Sq+param.sq_off.head;
				SqTail=Sq+param.sq_off.tail;
				SqMask=Sq+param.sq_off.ring_mask;
				
				CqLength=param.cq_off.cqes+param.cq_entries*sizeof(struct io_uring_cqe);
				Cq=mmap(0,CqLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_CQ_RING);
				if(Cq==MAP_FAILED){
					
					CqHead=Cq+param.cq_off.head;
					CqTail=Cq+param.cq_off.tail;
					CqMask=Cq+param.cq_off.ring_mask;
					Cqes=Cq+param.cq_off.cqes;

					SqeLength=param.sq_entries*sizeof(struct io_uring_sqe);
					Sqe=mmap(0,SqeLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_SQES);
					if(Sqe==MAP_FAILED){
							return 0;
					}
					munmap(Cq,CqLength);
				}
				munmap(Sq,SqLength);
			}
			close(IoUringFd);
		}
		return errno;

	}
	/**********************
	* See BufferManager.h *
	**********************/
	int deinitIoBufferManager(){
		munmap(Sq,SqeLength);
		munmap(Cq,CqLength);
		munmap(Sq,SqLength);
		close(IoUringFd);	
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int allocIoBuffer(IoBuffer *buffer,size_t bufferlength){
		return 0;
	}		
	/**********************
	* See BufferManager.h *
	**********************/
	int freeIoBuffer(IoBuffer *buffer){
		return 0;
	}
	/**********************************************
	* Send buffer for reading and swap active     *
	* buffer.                                     *
	**********************************************/
	static void sendAndSwapIoBuffer(IoBuffer *buffer){
		;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t getIoBufferByte(IoBuffer *buffer){
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint32_t getIoBuffer4Byte(IoBuffer *buffer){
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint64_t getIoBuffer8Byte(IoBuffer *buffer){
		return 0;
	}

#else
/**********************************************
* If asyncnorance read and writes aren't      *
* avaible on system compiled then this else   *
* will be implementation done by blocking     *
* reads.                                      *
**********************************************/
#error "There is no implementation for blocking BufferManager.c. DO IT!"

	/**********************
	* See BufferManager.h *
	**********************/
	int initIoBufferManager(uint32_t initialmemory,uint32_t numbuffers){	
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int deinitIoBufferManager(){
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int allocIoBuffer(IoBuffer *buff,uint32_t bufferlength){
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t getIoBufferByte(IoBuffer *buffer){
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint32_t getIoBuffer4Byte(IoBuffer *buffer){
		return 0;
	}

#endif /* LINUX_VERSION_CODE>=KERNEL_VERSION(5,1,0) */
