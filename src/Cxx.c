/****************************************************************
* Main source file for compiling C++ to C.                      *
****************************************************************/
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<string.h>
#include"CxxLex.h"
#include"OpHand.h"
#include"PrintTools.h"

/******************************
* String that is version of   *
* the program.                *
******************************/
#define VERSION "0.0"
/******************************
* String returned by version  *
* option.                     *
******************************/
#define VERSION_OP_STRING "Version: " VERSION "\n"
/******************************
* String returned by help     *
* option and if no arguments  *
* where given.                *
******************************/
#define USAGE "Usage: "

/*******************************
* Structure for linked listing *
* EtocSource structure.        *
*******************************/
typedef struct EtocSourceLink{
	struct EtocSourceLink *next;
	EtocSource source;
}EtocSourceLink;

	/*******************************************
	* Handles arguments and start compilation  *
	* to given folder or C++ source files.     *
	*                                          *
	* Non option are considered as file names. *
	* Every single character option has long   *
	* version but not all long options have    *
	* single.                                  *
	* Options:                                 *
	*  --version or -v prints version number.  *
	*  --help or -h prints usage.              *
	*******************************************/
	int main(int argn,char **args){
		//*** VARIABLES ***//
		EtocSourceLink *sourcesfirst;
		EtocSourceLink *sourceslast=(EtocSourceLink*)&sourcesfirst;

		//*** INITIALIZATION ***//
		//**LOAD CONFIGURATION **//
		// TODO: Configuration file!

		//** HANDLE ARGUMENTS **//
    const Option argops[]={
			{"version",.variable.str=(char **)VERSION_OP_STRING,.value.v32=sizeof(VERSION_OP_STRING),'v',{0,OPHAND_PRINT}},
			{"help"   ,.variable.str=(char **)USAGE            ,.value.v32=sizeof(USAGE)            ,'h',{0,OPHAND_PRINT}}
    };
    // Note that first argument from the main is the call path!
    // TODO: If help or version was given there is no need to tell usage
    //       however if nether is given and files are not given usage
    //       should be shown anyway!
		if(opHand(argn-1,args+1,argops,sizeof(argops)/sizeof(Option))){

			//*** PARSE THE FILES ***//
			for(char **arg=args+1;*arg;arg++){
				int32_t filedesc=open(*arg,O_RDONLY);
				if(filedesc>-1){

					// Allocate new source file structure to the linked list.
					sourceslast->next=malloc(sizeof(EtocSourceLink));
          sourceslast=sourceslast->next;

          // Store the file descriptor
          sourceslast->source.filedesc=filedesc;

					// Query size of the file so that we can nmap it as whole
					// and do know when our file ends.
					struct stat filestats;
					fstat(filedesc,&filestats);

					sourceslast->source.bufferlen=filestats.st_blksize;
					sourceslast->source.buffer=mmap(0,sourceslast->source.bufferlen,PROT_READ,MAP_PRIVATE,filedesc,0);
					sourceslast->source.bufferpoint=0;

          CxxAbstractSyntaxTreeNode *tree=0;
					genCxxSyntaxTree(&sourceslast->source,&tree);
          freeCxxSyntaxTree(tree);

				}
				else{
						(void)printStrCat(STDERR_FILENO,*arg," opening failed!\n",strlen(*arg),17);
						break;
				}
			}
			// Mark current ending of source files linked list to null pointer
			// to make sure ending is found.
			sourceslast->next=0;

			//*** EXIT ***//
			// Make sure nmaps and files are freed.
			while(sourcesfirst){
				munmap(sourcesfirst->source.buffer,sourcesfirst->source.bufferlen);
				close(sourcesfirst->source.filedesc);
				EtocSourceLink *temp=sourcesfirst->next;
				free(sourcesfirst);
				sourcesfirst=temp;
			}
		}
		else (void)write(STDOUT_FILENO,USAGE,sizeof(USAGE));
		return 0;
	}
