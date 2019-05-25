/***************************************************
* Module gives easy to functions for printing more *
* complicated output. Meant to be alternative to   *
* printf and other formatting bullshit             *
***************************************************/
#ifndef _PRINT_TOOLS_H_
#define _PRINT_TOOLS_H_

/**************************************************
* Macro for writing that uses sizeof to detect    *
* length of the string.                           *
**************************************************/
#define printconst(D,S) (void)write(D,S,sizeof(S)-1)
/**************************************************
* Macro for writing that uses strlen to detect    *
* length of the string.                           *
**************************************************/
#define print(D,S) (void)write(D,S,strlen(S))
/**************************************************
* Concatenates two strings and prints the result  *
* to given file descriptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat(const int fd, const char *str1,const char *str2,const int str1len,const int str2len);
/**************************************************
* Concatenates three strings and prints the       *
* result to given file desctiptor. Returns amount *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat3(const int fd,const char *str1,const char *str2,const char *str3,const int str1len,const int str2len,const int str3len);
/**************************************************
* Concatenates four strings and prints the result *
* to given file desctiptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat4(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const int str1len,const int str2len,const int str3len,const int str4len);
/**************************************************
* Concatenates four strings and prints the result *
* to given file desctiptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat5(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const char *str5,const int str1len,const int str2len,const int str3len,const int str4len,const int str5len);

#endif /* _PRINT_TOOLS_H_ */
