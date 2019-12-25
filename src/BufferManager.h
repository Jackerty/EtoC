/*********************************************************
* Module enable asyncroness IO calls when possible and   *
* uses alternating buffer pairs.                         *
*********************************************************/
#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include<stdint.h>
#ifdef __linux__
	// Check that linux version supports io_uring.
	#include<linux/version.h>
	#if LINUX_VERSION_CODE>=KERNEL_VERSION(5,1,0)
		#include<sys/uio.h>
		#include<linux/io_uring.h>
	#endif
#endif /* __linux__ */

/***************************************
* Data structure to keep information   *
* about buffers there length and       *
* state.                               *
***************************************/
#if LINUX_VERSION_CODE>=KERNEL_VERSION(5,1,0)
typedef struct IoBuffer{
	struct io_uring_sqe *constsqe;
	int32_t buffpoint;
	int32_t forwardhead;
	int32_t length;
	uint8_t buffers[2*4096];
	struct iovec vec;
}IoBuffer;
#else
typedef struct IoBuffer{
	uint8_t *buffer;
	int fd;
}IoBuffer;
#endif /* LINUX_VERSION_CODE>=KERNEL_VERSION(5,1,0) */

/***************************************
* Initialize the buffer manager.       *
***************************************/
int initIoBufferManager(uint16_t numberofbuffers,IoBuffer **buffers);
/***************************************
* Deallocate any memory associated     *
* with BufferManager.                  *
***************************************/
int deinitIoBufferManager(IoBuffer *buffers);
/***************************************
* Allocate new buffer to be used in IO *
* buffer.                              *
***************************************/
int prepIoBufferForRead(IoBuffer *buff);
/***************************************
* Allocate new buffer to be used in IO *
* buffer.                              *
***************************************/
int prepIoBufferForWrite(IoBuffer *buff);
/***************************************
* Read buffer to the full. This        *
* function is ment to be called before *
* asking bytes or string checking as   *
* initialization call.                 *
***************************************/
int fullReadIoBuffer(IoBuffer *buffer);
/***************************************
* Sometimes you have to check that     *
* kernel has responded.                *
***************************************/
void waitResponseIoBuffer(IoBuffer *buffer);
/***************************************
* Get next byte of the buffer.         *
***************************************/
uint8_t getIoBufferByte(IoBuffer *buffer);
/***************************************
* Check next string is in buffer.      *
* Returns 1 if str is found from the   *
* buffer. Returns 0 otherwise.         *
***************************************/
uint8_t checkIoBufferStr(IoBuffer *restrict buffer,const uint8_t *restrict str,int32_t length);
/***************************************
* Comsume the what was read without    *
* returning the string.                *
***************************************/
uint8_t consumeIoBuffer(IoBuffer *buffer);
/***************************************
* Consume the what was read with       *
* returing the copy of it.             *
***************************************/
uint8_t consumeCpyIoBuffer(IoBuffer *restrict buffer,uint8_t *str);
	/***************************************
	* Set the file descriptor without      *
	* knowing buffers insides.             *
	***************************************/
	static inline void setIoBufferFd(IoBuffer *buffer,int fd){
		buffer->constsqe->fd=fd;
	}
	/***************************************
	* Check next byte without moving       *
	* reading head.                        *
	***************************************/
	static inline uint8_t checkByteIoBuffer(const IoBuffer *buffer){
		// We just need to give byte at index forwardhead since
		// other functions move head to next byte.		
		return buffer->buffers[buffer->forwardhead];
	}

#endif /* _BUFFER_MANAGER_H_ */
