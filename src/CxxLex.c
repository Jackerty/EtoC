/***************************************
* See CxxLex.h for module descriotion. *
***************************************/
#include<stdint.h>
#include<stdlib.h>
#include<ctype.h>
#include"BufferManager.h"
#include"CxxLex.h"
#include"Etc.h"

/****************************************************************
* Constant array for ends of the keywords in C++. Maybe little  *
* less memory usage since no null ending. Also sizeof can be    *
* used to get length of the rest of the structure.              *
****************************************************************/
static const uint8_t EndStruct[]={'u','c','t'}; 

	/****************************************************************
	* Allocate C++ syntax tree node.                                *
	****************************************************************/
	static CxxSyntaxTreeNode *allocCxxNode(){
		CxxSyntaxTreeNode *node=malloc(sizeof(CxxSyntaxTreeNode));
		node->childlen=0;
		// This may not be evident but putting memory location of the oldest child
		// to be youngest child makes so that pointer to new yougest child can be
		// added without if else clause checking is yougestchild null.
		// Negative for this in double linked list system is that memory location of
		// pointer to oldest child is a ending marker when going through the children backwards.
		node->childrenoldest=0;
		node->childrenyoungest=(CxxSyntaxTreeNode*)&node->childrenoldest;
		node->siblingsyounger=0;
		return node;
	}
	/****************************************************************
	* Allocate new child for the given parent and return the child. *
	* NOTE: DOESN'T CHECK PARENT'S NULLNESS!.                       *
	****************************************************************/
	static CxxSyntaxTreeNode *addCxxChild(CxxSyntaxTreeNode *parent){

		// Allocate the new child for the parent.
		CxxSyntaxTreeNode *newchild=allocCxxNode();

		// Add child to parents linked list.
		parent->childrenyoungest->siblingsyounger=newchild;
		newchild->siblingsolder=parent->childrenyoungest;
		parent->childlen++;
		parent->childrenyoungest=newchild;
		// Link to parent
		newchild->parent=parent;
		// Information set up of child.

		return newchild;
	}
	/****************************************************************
	* Maps token to string names.                                   *
	* This function is mostle for debugging.                        *
	****************************************************************/
	char *getTokenStr(SyntaxTreeToken token){
		switch(token){
			case CXX_TOKEN_PREPROCESS_EMPTY    : return "CXX_TOKEN_PREPROCESS_EMPTY";
			case CXX_TOKEN_PREPROCESS_INCLUDE  : return "CXX_TOKEN_PREPROCESS_INCLUDE";
			case CXX_TOKEN_PREPROCESS_DEFINTION: return "CXX_TOKEN_PREPROCESS_DEFINTION";
			case CXX_TOKEN_PREPROCESS_IF       : return "CXX_TOKEN_PREPROCESS_IF";
			case CXX_TOKEN_PREPROCESS_IFDEF    : return "CXX_TOKEN_PREPROCESS_IFDEF";
			case CXX_TOKEN_PREPROCESS_ELSE     : return "CXX_TOKEN_PREPROCESS_ELSE";
			case CXX_TOKEN_PREPROCESS_ENDIF    : return "CXX_TOKEN_PREPROCESS_ENDIF";
			case CXX_TOKEN_PREPROCESS_PRAGMA   : return "CXX_TOKEN_PREPROCESS_PRAGMA";
			case CXX_TOKEN_PREPROCESS_WARNING  : return "CXX_TOKEN_PREPROCESS_WARNING";
			case CXX_TOKEN_PREPROCESS_ERROR    : return "CXX_TOKEN_PREPROCESS_ERROR";
			case CXX_TOKEN_PREPROCESS_MESSAGE  : return "CXX_TOKEN_PREPROCESS_MESSAGE";
			case CXX_TOKEN_PREPROCESS_LINE     : return "CXX_TOKEN_PREPROCESS_LINE";
			case CXX_TOKEN_COMMENT_LINE        : return "CXX_TOKEN_COMMENT_LINE"; 
			case CXX_TOKEN_COMMENT_BLOCK       : return "CXX_TOKEN_COMMENT_BLOCK";
			case CXX_TOKEN_IDENTIDIER          : return "CXX_TOKEN_IDENTIDIER";
			case CXX_TOKEN_ROOT                : return "CXX_TOKEN_ROOT";
			case CXX_TOKEN_STRUCT              : return "CXX_TOKEN_STRUCT";
			default                            : return "unknown";
		}
	}
	/****************
	* See Cxxlex.h  *
	****************/
	CxxSyntaxError getCxxToken(IoBuffer *buffer,CxxSyntaxTreeNode *node){
		CxxSyntaxError error=CXX_SYNTAX_SUCCESS;
		uint8_t byte;
		
		jmp_STATISTIC_OR_EMPTY:
		switch(getIoBufferByte(buffer)){
			// Collect newlines, space, and tabs
			// before next
			case '\n':
				node->newlines++;
				goto jmp_STATISTIC_OR_EMPTY;
			case  ' ':
				node->spaces++;
				goto jmp_STATISTIC_OR_EMPTY;
			case '\t':
				node->tabs++;
				goto jmp_STATISTIC_OR_EMPTY;
			case '\r':
				goto jmp_STATISTIC_OR_EMPTY;
			
			// Preprocessing symbol.
			case '#':
				// Loop through white-space.
				while((byte=getIoBufferByte(buffer))==' ' && byte=='\t');
				switch((byte=getIoBufferByte(buffer))){
					// newline means empty
					case '\n':
						node->token=CXX_TOKEN_PREPROCESS_EMPTY;
						goto jmp_WAS_IDENTIFIER;
					// Definition
					case 'd':
						node->token=CXX_TOKEN_PREPROCESS_DEFINTION;
						goto jmp_WAS_IDENTIFIER;
					// Ether endif, else, or error
					case 'e':
						goto jmp_WAS_IDENTIFIER;
					// Ether include, if, or ifdef
					case 'i':
						if((byte=getIoBufferByte(buffer))=='f'){
							// Ether if, or ifdef
							if((byte=getIoBufferByte(buffer))=='d'){
								;
							}
							else if(byte==' ' && byte=='\t'){
								node->token=CXX_TOKEN_PREPROCESS_IF;
							}
						}
						else if(byte=='n'){
							;
						}
						else ;
						goto jmp_WAS_IDENTIFIER;
				}
				goto jmp_WAS_IDENTIFIER;
			case 's':
				// Ether short, sizeof, static, static_assert, static_cast, struct,
				//       switch, synchronized
				switch((byte=getIoBufferByte(buffer))){
					case 't':
						// Ether static_assert, static_cast, or struct
						if((byte=getIoBufferByte(buffer))=='r'){
							if(checkIoBufferStr(buffer,EndStruct,sizeof(EndStruct))==0) goto jmp_WAS_IDENTIFIER;
							node->token=CXX_TOKEN_STRUCT;
							break;
						}
						goto jmp_WAS_IDENTIFIER;
				}
				goto jmp_WAS_IDENTIFIER;
			// No more input.
			case 0:
				return CXX_NO_MORE_INPUT;
			// Default here means identifier not a keyword.
			// We have jmp in it so that if we have 
			// identifier that starts with keywords (or 
			// lookalike) we can jump in.
			default:
			jmp_WAS_IDENTIFIER:
			node->token=CXX_TOKEN_IDENTIDIER;
			while(byte>0 && isAlNumUnder(byte)) byte=getIoBufferByte(buffer);
			if(byte==0) return CXX_MORE_INPUT_ERROR;
			else{
				consumeCpyIoBuffer(buffer,0);
				return CXX_SYNTAX_SUCCESS;
			}
		}
		consumeIoBuffer(buffer);
		return CXX_SYNTAX_SUCCESS;
	}
	/****************
	* See Cxxlex.h  *
	****************/
	CxxSyntaxError genCxxSyntaxTree(IoBuffer *buffer,CxxSyntaxTreeNode **trunk){
		
		CxxSyntaxError result=CXX_SYNTAX_SUCCESS;
		if(fullReadIoBuffer(buffer)==0){
		
			// If error happens result will be changed.
			// This is returned matter what.
			
			// Allocate root node.
			*trunk=malloc(sizeof(CxxSyntaxTreeNode));
			(*trunk)->siblingsolder=0;
			(*trunk)->siblingsyounger=0;
			(*trunk)->parent=0;
			(*trunk)->childrenoldest=0;
			(*trunk)->childrenyoungest=(CxxSyntaxTreeNode*)&(*trunk)->childrenoldest;
			(*trunk)->childlen=0;
			(*trunk)->token=CXX_TOKEN_ROOT;
			(*trunk)->spaces=0;
			(*trunk)->tabs=0;
			(*trunk)->newlines=0;
			
			// Use this variable to move along tree.
			CxxSyntaxTreeNode *iternode=*trunk;
			CxxSyntaxTreeNode *temp1=allocCxxNode();
			// Make sure that fullread has been done in kernel side.
			waitResponseIoBuffer(buffer);
			if(buffer->length>0){
				while(getCxxToken(buffer,temp1)==CXX_SYNTAX_SUCCESS){
				
					printf("%s\n",getTokenStr(temp1->token));
					// We will disable case warning not all enumeration have
					// case. This to keep gcc output be less cluttered.
					#pragma GCC diagnostic push
					#pragma GCC diagnostic ignored "-Wswitch"

					switch(temp1->token){
					case CXX_TOKEN_IDENTIDIER:
						break;
					case CXX_TOKEN_COMMENT_BLOCK:
						break;
					}

					#pragma GCC diagnostic pop

				}
			}
			else result=CXX_SYNTAX_BUFFER_ERROR;
		}
		else result=CXX_SYNTAX_BUFFER_ERROR;

		return result;
	}
	/****************
	* See Cxxlex.h  *
	****************/
	void freeCxxSyntaxTree(CxxSyntaxTreeNode *tree){
		CxxSyntaxTreeNode *ite=tree;
		while(ite){
			if(ite->childrenoldest){
				// To prevent infinite loop
				// we have to mark that there
				// is no children
				CxxSyntaxTreeNode *nochild=ite;
				ite=ite->childrenoldest;
				nochild->childrenoldest=0;
			}
			else if(ite->siblingsyounger){
				CxxSyntaxTreeNode *remove=ite;
				ite=ite->siblingsyounger;
				free(remove);
			}
			else{
				CxxSyntaxTreeNode *remove=ite;
				ite=ite->parent;
				free(remove);
			}
		}
	}
