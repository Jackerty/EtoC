#ifndef _CXX_LEX_H_
#define _CXX_LEX_H_

#include<stddef.h>
#include<stdint.h>

/****************************************
* Structure for the source file buffer  *
* handling which is nmapped or          *
* otherwise fully in memory.            *
****************************************/
typedef struct EtocSource{
  char *buffer;
  int bufferlen;
  int bufferpoint;
  int filedesc;
}EtocSource;
/****************************************
* Token types.                          *
****************************************/
typedef enum CxxTokenType{
	CXX_TOKEN_COMMENT, // line or block
  CXX_TOKEN_INCLUDE,
  CXX_TOKEN_PREPROCESS_DEFINTION,
  CXX_TOKEN_PREPROCESS_IF,
  CXX_TOKEN_PREPROCESS_IFDEF,
  CXX_TOKEN_PREPROCESS_ELSE,
  CXX_TOKEN_PREPROCESS_ENDIF,
  CXX_TOKEN_PRAGMA,
  CXX_TOKEN_BRACES,      // { or }
  CXX_TOKEN_PARENTHES,   // ( or )
  CXX_TOKEN_BRACKETS,    // [ or ]
  CXX_TOKEN_COMMA,       // ,
  CXX_TOKEN_NUMBER_BIN,
  CXX_TOKEN_NUMBER_OCTAL,
  CXX_TOKEN_NUMBER_DEC,
  CXX_TOKEN_NUMBER_HEX,
  CXX_TOKEN_CHARACTER,   // 'x'
  CXX_TOKEN_STRING,		   // "HELLO world"
  CXX_TOKEN_IDENTIFIER,
  CXX_TOKEN_DECLERATION,
  CXX_TOKEN_OPERATOR,
  CXX_TOKEN_IF,
  CXX_TOKEN_ELSE,
  CXX_TOKEN_ELSEIF,
  CXX_TOKEN_FUNCTION,
  CXX_TOKEN_STRUCTURE,
  CXX_TOKEN_CLASS,
  CXX_TOKEN_STATIC,
  CXX_TOKEN_TYPEDEF,
  CXX_TOKEN_ALIGNAS,
  CXX_TOKEN_ALIGNOF,
  CXX_TOKEN_AND,
  CXX_TOKEN_AND_EQ,
  CXX_TOKEN_ASM,
  CXX_TOKEN_AUTO,
  CXX_TOKEN_BOOL,
  CXX_TOKEN_CHAR,
  CXX_TOKEN_INLINE,
  CXX_TOKEN_INT,
  CXX_TOKEN_LONG,
  CXX_TOKEN_PRIVATE,
  CXX_TOKEN_PROTECTED,
  CXX_TOKEN_PUBLIC,
  CXX_TOKEN_THIS,
  CXX_TOKEN_STATEMENT_END // ;
}CxxTokenType;
/***************************************
* Syntax error that can be returned    *
* generating the syntax.               *
***************************************/
typedef enum CxxSyntaxError{
	CXX_SYNTAX_GOOD,
	CXX_SYNTAX_
}CxxSyntaxError;
/***************************************
* Token returned by lexer.             *
***************************************/
typedef struct CxxToken{
  char *characters;
  int len;
  uint16_t numnewlines;
  CxxTokenType type;
}CxxToken;
/**************************************
* Symbol table handler.               *
**************************************/
typedef struct SymbolTable{
	char *name;
	struct CxxAbstractSyntaxTreeNode *startnode;
}SymbolTable;
/**************************************
* Structure for abstract syntax tree. *
**************************************/
typedef struct CxxAbstractSyntaxTreeNode{
	struct CxxAbstractSyntaxTreeNode *siblingsyounger;
	struct CxxAbstractSyntaxTreeNode *siblingsolder;
	struct CxxAbstractSyntaxTreeNode *childrenoldest;
	struct CxxAbstractSyntaxTreeNode *childrenyoungest;
	struct CxxAbstractSyntaxTreeNode *parent;
	CxxToken *token;
	SymbolTable symbols;
  uint32_t childlen;
}CxxAbstractSyntaxTreeNode;
/*************************************************
* Get next element of source.                    *
*************************************************/
CxxToken *cxxLexGetNext(EtocSource *file);
/*************************************************
* Generates using the lexer abstract source tree *
* Zero means successful generation and errors    *
* are coded into numbers.                        *
*************************************************/
CxxSyntaxError genCxxSyntaxTree(EtocSource *source,CxxAbstractSyntaxTreeNode **tree);
/*************************************************
* Release the memory allocated during            *
* genCxxSyntaxTree.                              *
*************************************************/
void freeCxxSyntaxTree(CxxAbstractSyntaxTreeNode *tree);
/*************************************************
* Token type to string of the type.              *
*************************************************/
char *tokenTypeToStr(const CxxToken *token,uint32_t *len);

#endif /* _CXX_LEX_H_ */
