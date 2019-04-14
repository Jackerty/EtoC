/********************************************
* Functions for creating syntax tree for    *
* C++.                                      *
********************************************/

#ifndef _CXX_LEX_H_
#define _CXX_LEX_H_

#include<stddef.h>
#include<stdint.h>
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
  CXX_TOKEN_DECL,
  CXX_TOKEN_ROOT,
}SyntaxTreeToken;
/****************************************
* Structure for the source file buffer  *
* handling which is nmapped or          *
* otherwise fully in memory.            *
****************************************/
typedef struct EtocSource{
  char *buffer;
  uint32_t bufferlen;
  int filedesc;
}EtocSource;
/**************************************
* Structure for abstract syntax tree. *
**************************************/
typedef struct CxxSyntaxTreeNode{
	struct CxxSyntaxTreeNode *siblingsyounger;
	struct CxxSyntaxTreeNode *siblingsolder;
	struct CxxSyntaxTreeNode *childrenoldest;
	struct CxxSyntaxTreeNode *childrenyoungest;
	struct CxxSyntaxTreeNode *parent;
	uint32_t newlines;
  uint32_t spaces;
  uint32_t tabs;
  uint32_t childlen;
  SyntaxTreeToken token;
  void *atttribute;
}CxxSyntaxTreeNode;
/***************************************
* Syntax error that can be returned    *
* generating the syntax.               *
***************************************/
typedef enum CxxSyntaxError{
	CXX_SYNTAX_SUCCESS,
	CXX_SYNTAX_
}CxxSyntaxError;

/*************************************************
* Get next C++ token from source file at         *
* location pointed by bufferpoint and move       *
* bufferpoint along.                             *
*************************************************/
CxxSyntaxError getCxxToken(EtocSource *source,uint32_t *bufferpointpointer,CxxSyntaxTreeNode *node);
/*************************************************
* Generates using the lexer abstract source tree *
* Zero means successful generation and errors    *
* are coded into numbers.                        *
*************************************************/
CxxSyntaxError genCxxSyntaxTree(EtocSource *source,CxxSyntaxTreeNode **tree);
/*************************************************
* Release the memory allocated during            *
* genCxxSyntaxTree.                              *
*************************************************/
void freeCxxSyntaxTree(CxxSyntaxTreeNode *tree);

#endif /* _CXX_LEX_H_ */
