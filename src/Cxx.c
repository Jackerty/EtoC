/****************************************************************
* Main source file for compiling C++ to C.                      *
****************************************************************/
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<ctype.h>
#include"CxxLex.h"
#include"OpHand.h"
#include"PrintTools.h"
#include"ThreadTown.h"

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
* where given. Strings tells  *
* usage of the program and    *
* options.                    *
******************************/
#define USAGE "Usage: cxxtoc [options] sourcefile...\n" \
              "\n"\
              "Compiler for C++ to C\n" \
              "\n"\
              "Options:\n" \
              "-v, --version\t give version information\n" \
              "-h, --help   \t give usage information\n" \
              "\n"

  /*******************************************
  * Job to build syntax tree from given     *
  * file.                                    *
  *******************************************/
	void *orderGenSyntaxTree(void* param){
		// Cast the parameter to actual parameters.
		char *file=param;

		// File description
    int fd;

		// Check that file exists by opening file descriptor.
		if((fd=open(file,O_RDONLY))>-1){

			// Call the generator.
			;

			// We are done reading the file so close
			close(fd);

			// Add next job to list.
			;

			//REMOVE ME!!!!!!!!
			signalThreadTownToStop();
			//REMOVE ME!!!!!!!!

		}
		else{
			// Note that strerror comes from read-only memory here!!
			char *errnostr=strerror(errno);
			printStrCat5(STDOUT_FILENO
			            ,"File \""
			            ,file
			            ,"\" access failed with errno: "
			            ,errnostr,"\n"
			            ,6
			            ,strlen(file)
								  ,28
								  ,strlen(errnostr)
								  ,1
								  );
			signalThreadTownToStop();
		}

		return 0;
	}
	/*******************************************
	* Handles arguments and start compilation  *
	* to given folder or C++ source files.     *
	*                                          *
	* Operands are considered as source files. *
	* Every single character option has long   *
	* version but not all long options have    *
	* single.                                  *
	* Options:                                 *
	*  --version or -v prints version number.  *
	*  --help or -h prints usage.              *
	*******************************************/
	int main(int argn,char **args){
		//*** VARIABLES ***//

		//*** INITIALIZATION ***//
		//**LOAD CONFIGURATION **//
		// TODO: Configuration file!

		//** HANDLE ARGUMENTS **//
    const Option argops[]={
			{"version",.variable.str=(char **)VERSION_OP_STRING,.value.v32=sizeof(VERSION_OP_STRING),'v',{0,OPHAND_PRINT}},
			{"help"   ,.variable.str=(char **)USAGE            ,.value.v32=sizeof(USAGE)            ,'h',{0,OPHAND_PRINT}}
    };
    // Note that first argument from the main is the call path!
    // There for we have one less argument and pointer is moved
    // one a head.
		if(argn>1){
			if(opHand(argn-1,args+1,argops,sizeof(argops)/sizeof(Option))){

				//** BUILD TOWN FOR THREAD WORKERS **//
				{
					uint32_t cpucount=sysconf(_SC_NPROCESSORS_ONLN);
					buildThreadTown(cpucount);
				}

				// Check that files are accessable and
				// push first list of task to queue.
				{
					ThreadTownJob *initjob;
					// By this initilization we don't have specially handle last
					// job when we call the addThreadTownJobUnsafe.
					ThreadTownJob *lastjob=(ThreadTownJob*)&initjob;
					for(char **file=args+1;*file;file++){
						lastjob->next=malloc(sizeof(ThreadTownJob));
						lastjob=lastjob->next;

						lastjob->func=orderGenSyntaxTree;
						lastjob->param=*file;
					}

					addThreadTownJobUnsafe(initjob,lastjob);
				}

				void *callerreturn=populateThreadTown();
				void **restofresults=burnThreadTown();

				//*** HANDLE RETURN VALUES ***//

				// We need to free restofresult array.
				free(restofresults);

			}
		}
		else printconst(STDOUT_FILENO,USAGE);

		return 0;
	}
