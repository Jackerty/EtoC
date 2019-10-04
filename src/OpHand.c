/********************************************************************
* This module is general function for option handling.              *
********************************************************************/
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include"OpHand.h"

	/*********************************
	* Handles non-argument options.  *
	*********************************/
	static uint8_t switchNonArgument(const Option *option){
		switch(option->flags.type){
			case OPHAND_VALUE:
				*option->variable.p32=option->value.v32;
				break;
			case OPHAND_OR:
				*option->variable.p32|=option->value.v32;
				break;
			case OPHAND_AND:
				*option->variable.p32&=option->value.v32;
				break;
			case OPHAND_POINTER_VALUE:
				*option->variable.str=option->value.str;
				break;
			case OPHAND_PRINT:
				(void)write(STDOUT_FILENO,(char*)option->variable.str,option->value.v32);
				break;
			case OPHAND_FUNCTION:
				// Return whatever callback returns
				return option->variable.func(option->option,option->value.coderdata,0);
		}
		return !option->flags.stop;
	}
	/*********************************
	* Handles argument options.      *
	*********************************/
	static uint8_t switchArgument(char *arg,const Option *option){
		switch(option->flags.type){
			case OPHAND_VALUE:
				*option->variable.p32=atoi(arg);
				break;
			case OPHAND_POINTER_VALUE:
				*option->variable.str=arg;
				break;
			case OPHAND_FUNCTION:
				// Return whatever callback returns
				return option->variable.func(option->option,option->value.coderdata,arg);
		}
		return !option->flags.stop;
	}
	/***************
	* See OpHand.h *
	***************/
	OpHandReturn opHand(int argn,char **args,const Option *options,uint32_t optionslen){

		// Points to next location where loop would
		// put non-option arguments. Used after all
		// arguments are processed to mark end of
		// non-option arguments.
    uint32_t nonoptpoint=0;

    // This stores what option is hit after long or short option
    // is hit.
    uint32_t foundoption;

    // Simple start going through arguments and compare to options.
    // Remember that first argument is the location of the execution.
    for(uint32_t arg=0;arg<argn;arg++){
      if(args[arg][0]=='-'){
				if(args[arg][1]=='-'){
					if(args[arg][2]!='\0'){
						for(foundoption=0;foundoption<optionslen;foundoption++){
							if(options[foundoption].longoption && strcmp(args[arg]+2,options[foundoption].longoption)==0){
								// If argument is needed we know that it is
								// next string in args array.
								if(HAS_ARGUMENT(options[foundoption].flags)){
									// Check that next argument exist.
									if(++arg<argn){
										uint8_t result=switchArgument(args[arg],options+foundoption);
										if(result) goto jmp_OUTER_LOOP_CONTINUE;
										else return OPHAND_PROCESSING_STOPPED;
									}
									return OPHAND_NO_ARGUMENT;
								}
								else{
									uint8_t result=switchNonArgument(options+foundoption);
									if(result) goto jmp_OUTER_LOOP_CONTINUE;
									else return OPHAND_PROCESSING_STOPPED;
								}
							}
						}
						return OPHAND_UNKNOW_OPTION;
					}
					else{
						// Since two lines where given execution ends here.
						break;
					}
				}
				else{
					if(args[arg][1]!='\0'){
						for(uint32_t foundoption=0;foundoption<optionslen;foundoption++){
							if(args[arg][1]==options[foundoption].option){
								// If argument is needed we have to check
								// is argument start second offset or is it next argument.

								if(HAS_ARGUMENT(options[foundoption].flags)){
									if(args[arg][2]!='\0'){
											uint8_t result=switchArgument(args[arg]+2,options+foundoption);
											if(result) goto jmp_OUTER_LOOP_CONTINUE;
											else return OPHAND_PROCESSING_STOPPED;
									}
									else{
										// Check that next argument exist.
										if(++arg<argn){
											uint8_t result=switchArgument(args[arg],options+foundoption);
											if(result) goto jmp_OUTER_LOOP_CONTINUE;
											else return OPHAND_PROCESSING_STOPPED;
										}
									}
									return OPHAND_NO_ARGUMENT;
								}
								else{
									uint8_t result=switchNonArgument(options+foundoption);
									if(result) goto jmp_OUTER_LOOP_CONTINUE;
									else return OPHAND_PROCESSING_STOPPED;
								}
							}
						}
					}
					return OPHAND_UNKNOW_OPTION;
				}
      }
			else args[nonoptpoint++]=args[arg];
			jmp_OUTER_LOOP_CONTINUE:;
    }

    // Mark the ending as description wanted!
    *(args+nonoptpoint)=0;

		return OPHAND_PROCESSING_DONE;
	}

