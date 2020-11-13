/**********************************************
* See BufferManager.h for module description. *
**********************************************/
#define _GNU_SOURCE
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include"BufferManager.h"
#include"Etc.h"
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
#include<ctype.h>

	/**********************************************
	* Since we are using linux system calls       *
	* diectly let's make easy to use static       *
	* inlines of these functions. Look liburing's *
	* syscall.c for descriptions. Don't use       *
	* underscore naming so that that if someone   *
	* puts system calls declearation header no    *
	* conflict arises.                            *
	**********************************************/
	static inline int ioUringSetup(unsigned int entries,struct io_uring_params *p){
		return syscall(__NR_io_uring_setup,entries,p);
	}
	static inline int ioUringRegister(int fd,unsigned int opcode,const void *arg,unsigned nr_args){
		return syscall(__NR_io_uring_register,fd,opcode,arg,nr_args);
	}
	static inline int ioUringEnter(int fd,unsigned int numsubmit,unsigned int mincompl,unsigned int flags,sigset_t *sig){ 
		// Last parameter has something to do with signals.
		return syscall(__NR_io_uring_enter,fd,numsubmit,mincompl,flags,sig,_NSIG/8);
	}
/**********************************************
* Constant IO buffer sizes value. Can be      *
* changed from out side of the file by macro. *
* Is signed integer sinze IO gives back -1 if *
* error occurs so we are just considend.      *
* Default 4096 should caver most modern hard  *
* drives?                                     *
**********************************************/
#ifdef BUFFER_MANAFER_IO_BUFFER_SIZE
	const int32_t bufferManagerBufferSize=BUFFER_MANAFER_IO_BUFFER_SIZE;
#else
	const int32_t bufferManagerBufferSize=4096;
