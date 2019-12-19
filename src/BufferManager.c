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
#include<sys/uio.h>
#include<linux/io_uring.h>
#include<errno.h>
#include<stdatomic.h>
#include<pthread.h>

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
static int IoUringFd;
static void *Sq,*Cq;
static struct io_uring_sqe *Sqe;
static size_t SqLength;
static size_t SqeLength;
static size_t CqLength;
static struct io_uring_cqe *Cqes;
static uint32_t *SqHead;
static uint32_t *SqTail;
static _Atomic uint32_t SqVirtualTail;
static uint32_t *SqMask;
static uint32_t *SqArray;
static uint32_t *SqFlags;
static uint32_t *CqHead;
static uint32_t *CqTail;
static uint32_t *CqMask;
static uint16_t numberOfBuffers;
/**********************************************
* Variable tells how many buffers are         *
* submited at ones to when ioUringEnter is    *
* called. Has to be power of two minus one as *
* the number is stored in minus one format    *
* (expected one).                             *
**********************************************/
static uint32_t expected;
/**********************************************
* State array for each buffer.                *
**********************************************/
static atomic_int_fast8_t *ready;
/**********************************************
* Flag bit do we use sqpolling.               *
**********************************************/
static union{
	struct{
		uint8_t nosqpoll:1;
	}bits;
	uint8_t shadow;
}Flag;

	/**********************
	* See BufferManager.h *
	**********************/
	int initIoBufferManager(uint16_t numberofbuffers,IoBuffer **buffers){
		
		// First let's do equal job to liburing's
		// io_uring_queue_init.
		struct io_uring_params param;
		memset(&param,0,sizeof(param));
		// If uid is zero a.k.a root we can use
		// kernel side polling and not have
		// to send submits constantly.
		Flag.bits.nosqpoll=(getuid()!=0);
		if(!Flag.bits.nosqpoll){
			param.flags=IORING_SETUP_SQPOLL;
		}
		// We allocate ring size to be number of buffers simple because
		// we give every buffer there own io_uring_sqe structure entry. 
		// No need for allocating and deallocating io_uring_sqe structures.
		IoUringFd=ioUringSetup(numberofbuffers,&param);
		if(IoUringFd>=0){
			
			SqLength=param.sq_off.array+param.sq_entries*sizeof(uint32_t);
			Sq=mmap(0,SqLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_SQ_RING);
			if(Sq!=MAP_FAILED){
				
				SqHead=Sq+param.sq_off.head;
				SqTail=Sq+param.sq_off.tail;
				SqVirtualTail=*SqTail;
				SqMask=Sq+param.sq_off.ring_mask;
				SqArray=Sq+param.sq_off.array;
				SqFlags=Sq+param.sq_off.flags;
				
				CqLength=param.cq_off.cqes+param.cq_entries*sizeof(struct io_uring_cqe);
				Cq=mmap(0,CqLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_CQ_RING);
				if(Cq!=MAP_FAILED){
					
					CqHead=Cq+param.cq_off.head;
					CqTail=Cq+param.cq_off.tail;
					CqMask=Cq+param.cq_off.ring_mask;
					Cqes=Cq+param.cq_off.cqes;

					SqeLength=param.sq_entries*sizeof(struct io_uring_sqe);
					Sqe=mmap(0,SqeLength,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_POPULATE,IoUringFd,IORING_OFF_SQES);
					if(Sqe!=MAP_FAILED){
							
							// Allocate buffers.
							// TODO: Should this be allocated to cache line alignment by element
							//       so that threads don't cause writebacks to other threads work?
							*buffers=malloc(sizeof(IoBuffer)*numberofbuffers);
							numberOfBuffers=numberofbuffers;
							if(buffers){
								
								ready=malloc(sizeof(atomic_int_fast8_t)*numberofbuffers);
								if(ready){
									
									// Initialize the buffers for reading.
									// This way we avoid having to call 
									// preporation for reading function or
									// make so that every time we call for 
									// read we have to prepare the structure.
									for(uint16_t b=0;b<numberofbuffers;b++){
										
										(*buffers)[b].constsqe=Sqe+b;
										(*buffers)[b].constsqe->opcode=IORING_OP_READV;
										(*buffers)[b].constsqe->flags=0;
										(*buffers)[b].constsqe->ioprio=0;
										(*buffers)[b].constsqe->off=0;
										(*buffers)[b].constsqe->addr=(uint64_t)&(*buffers)[b].vec;
										(*buffers)[b].constsqe->len=1;
										(*buffers)[b].constsqe->rw_flags=0;
										(*buffers)[b].constsqe->user_data=b;
										(*buffers)[b].constsqe->__pad2[0]=0;
										(*buffers)[b].constsqe->__pad2[1]=0;
										(*buffers)[b].constsqe->__pad2[2]=0;
										(*buffers)[b].buffpoint=0;
										(*buffers)[b].forwardhead=0;
									}
									
									#if 0
									// Get smaller power of two of the number.
									expected=1;
									while(expected<numberOfBuffers) expected<<=1;
									expected>>=1;
									#endif
									return 0;
								}
								free(buffers);
							}
							munmap(Sqe,SqLength);
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
	int deinitIoBufferManager(IoBuffer *buffers){
		free(ready);
		free(buffers);
		munmap(Sq,SqeLength);
		munmap(Cq,CqLength);
		munmap(Sq,SqLength);
		close(IoUringFd);
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int prepIoBufferForRead(IoBuffer *buffer){
		buffer->constsqe;
		
		
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int prepIoBufferForWrite(IoBuffer *buffer){
		buffer;
		
		
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
	static int sendIoBuffer(IoBuffer *buffer){
		// Get mask and Sqe we are adding to the ring.
		// Volatile is there to make sure that optimazer
		// does move address taking to ring adding making
		// operation non-atomic.
		volatile uint32_t mask=*SqMask;
		volatile uint32_t sqeindex=(uint32_t)buffer->constsqe->user_data;
		
		// Set ready state to send.
		ready[sqeindex]=0;
		
		// We have to somehow allocate tail index and move SqTails value.
		// This should be done atomicly so that other thread doesn't allocte
		// same index. Problem is that if we have kthread polling we can't
		// add the tail before ring has the sqeindex.
		uint32_t tail=SqVirtualTail++;
		SqArray[tail&mask]=sqeindex;
		atomic_fetch_add(SqTail,1);
		
		if(Flag.bits.nosqpoll){
			#if 1
			if(ioUringEnter(IoUringFd,1,0,0,0)==1) return 0;
			else return 1;
			#else
			// Idea here is that when tail modulos of expected
			// (which is power of 2 so that overflow isn't problem)
			// is zero then we call submit by amount of expected.
			// TODO: Has a bug where if less the expected file cause
			//       IO behavior has to change!
			if((tail&expected)==0){
				return ioUringEnter(IoUringFd,expected,0,0,0);
			}
			#endif
		}
		else if(atomic_load(SqFlags)&IORING_SQ_NEED_WAKEUP){
			if(ioUringEnter(IoUringFd,1,0,IORING_ENTER_SQ_WAKEUP,0)==1) return 0;
			return 1;
		}
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	void waitResponseIoBuffer(IoBuffer *buffer){
		static pthread_mutex_t testlock=PTHREAD_MUTEX_INITIALIZER;
		// Load user_data to the local variable for faster access.
		// user_data is thread id basicly that we use offset to 
		// right location in ready array.
		volatile uint32_t sqeindex=buffer->constsqe->user_data;
		if(ready[sqeindex]==0){
			
			if(pthread_mutex_trylock(&testlock)==0){
				do{
					// GDB doesn't see the *CqTail or *CqHead
					// since it is mmap to be shared only with
					// debug process and kernel so for debugging
					// purposes it would be better load to local
					// variables. Optimizer should load these these
					// to registers so it shouldn't matter. 
					uint32_t head=atomic_load(CqHead);
					uint32_t tail=atomic_load(CqTail);
					while(head!=tail){
						struct io_uring_cqe *cqe;
						unsigned index=head&(*CqMask);
						cqe=Cqes+index;
						if((int)cqe->res>=0) buffer->length+=(int)cqe->res;
						else buffer->length=-1;
						if(buffer->length<sizeof(buffer->buffers)/2) ready[cqe->user_data]=-1;
						else ready[cqe->user_data]=1;
						head++;
						atomic_store(CqHead,head);
					}
					// If our buffer isn't ready then we
					// have to poll changes in the tail.
				}while(ready[sqeindex]==0);
				pthread_mutex_unlock(&testlock);
			}
			else{
				while(ready[sqeindex]==0);
			}
		}
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int fullReadIoBuffer(IoBuffer *buffer){
		// For first read we can read both buffers
		// Set iovec to longer.
		buffer->vec.iov_base=buffer->buffers;
		buffer->vec.iov_len=sizeof(buffer->buffers);
		// Set length of the read to zero.
		// This is mostly so that old data is not left hanging.
		buffer->length=0;
		int result=sendIoBuffer(buffer);
		// Now we have 
		buffer->vec.iov_len=sizeof(buffer->buffers)/2;
		return result;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t getIoBufferByte(IoBuffer *buffer){
		// For some reason GCC didn't see that forwardhead is initialized
		// at the initIoBufferManager.
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wsequence-point"
		// We check that state isn't last read.
		// Special handle last read since we should
		// worry about not over reading.
		if(ready[buffer->constsqe->user_data]!=-1){
			buffer->forwardhead=(buffer->forwardhead++)&(sizeof(buffer->buffers)-1);
			if(buffer->forwardhead==sizeof(buffer->buffers)/2 || buffer->forwardhead==0){
				buffer->length-=sizeof(buffer->buffers)/2;
				waitResponseIoBuffer(buffer);
				if(buffer->length<=0) return 0;
			}
		}
		else if(buffer->forwardhead++==buffer->length) return 0;
		return buffer->buffers[buffer->forwardhead];
		#pragma GCC diagnostic pop
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t checkIoBufferStr(IoBuffer *restrict buffer,const uint8_t *restrict str,int32_t length){
		// Check that we keep in the bounds of the buffer.
		// Since forwardhead keeps track both buffers at the same time
		// we have to ask how much we offset in current buffer.
		// We use and rather then modulos for speed since
		// our buffer size has is power 2.
		if(length+(buffer->forwardhead&(sizeof(buffer->buffers)/2)-1)<sizeof(buffer->buffers)/2){
			// We can just compare since our string fit remainder
			// of the buffer.
			if(memcmp(buffer->buffers+buffer->forwardhead,str,length)==0){
				buffer->forwardhead+=length;
				if(isspace(buffer->buffers[buffer->forwardhead])){
					return 1;
				}
			}
		}
		else{
			// We have to read more then we have buffer.
			// Check buffer amount first and then on second
			// read check new buffer.
			size_t remainder=sizeof(buffer->buffers)-buffer->forwardhead;
			if(memcmp(buffer->buffers+buffer->forwardhead,str,remainder)==0){
				// Buffer is used so make sure that new buffer is read.
				
				size_t rest=length-remainder;
				if();
			}
		}
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t consumeIoBuffer(IoBuffer *buffer){
		// Check should we read new buffer.
		// Member buffpoint is used to ask which buffer will be done
		// and member forwardhead is used to tell are which buffer really done with.
		if(buffer->buffpoint<(sizeof(buffer->buffers)/2) && buffer->forwardhead>=(sizeof(buffer->buffers)/2)){
			buffer->vec.iov_base=buffer->buffers+sizeof(buffer->buffers)/2;
		}
		else if(buffer->buffpoint>=(sizeof(buffer->buffers)/2) && buffer->forwardhead<(sizeof(buffer->buffers)/2)){
			buffer->vec.iov_base=buffer->buffers;
		}
		else goto jmp_FAST_EXIT;
		
		sendIoBuffer(buffer);
		
		// Jump is used to when no new buffer isn't needed read.
		jmp_FAST_EXIT:
		buffer->buffpoint=buffer->forwardhead;
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t consumeCpyIoBuffer(IoBuffer *restrict buffer,uint8_t *str){
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
