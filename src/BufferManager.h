/*********************************************************
* Module enable asyncroness IO calls when possible and   *
* uses alternating buffer pairs. Buffer                         *
*********************************************************/
#include<stdint.h>
#incldue<aio.h>

/***************************************
* Data structure to keep information   *
* about buffers there length and       *
* state.                               *
***************************************/
typedef struct IoBuffer{
	struct aiocb aio;
	uint8_t *buffer1;
	uint8_t *buffer2;
}IoBuffer;

/***************************************
* Initialize the buffer manager that   *
* aren't visiable to programmer.       *
***************************************/
int initIoBufferManager(uint32_t initialmemory,uint32_t numbuffers);
/***************************************
* Allocate new buffer to be used in IO *
* buffer.                              *
***************************************/
int allocIoBuffer(IoBuffer *buff,uint32_t bufferlength);
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
* Deallocate any memory associated     *
* with BufferManager module not        *
* visiable to the programmer.          *
***************************************/
int deinitIoBufferManager();