#endif

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
//TODO: Needed? Time of writting appears only in initIOBufferManager.
static int16_t numberOfBuffers;
/**********************************************
* Variable tells how many buffers are         *
* submited at ones to when ioUringEnter is    *
* called. Has to be power of two minus one as *
* the number is stored in minus one format    *
* (expected one).                             *
**********************************************/
static uint32_t expected;
/**********************************************
* State array for each buffer and macros for  *
* naming those states.                        *
**********************************************/
static atomic_int_fast8_t *ready;
#define READY_INFLIGHT 0
#define READY_RECEIVED_BACK 1
#define READY_NOMORE_ERROR -1
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
	int initIoBufferManager(int16_t numberofbuffers,IoBuffer **buffers){
		
		// Return value. Changed to indicate error.
		// Functions called which may cause errno 
		// are io_uring_setup, io_uring_register,
		// and malloc.
		int retuner=0;
		
		// First let's do equal job to liburing's
		// io_uring_queue_init.
		struct io_uring_params param;
		memset(&param,0,sizeof(param));
		
		// We can use kernel side polling
		// and not have to send submits 
		// constantly by IORING_SETUP_SQPOLL.
		// This is behind root access because
		// kernel thread is created.
		// It also requires file registration.
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
						
						// Allocate buffers structures.
						*buffers=malloc(sizeof(IoBuffer)*numberofbuffers);
						if(*buffers){
							numberOfBuffers=numberofbuffers;
							
							ready=malloc(sizeof(atomic_int_fast8_t)*numberofbuffers);
							if(ready){
								
								// Initialize the buffers for reading.
								// This way we avoid having to call 
								// preporation for reading function or
								// make so that every time we call for 
								// read we have to prepare the structure.
								for(int16_t b=numberofbuffers-1;b>=0;b--){
									
									(*buffers)[b].constsqe=Sqe+b;
									(*buffers)[b].constsqe->flags=IOSQE_FIXED_FILE;
									(*buffers)[b].constsqe->ioprio=0;
									(*buffers)[b].constsqe->off=0;
									(*buffers)[b].constsqe->rw_flags=0;
									(*buffers)[b].constsqe->fd=b;
									(*buffers)[b].constsqe->__pad2[0]=0;
									(*buffers)[b].constsqe->__pad2[1]=0;
									(*buffers)[b].constsqe->__pad2[2]=0;
									(*buffers)[b].buffpoint=0;
									(*buffers)[b].forwardhead=0;
									(*buffers)[b].constsqe->len=bufferManagerBufferSize;
									// Allocate the actual double buffer on it's own so
									// that they can be cache aligned.
									(*buffers)[b].buffers=malloc(2*bufferManagerBufferSize);
									prepIoBufferForRead((*buffers)+b);
								}
								
								// Give every buffer file registery.
								// Initialize register array by -1 
								// which is symbol for sparse.
								int arg[numberofbuffers];
								for(int16_t i=numberofbuffers;i>=0;i--) arg[i]=-1;
								if(ioUringRegister(IoUringFd,IORING_REGISTER_FILES,arg,numberofbuffers)==0){
									return 0;
								}
								retuner=-errno;
							}
							else{
								retuner=ECHILD;
							}
							free(buffers);
						}
						else{
							retuner=E2BIG;
						}
						munmap(Sqe,SqLength);
					}
					munmap(Cq,CqLength);
				}
				munmap(Sq,SqLength);
			}
			close(IoUringFd);
		}
		else retuner=errno;
		return retuner;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int deinitIoBufferManager(IoBuffer *buffers){
		// Free dynamic allocations of members.
		for(int16_t i=numberOfBuffers-1;i>=0;i--) free(buffers[i].buffers);
		// Free buffers array.
		free(buffers);
		// Free module's globals.
		free(ready);
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
		buffer->constsqe->opcode=IORING_OP_READ;
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int prepIoBufferForWrite(IoBuffer *buffer){
		//TODO: HOW TO SWITCH TO WRITING?
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
		ready[sqeindex]=READY_INFLIGHT;
		
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
		volatile uint32_t head;
		volatile uint32_t tail;
		if(ready[sqeindex]==READY_INFLIGHT){
			// This mutex makes sure only one thread handles
			// response queue. One that get's the lock will
			// handle every single one it sees and unlocks
			// if thread has found it's own response.
			// TODO: Is there theoritically possibility to get
			//       response so late that mutex lock should
			//       check in the while loop other treads
			//       are spinning. Or should we make this
			//       more friendlier to multiple threads.
			if(pthread_mutex_trylock(&testlock)==0){
				do{
					// GDB doesn't see the *CqTail or *CqHead
					// since it is mmap to be shared only with
					// debug process and kernel so for debugging
					// purposes it would be better load to local
					// variables. Optimizer should load these
					// to registers so it shouldn't matter.
					head=atomic_load(CqHead);
					tail=atomic_load(CqTail);
					while(head!=tail){
						struct io_uring_cqe *cqe;
						unsigned index=head&(*CqMask);
						cqe=Cqes+index;
						if((int32_t)cqe->res>=0) buffer->length+=(int32_t)cqe->res;
						else buffer->length=(int32_t)cqe->res;
						// Comparasion check that we didn't get back fully used buffer
						// a.k.a file did not have more bytes to be read.
						// We note that length may vary because fullReadIoBuffer read
						// both buffers in one go.
						if((buffer->length&(bufferManagerBufferSize-1))<bufferManagerBufferSize){
							ready[cqe->user_data]=READY_NOMORE_ERROR;
						}
						else ready[cqe->user_data]=READY_RECEIVED_BACK;
						head++;
						atomic_store(CqHead,head);
					}
					// If our buffer isn't ready then we
					// have to poll changes in the tail.
				}while(ready[sqeindex]==READY_INFLIGHT);
				pthread_mutex_unlock(&testlock);
			}
			else{
				// TODO: Read todo before mutex_trylock.
				while(ready[sqeindex]==READY_INFLIGHT);
			}
		}
	}
	/**********************************************
	* waitResponseIoBuffer function but decrement *
	* used buffers size. Is used internally in    *
	* module to keep length in check and check    *
	* next buffer amount has been read.           *
	* Should be called before reading  next       *
	* buffer.                                     *
	**********************************************/
	static inline void changeIoBuffer(IoBuffer *buffer){
		buffer->length-=bufferManagerBufferSize;
		waitResponseIoBuffer(buffer);
	}
	/**********************
	* See BufferManager.h *
	**********************/
	int fullReadIoBuffer(IoBuffer *buffer){
		// First read first buffer. Wait for the response. 
		buffer->constsqe->addr=(__u64)buffer->buffers;
		// Set length of the read to zero.
		// This is mostly so that old data is not left hanging.
		buffer->length=0;
		int result=sendIoBuffer(buffer);
		
		if(result>0);
		
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
		
		// We check that character isn't last read.
		// Special handle last read since we should
		// worry about over reading.
		int8_t byte=buffer->buffers[buffer->forwardhead];
		if(ready[buffer->constsqe->user_data]!=READY_NOMORE_ERROR){
			buffer->forwardhead=(++buffer->forwardhead)&(bufferManagerBufferSize*2-1);
			if(buffer->forwardhead==bufferManagerBufferSize || buffer->forwardhead==(bufferManagerBufferSize*2)){
				changeIoBuffer(buffer);
				if(buffer->length==0) return EOT;
				else if(buffer->length<0) return 0;
			}
		}
		return byte;
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
		// our buffer size is power 2.
		//TODO: Should memcmp move forwardhead
		//      amount that memcmp says so there
		//      is agreement when memcmp fails?
		if(length+(buffer->forwardhead&(bufferManagerBufferSize-1))<bufferManagerBufferSize){
			// We can just compare since our string fit remainder
			// of the buffer.
			if(memcmp(buffer->buffers+buffer->forwardhead,str,length)==0){
				// We are plussing one so that we are going to next letter.
				if(length+1+(buffer->forwardhead&(bufferManagerBufferSize-1))>=bufferManagerBufferSize){
					// TODO: How to report an error??
					changeIoBuffer(buffer);
					if(buffer->length<0) return 0;
				}
				buffer->forwardhead+=length+1;
				buffer->forwardhead&=bufferManagerBufferSize*2-1;

				// Check that there isn't more letters for string.
				if(!isAlNumUnder(checkByteIoBuffer(buffer))){
					return 1;
				}
			}
		}
		else{
			// We have to read more then we have buffer.
			// Check buffer amount first and then on second
			// read check new buffer.
			size_t remainder=bufferManagerBufferSize-buffer->forwardhead;
			if(memcmp(buffer->buffers+buffer->forwardhead,str,remainder)==0){
				// Buffer is used so make sure that new buffer is read.
				// Then read variable rest amount of new buffer.
				changeIoBuffer(buffer);
				if(buffer->length>0){
					// Update forwardhead to start of new buffer.
					// We do this by modulus which. Since buffer
					// size power of two we can use faster and 
					// minus one modulus.
					buffer->forwardhead+=remainder;
					buffer->forwardhead&=bufferManagerBufferSize*2-1;
					// Let's make rest variable so that we have easier
					// time to compare middle of the string.
					const size_t rest=length-remainder;
					if(memcmp(buffer->buffers+buffer->forwardhead,str+remainder,rest)==0){
						// We are reading length plus one since we are going
						// to next character. We just changed to buffer we 
						// don't have to worry that over reading.
						buffer->forwardhead+=length+1;
					
						// Check that there isn't more letter for string.
						if(!isAlNumUnder(checkByteIoBuffer(buffer))){
							return 1;
						}
					}
				}
				// We don't have more buffer to read?
				// TODO: Error report?? Do we have to
				//       tell caller to stop calling
				//       buffer manager about this
				//       file??
			}
		}
		return 0;
	}
	/**********************************************
	* Send new buffer in if buffer point moved    *
	* beyond buffer then send that buffer for     *
	* reading.                                    *
	**********************************************/
	static void switchAndSendIoBuffer(IoBuffer *buffer){
		//TODO: ENGLISH! DO YOU SPEAK IT!
		// Check should we read new buffer.
		// Member buffpoint is used to ask which buffer is be done
		// and member forwardhead is used to tell which buffer really done with.
		// else return statement means we state inside of a buffer.
		if(buffer->buffpoint<bufferManagerBufferSize && buffer->forwardhead>=bufferManagerBufferSize){
			buffer->constsqe->addr=(__u64)buffer->buffers+bufferManagerBufferSize;
		}
		else if(buffer->buffpoint>=bufferManagerBufferSize && buffer->forwardhead<bufferManagerBufferSize){
			buffer->constsqe->addr=(__u64)buffer->buffers;
		}
		else return;
		// We did move beyong buffer so send it for reading.
		sendIoBuffer(buffer);
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t consumeIoBuffer(IoBuffer *buffer){
		switchAndSendIoBuffer(buffer);
		// Just move bufferpoint to forwardhead as that
		// is how much we read.
		buffer->buffpoint=buffer->forwardhead;
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t consumeCpyIoBuffer(IoBuffer *restrict buffer,uint8_t *restrict str){
		// We have to copy what is the buffers to between bufferpoint and forwardhead.
		// We can't just give pointer point to buffer as buffer are reused and having
		// filed size buffer is too much memory usage. 
		
		switchAndSendIoBuffer(buffer);
		buffer->buffpoint=buffer->forwardhead;
		return 0;
	}
	/**********************
	* See BufferManager.h *
	**********************/
	uint8_t setIoBufferFd(IoBuffer *buffer,int fd){
		// How IORING_SETUP_SQPOLL needs file registration but it 
		// has other internal benefits as well. According to io_uring.pdf
		// file reference is queried per write/read so by file registering
		// this query is done ones per file description change.
		volatile uint32_t sqeindex=buffer->constsqe->user_data;
		struct io_uring_files_update update={.offset=sqeindex,.resv=0,.fds=(__aligned_u64)&fd};
		if(ioUringRegister(IoUringFd,IORING_REGISTER_FILES_UPDATE,&update,1)==0){
			return 0;
		}
		else return errno;
	}
#elif LINUX_VERSION_CODE<KERNEL_VERSION(5,1,0)
/**********************************************
* Before linux version 5.1.0 only avaible     *
* option for asynchronous IO was POSIX AIO    *
* interface which is implemented by blocking  *
* thread in GNU's c libary.                   *
**********************************************/
#error "There is no implementation for POSIX AIO BufferManager.c. DO IT!"

#else
/**********************************************
* If asyncnorance read and writes aren't      *
* avaible on system then this else will be    *
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
