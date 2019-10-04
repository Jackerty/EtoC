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
	uint8_t buffer1[4096];
	uint8_t buffer2[4096];
	int fd;
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
int initIoBufferManager();
/***************************************
* Deallocate any memory associated     *
* with BufferManager.                  *
***************************************/
int deinitIoBufferManager();
/***************************************
* Allocate new buffer to be used in IO *
* buffer.                              *
***************************************/
int allocIoBuffer(IoBuffer *buff,size_t bufferlength);
/***************************************
* Change file desciption of the        *
* buffer.                              *
***************************************/
inline void changeFdIoBuffer(IoBuffer *buff,int fd){
	buff->fd=fd;
}
/***************************************
* Get next byte from the buffer.       *
***************************************/
uint8_t getIoBufferByte(IoBuffer *buffer);
/***************************************
* Get next 4 byte integer from the     *
* buffer.                              *
***************************************/
uint32_t getIoBuffer4Byte(IoBuffer *buffer);
/***************************************
* Get next 4 byte integer from the     *
* buffer.                              *
***************************************/
uint64_t getIoBuffer8Byte(IoBuffer *buffer);

#endif /* _BUFFER_MANAGER_H_ */
