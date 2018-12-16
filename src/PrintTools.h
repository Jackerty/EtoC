/***************************************************
* Module gives easy to functions for printing more *
* complicated output. Meant to be alternative to   *
* printf and other formatting bullshit             *
***************************************************/
#ifndef _PRINT_TOOLS_H_
#define _PRINT_TOOLS_H_

/**************************************************
* Concatenates the two strings and prints the     *
* result to given file descriptor. Returns amount *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat(const int fd,char *str1,char *str2,int str1len,int str2len);

#endif /* _PRINT_TOOLS_H_ */
