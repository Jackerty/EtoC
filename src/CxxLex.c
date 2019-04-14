/***************************************
* See CxxLex.h for module descriotion. *
***************************************/
#include<stdint.h>
#include<stdlib.h>
#include"CxxLex.h"

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
		node->childrenyoungest=(CxxSyntaxTreeNode*)&newchild->childrenoldest;
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
	/****************
	* See cxxlex.h  *
	****************/
	CxxSyntaxError getCxxToken(EtocSource *source,uint32_t *bufferpointpointer,CxxSyntaxTreeNode *node){

		// Capture buffer pointer here for easier writting.
		uint32_t bufferpoint=*bufferpointpointer;

    while(bufferpoint<source->bufferlen){

			switch(source->buffer[bufferpoint]){
				// Collect newlines, space, and tabs
				// before next
				case '\n':
					node->newlines++;
					continue;
				case  ' ':
					node->spaces++;
					continue;
				case '\t':
					node->tabs++;
					continue;
				case '\r':
					continue;

				// Preprocessing symbol.
				case '#':
					// Loop through whitespace.
					while(source->buffer[bufferpoint++]==' ' && source->buffer[bufferpoint]=='\t');
					switch(source->buffer[bufferpoint]){
						// newline means empty
						case '\n':
							node->token=CXX_TOKEN_PREPROCESS_EMPTY;
							break;
						// Definition
						case 'd':
							break;
						// Ether endif, else, or error
						case 'e':
							break;
						// Ether include, if, or ifdef
						case 'i':
							break;
					}
					break;

				//
			}

    }

    // Store buffer pointer back to memory.
		*bufferpointpointer=bufferpoint;

		return CXX_SYNTAX_SUCCESS;
	}
	/****************
  * See cxxlex.h  *
  ****************/
	CxxSyntaxError genCxxSyntaxTree(EtocSource *source,CxxSyntaxTreeNode **tree){
		// If error happens result will be changed.
		// This is returned matter what.
		CxxSyntaxError result=CXX_SYNTAX_SUCCESS;

		// While reading the source code keep in mind the location of
		// buffer.
    uint32_t bufferpoint=0;

    // Allocate root node.
    *tree=malloc(sizeof(CxxSyntaxTreeNode));
    (*tree)->siblingsolder=0;
    (*tree)->siblingsyounger=0;
    (*tree)->parent=0;
    (*tree)->childrenoldest=0;
    (*tree)->childrenyoungest=(CxxSyntaxTreeNode*)&(*tree)->childrenoldest;
    (*tree)->childlen=0;
    (*tree)->token=CXX_TOKEN_ROOT;
    (*tree)->spaces=0;
    (*tree)->tabs=0;
    (*tree)->newlines=0;

    // Use this variable to move along tree.
    CxxSyntaxTreeNode *iternode=*tree;
		do{

			CxxSyntaxTreeNode *temp1=allocCxxNode();

			switch(temp1.token){
			case CXX_TOKEN:
			case :
			}

		}while(bufferpoint<source->bufferlen);


		return result;
	}
	/****************
	* See cxxlex.h  *
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
