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
#include"OpHand.h"
#include"PrintTools.h"
#include"ThreadTown.h"
#include"Hash.h"
#include"CxxLex.h"

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
#define USAGE "Usage: cxxtoc {info-options|[compiler-options] files}...\n" \
              "\n"\
              "Compiler for C++ to C.\n" \
              "\n"\
              "Info-options:\n" \
              "-v, --version\t print version information and stop execution.\n" \
              "-h, --help   \t print usage information and stop execution.\n" \
              "Compiler-options:\n" \
              "-j n, --thread-count n \t manually set number of threads compilation is done." \
              "\n"

/******************************
* Hash table to checking that *
* files given aren't already  *
* processed.                  *
******************************/
HashTable FileTable;

  /*******************************************
  * Job to build syntax tree from given      *
  * file.                                    *
  *******************************************/
	void *orderGenSyntaxTree(void* param){
		// Cast the parameter to actual parameters.
		char *file=param;

		// Check that file isn't opened yet
		// by checking hashmap entry doesn't exist.
		// Note that file name is directly used what is given
		// from commandline so file.cpp and ../folder/file.cpp
		// could be treated as different files.
		if(!getHashTableEntry(&FileTable,file)){
			HashEntry *entry=malloc(sizeof(HashEntry));
			entry->key=file;
			addHashTableEntry(&FileTable,entry);
		}
		// File description.
    int fd;

		// Check that file exists by opening file descriptor.
		if((fd=open(file,O_RDONLY))>-1){
			posix_fadvise(fd,0,0,POSIX_FADV_SEQUENTIAL);

			// Call the generator.
			IoBuffer buffer;

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
	* Options: Look at USAGE macro!            *
	*******************************************/
	int main(int argn,char **args){
		//*** VARIABLES ***//
		uint32_t threadnum=0;

		//*** INITIALIZATION ***//
		//**LOAD CONFIGURATION **//
		// TODO: Configuration file!

		//** HANDLE ARGUMENTS **//
    const Option argops[]={
			{"version",.variable.str=(char **)VERSION_OP_STRING,.value.v32=sizeof(VERSION_OP_STRING),'v',{0,1,OPHAND_PRINT}},
			{"help"   ,.variable.str=(char **)USAGE            ,.value.v32=sizeof(USAGE)            ,'h',{0,1,OPHAND_PRINT}},
			{"thread-count",.variable.p32=(int32_t*)&threadnum,.value.v32=0,'j',{1,0,OPHAND_VALUE}}
    };
    // Note that first argument from the main is the call path!
    // There for we have one less argument and pointer is moved
    // one a head.
		if(argn>1){
			OpHandReturn opresult=opHand(argn-1,args+1,argops,sizeof(argops)/sizeof(Option));
			if(opresult==OPHAND_PROCESSING_DONE){

				// If args+1 doesn't point to non-option there is no source files
				// to handle.
				if(*(args+1)){

					//** BUILD TOWN FOR THREAD WORKERS **//
					if(!threadnum){
						uint32_t cpucount=sysconf(_SC_NPROCESSORS_ONLN);
						threadnum=cpucount;
					}
					buildThreadTown(threadnum);

					// Check that files are accessable and
					// push first list of task to queue.
					uint32_t filecount=0;
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

							filecount++;
						}

						addThreadTownJobUnsafe(initjob,lastjob);
					}

					//** INITIALIZE REST OF THE SYSTEMS **//
					// Initialize the BufferManager.
					// Each thread should have it's own IO buffer
					// so that every thread is working on mostly on
					// it's own file.
					// Initialize buffer memory size per buffer
					// to 4096 since most harddisk have that as block size
					// or something that divisions 4096.
					if(initIoBufferManager(4096,threadnum)==0){

						// Initialize hash table keep track source files.
						// filecount is doubled as initialize size of the
						// table because headers aren't meant to be listed.
						if(initHashTable(&FileTable,filecount*2)==0){
							
							//** START COMPILING THREADS **//
							void *callerreturn=populateThreadTown();
							void **restofresults=burnThreadTown();

							//*** HANDLE RETURN VALUES ***//
		
							// We need to free restofresult array.
							free(restofresults);
							destroyHashTable(&FileTable);
						}
						else printconst(STDERR_FILENO,"main.c | initHashTable | error!\n");
						deinitIoBufferManager();
					}
					else printconst(STDERR_FILENO,"main.c | IO buffer | error!\n");
				}
				else{
					printconst(STDOUT_FILENO,"No source files given!\n");
					return 1;
				}
			}
			else if(opresult==OPHAND_UNKNOW_OPTION){
				printconst(STDERR_FILENO,"Unkown option encountered!\n");
				return 2;
			}
			else if(opresult==OPHAND_NO_ARGUMENT){
				printconst(STDERR_FILENO,"Option that has argument given but argument for it is not given?\nPlease check input correctness.\n");
				return 3;
			}
			// If opresult equals OPHAND_PROCESSING_STOPPED let
			// processing fall through return 0 since any options
			// having stopping does not need processing.
		}
		else printconst(STDOUT_FILENO,USAGE);

		return 0;
	}
