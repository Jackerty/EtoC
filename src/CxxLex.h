/********************************************
* Functions for creating and handling of    *
* syntax tree for C++.                      *
********************************************/
#ifndef _CXX_LEX_H_
#define _CXX_LEX_H_

#include<stdint.h>
#include"BufferManager.h"

/***************************************
* Syntax error that can be returned    *
* generating the syntax.               *
***************************************/
typedef enum CxxSyntaxError{
	CXX_SYNTAX_SUCCESS,
	CXX_NO_MORE_INPUT,
	CXX_SYNTAX_BUFFER_ERROR,
	CXX_MORE_INPUT_ERROR
}CxxSyntaxError;
/****************************************
* Syntax tree tokens.                   *
****************************************/
typedef enum SyntaxTreeToken{
	CXX_TOKEN_PREPROCESS_EMPTY,
	CXX_TOKEN_PREPROCESS_INCLUDE,
	CXX_TOKEN_PREPROCESS_DEFINTION,
	CXX_TOKEN_PREPROCESS_IF,
	CXX_TOKEN_PREPROCESS_IFDEF,
	CXX_TOKEN_PREPROCESS_ELSE,
	CXX_TOKEN_PREPROCESS_ENDIF,
	CXX_TOKEN_PREPROCESS_PRAGMA,
	CXX_TOKEN_PREPROCESS_WARNING,
	CXX_TOKEN_PREPROCESS_ERROR,
	CXX_TOKEN_PREPROCESS_MESSAGE,
	CXX_TOKEN_PREPROCESS_LINE,
	CXX_TOKEN_COMMENT_LINE,
	CXX_TOKEN_COMMENT_BLOCK,
	CXX_TOKEN_STRUCT,
	CXX_TOKEN_IDENTIDIER,
	CXX_TOKEN_ROOT
}SyntaxTreeToken;
/**************************************
* Structure for abstract syntax tree. *
**************************************/
typedef struct CxxSyntaxTreeNode{
	struct CxxSyntaxTreeNode *siblingsyounger;
	struct CxxSyntaxTreeNode *siblingsolder;
	struct CxxSyntaxTreeNode *childrenoldest;
	struct CxxSyntaxTreeNode *childrenyoungest;
	struct CxxSyntaxTreeNode *parent;
	void *attribute;
	uint32_t newlines;
	uint32_t spaces;
	uint32_t tabs;
	uint32_t childlen;
	SyntaxTreeToken token;
}CxxSyntaxTreeNode;

/********************************************
* Get next C++ token from source file at    *
* location pointed by bufferpoint and move  *
* bufferpoint along.                        *
********************************************/
CxxSyntaxError getCxxToken(IoBuffer *buffer,CxxSyntaxTreeNode *node);
/********************************************
* Get next C++ token from source file at    *
* location pointed by bufferpoint and move  *
* bufferpoint along.                        *
********************************************/
CxxSyntaxError genCxxSyntaxTree(IoBuffer *buffer,CxxSyntaxTreeNode **trunk);
/********************************************
* Release the memory allocated during       *
* genCxxSyntaxTree.                         *
********************************************/
void freeCxxSyntaxTree(CxxSyntaxTreeNode *tree);


#endif /* _CXX_LEX_H_ */
