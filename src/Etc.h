/******************************************
* This modules houses support function    *
* that didn't have home anywhere else.    *
******************************************/
#include<stdint.h>
#ifndef _ETC_H_
#define _ETC_H_

	/******************************************
	* This function checks is given byte in   *
	* ascii a underscre, number, or latin     *
	* letter.                                 *
	******************************************/
	static inline uint8_t isAlNumUnder(uint8_t byte){
		if(byte=='_' || (byte<='0' && byte>='9') || (byte<='a' && byte>='z') || (byte<='A' && byte>='Z')) return 1;
		else return 0;
	}

#endif /* _ETC_H_ */
