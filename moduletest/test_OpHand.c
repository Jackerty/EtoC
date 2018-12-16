/**************************************************************
* This module is for testing the OpHand module. Main creates  *
* test command line arguments.                                *
**************************************************************/
#include<stdint.h>
#include<unistd.h>
#include<string.h>
#include"../src/OpHand.h"

#define HELLO "HELLO\n"

uint8_t warning(char option,void *coderdata,const char *arg){
  if(arg[0]=='a') *(uint32_t *)coderdata=0xFFFF;
  else if(arg[3]=='w') *(uint32_t *)coderdata&=~0x0002;
	return 1;
}

uint8_t poll(char option,void *coderdata,const char *arg){
	*(int32_t *)coderdata=101;
	return 1;
}

int main(){

	// Simulated arguments
  char *args[]={"-c"
							 ,"you"
		           ,"-Wall"
		           ,"-h"
		           ,"side"
		           ,"with"
		           ,"--object-rack-on"
		           ,"--rendering"
		           ,"vulkan"
		           ,"idiots"
		           ,"--Warning"
		           ,"no-waterphobia"
		           ,"-n45"
		           ,"of"
		           ,"--warhammer"
		           ,"-P"
		           ,"--io"
		           ,"BLAAH"
		           ,"--"
		           ,"Second"
		           ,0
               };

  int32_t randflags=0;
	int32_t warnflags;
	char *rendering;
	int32_t number;
	int32_t warhammer;
	int32_t pollint;
	char *io;

	const Option options[]={{0               ,.variable.p32=&randflags       ,.value.v32=0x3                     ,'c',{0,OPHAND_OR}          }
									       ,{"Warning"       ,.variable.func=warning         ,.value.coderdata=(void *)&warnflags,'W',{1,OPHAND_FUNCTION}     }
									       ,{"help"          ,.variable.str=(char**)HELLO    ,.value.v32=6                       ,'h',{0,OPHAND_PRINT}        }
									       ,{"object-rack-on",.variable.p32=&randflags       ,.value.v32=0x2                     ,0  ,{0,OPHAND_AND}          }
									       ,{"rendering"     ,.variable.str=&rendering       ,.value.v32=0                       ,0  ,{1,OPHAND_POINTER_VALUE}}
									       ,{"number"        ,.variable.p32=&number          ,.value.v32=0                       ,'n',{1,OPHAND_VALUE}        }
									       ,{"warhammer"     ,.variable.p32=&warhammer       ,.value.v32=101                     ,0  ,{0,OPHAND_VALUE}        }
									       ,{"Poll"          ,.variable.func=poll            ,.value.coderdata=(void*)&pollint   ,'P',{0,OPHAND_FUNCTION}     }
									       ,{"io"            ,.variable.str=&io              ,.value.str=HELLO                   ,'i',{0,OPHAND_POINTER_VALUE}}
	                       };


	opHand(21,args,options,9);
	char *nonoptions[]={"you","side","with","idiots","of","BLAAH",0};
	uint8_t nonoptionnum=0;
	for(char **arg=args,**comp=nonoptions;*arg&&*comp;arg++,comp++) if(strcmp(*arg,*comp)==0) nonoptionnum++;

	if(nonoptionnum==6) (void)write(STDOUT_FILENO,"0 OK\n",5);
  if(randflags==0x2) (void)write(STDOUT_FILENO,"1 OK\n",5);
  if(warnflags==0xFFFD) (void)write(STDOUT_FILENO,"2 OK\n",5);
  if(strcmp(rendering,"vulkan")==0) (void)write(STDOUT_FILENO,"3 OK\n",5);
  if(number==45) (void)write(STDOUT_FILENO,"4 OK\n",5);
  if(warhammer==101) (void)write(STDOUT_FILENO,"5 OK\n",5);
	if(pollint==101) (void)write(STDOUT_FILENO,"6 OK\n",5);
	if(strcmp(io,HELLO)==0) (void)write(STDOUT_FILENO,"7 OK\n",5);

	opHand(1,args+19,0,0);
	if(strcmp(*(args+19),"Second")==0) (void)write(STDOUT_FILENO,"8 OK\n",5);

	return 0;
}
