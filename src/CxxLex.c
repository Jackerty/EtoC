/***********************************************************
* Functionality to lex C++ files (and C).                  *
***********************************************************/
#include<unistd.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include"CxxLex.h"
#include"PrintTools.h"

#define STOP_CASE case  ';': \
									case '\r': \
									case '\n': \
									case  ' ': \
									case '\t': \
									case  '<': \
									case  '>': \
									case  '{': \
									case  '}': \
									case  '(': \
									case  ')': \
									case  '[': \
									case  ']': \
									case  '=': \
									case  '*':

	/*****************
	* See cxxlex.h   *
	*****************/
	CxxToken *cxxLexGetNext(EtocSource *source){

		// Pointer to element which will be returned if found.
		// Otherwise return null.
		CxxToken *element=0;
    register enum CxxTokenType type;
    register uint16_t numnewlines;
		// If source code doesn't have anything interesting
		// jump back here.
    jmp_CONTINUE_OUTHER:
    if(source->bufferpoint<source->bufferlen){
			switch(source->buffer[source->bufferpoint]){
				//** SPACE OR TAB  **//
				// Just ignore in this level
				case '\n':
					numnewlines++;
				case '\r':
				case ' ':
				case '\t':
					source->bufferpoint++;
					goto jmp_CONTINUE_OUTHER;

				//** PREPROCESSING SYMBOL **//
				case '#':
					type=CXX_TOKEN_PREPROCESS_DEFINTION;
					break;
				//** CHARACTERS **//
				case '"':
					type=CXX_TOKEN_STRING;
					break;
				case '\'':
					type=CXX_TOKEN_CHARACTER;
					break;
				//** SEPARATOR COMMA **//
				case ',':
					type=CXX_TOKEN_COMMA;
					break;
				//** COMMENT OR OPERATOR **//
				case '/':
					if(source->bufferpoint+1<source->bufferlen){
						if(source->buffer[source->bufferpoint+1]=='/' || source->buffer[source->bufferpoint+1]=='*'){
							type=CXX_TOKEN_COMMENT;
							break;
						}
					}
				case '+':
				case '-':
				case '*':
				case '>':
				case '<':
				case '&':
				case '|':
				case '=':
					type=CXX_TOKEN_OPERATOR;
					break;
				//** MARK STAMENT ENDS **//
				case ';':
					type=CXX_TOKEN_STATEMENT_END;
					break;
				case '[':
				case ']':
					type=CXX_TOKEN_BRACKETS;
					break;
				case  '(':
				case  ')':
					type=CXX_TOKEN_PARENTHES;
					break;
				case  '{':
				case  '}':
					type=CXX_TOKEN_BRACES;
					break;
				//** NUMBER CHECK **//
				// TODO: Test what ever it is faster to
				//       have this list or just normal
				//       if check. We have local variable
				//       character defined underneath.
				case '0':
					type=CXX_TOKEN_NUMBER_HEX;
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					type=CXX_TOKEN_NUMBER_DEC;
					break;
				//** KEYWORDS **//
				// List find https://en.cppreference.com/w/cpp/keyword
        //
				// Starting with 'a' can be alignas, alignof, and, and_eq, asm, auto.
				// We use
				case 'a':
					type=CXX_TOKEN_AND;
					break;
				case 'b':
					// Starting with 'b' can be bitand, bitor, bool, break.
					type=CXX_TOKEN_BOOL;
					break;
				case 'c':
					// Starting with 'c' can be case,catch,char,char8_t,char16_t,char32_t,class,
					// compl,concept,const,consteval,constexpr,const_cast,continue,co_await,
					// co_return,co_yield
					type=CXX_TOKEN_CLASS;
					break;
				case 'i':
					// Starting with 'i' can be if,inline,int.
					type=CXX_TOKEN_IF;
					break;
				case 'p':
					// Starting with 'p' can be private,protected,public
					type=CXX_TOKEN_PUBLIC;
					break;
				case 's':
					// Starting with 's' can be short signed,sizeof,static,static_assert,
					// static_cast,struct,switch,synchronized.
          type=CXX_TOKEN_STATIC;
          break;
				case 't':
					// Starting with 't' can be template, this, thread_local, throw, true, try,
					// typedef, typeid, typename.
					type=CXX_TOKEN_THIS;
					break;

				//** NORMAL TEXT **//
				default:
					type=CXX_TOKEN_IDENTIFIER;
					break;
				case '\0':
					return 0;
					break;

			}

			element=malloc(sizeof(CxxToken));
			element->type=type;
			element->characters=source->buffer+source->bufferpoint;
			element->numnewlines=numnewlines;

			register uint32_t start=source->bufferpoint;
			source->bufferpoint++;

			register char character;

			// We will disable case warning not all enumeration have
			// case. We don't have case for everything because for
			// example all preprocessing has one case because # mark.
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wswitch"

			switch(type){
				case CXX_TOKEN_AND:
					goto jmp_IT_WAS_IDENTIFIER;
				case CXX_TOKEN_BOOL:
					if(source->bufferpoint<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							// Is it bool?
							case 'o':
								if((++source->bufferpoint)+1<source->bufferlen){
									if(source->buffer[source->bufferpoint++]=='o' && source->buffer[source->bufferpoint++]=='l'){
										switch(source->buffer[source->bufferpoint]){
											STOP_CASE
												goto jmp_OUT_OF_READING_WHILE;
										}
									}
								}
								break;
						}
					}
					goto jmp_IT_WAS_IDENTIFIER;
				// Keywords:
				// case,catch,char,char8_t,char16_t,char32_t,class,compl,
				// concept,const,consteval,constexpr,const_cast,continue,
				// co_await,co_return,co_yield
				case CXX_TOKEN_CLASS:
					if(source->bufferpoint<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							// Is it char?
							case 'h':
								if((++source->bufferpoint)+1<source->bufferlen){
									if(source->buffer[source->bufferpoint]=='a' && source->buffer[++source->bufferpoint]=='r'){
										if((++source->bufferpoint)<source->bufferlen){
											switch(source->buffer[source->bufferpoint]){
												STOP_CASE
													element->type=CXX_TOKEN_CHAR;
													goto jmp_OUT_OF_READING_WHILE;
											}
										}
									}
								}
								break;
							// Is it class?
							case 'l':
								// DANGER!
								// Following constant 2 comes from resofclass size
								// minus one which doesn't exist for when 2 is needed.
								if((++source->bufferpoint)+2<source->bufferlen){
									if(source->buffer[source->bufferpoint]=='a' && source->buffer[++source->bufferpoint]=='s' && source->buffer[++source->bufferpoint]=='s'){
										if(++source->bufferpoint<source->bufferlen){
											switch(source->buffer[source->bufferpoint]){
												case  ';':
												case '\n':
												case  ' ':
												case  '>':
												case '\t':
													goto jmp_OUT_OF_READING_WHILE;
											}
										}
									}
								}
								break;
						}
					}
					goto jmp_IT_WAS_IDENTIFIER;
				// Keywords:
				// if,inline,int
				case CXX_TOKEN_IF:
					if(source->bufferpoint+1<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							case 'f':
								switch(source->buffer[++source->bufferpoint]){
									case  ';':
									case '\n':
									case  ' ':
									case '\t':
										goto jmp_OUT_OF_READING_WHILE;
								}
							case 'n':
								switch(source->buffer[++source->bufferpoint]){
									case 'l':
										break;
									case 't':
										if((++source->bufferpoint)<source->bufferlen){
											switch(source->buffer[source->bufferpoint]){
												case  ';':
												case '\n':
												case  ' ':
												case '\t':
												case  '>':
													element->type=CXX_TOKEN_INT;
													goto jmp_OUT_OF_READING_WHILE;
											}
										}
										break;
								}
						}
					}
					goto jmp_IT_WAS_IDENTIFIER;
				// short signed,sizeof,static,static_assert,
				// static_cast,struct,switch,synchronized
				case CXX_TOKEN_STATIC:
          if(source->bufferpoint<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							// Is it a static or struct?
							case 't':
                if(++source->bufferpoint<source->bufferlen){
									switch(source->buffer[source->bufferpoint]){
										// Is it struct?
										case 'r':
											if((++source->bufferpoint)+3<source->bufferlen){
												if(source->buffer[source->bufferpoint]=='u' && source->buffer[++source->bufferpoint]=='c' && source->buffer[++source->bufferpoint]=='t'){
													switch(source->buffer[++source->bufferpoint]){
														case  ';':
														case '\n':
														case  ' ':
														case '\t':
														case  '{':
															element->type=CXX_TOKEN_STRUCTURE;
															goto jmp_OUT_OF_READING_WHILE;
													}
												}
											}
											break;
										// Is is static?
										case 'a':
											break;
									}
                }

								break;
						}
          }
					goto jmp_IT_WAS_IDENTIFIER;

				// private,protected,public
				case CXX_TOKEN_PUBLIC:
					if(source->bufferpoint+5<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							case 'r':
								switch(source->buffer[++source->bufferpoint]){
									// Is it private
									case 'i':
										if(source->buffer[++source->bufferpoint]=='v' && source->buffer[++source->bufferpoint]=='a' && source->buffer[++source->bufferpoint]=='t' && source->buffer[++source->bufferpoint]=='e'){
											;
										}
										break;
									// Is it protected
									case 'o':
										;
										break;
								}
								break;
								// Is it public?
							case 'u':
								break;
						}
					}
					goto jmp_IT_WAS_IDENTIFIER;

				// template, this, thread_local, throw, true, try,
				// typedef, typeid, typename.
				case CXX_TOKEN_THIS:
					if(source->bufferpoint<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							case 'e':
								break;
							case 'h':
								break;
							case 'r':
								break;
							case 'y':
								if((++source->bufferpoint)+1<source->bufferlen && source->buffer[source->bufferpoint++]=='p' && source->buffer[source->bufferpoint]=='e'){
									if((++source->bufferpoint)<source->bufferlen){
										switch(source->buffer[source->bufferpoint]){
											case 'd':
												if((++source->bufferpoint)+1<source->bufferlen && source->buffer[source->bufferpoint++]=='e' && source->buffer[source->bufferpoint]=='f'){
													if(++source->bufferpoint<source->bufferlen){
														switch(source->buffer[source->bufferpoint]){
															case  ';':
															case '\n':
															case  ' ':
															case '\t':
															case  '{':
																element->type=CXX_TOKEN_TYPEDEF;
																goto jmp_OUT_OF_READING_WHILE;
														}
													}
												}
												break;
											case 'i':
												break;
											case 'n':
												break;
										}
									}
								}
								break;
 						}
					}
					goto jmp_IT_WAS_IDENTIFIER;
					// We jump here if it is realized that
					// what we where reading was identifier
					// rather then keyword.
					jmp_IT_WAS_IDENTIFIER:
					element->type=CXX_TOKEN_IDENTIFIER;
				// Read until ending identifier comes.
				case CXX_TOKEN_IDENTIFIER:
					while(source->bufferpoint<source->bufferlen){
						switch(source->buffer[source->bufferpoint]){
							case  ' ':
							case '\r':
							case '\n':
							case '\t':
							case  '<':
							case  '>':
							case  ',':
							case  ';':
							case  '(':
							case  ')':
							case  '[':
							case  ']':
							case  '{':
							case  '}':
							case  '=':

								goto jmp_OUT_OF_READING_WHILE;
						}
						source->bufferpoint++;
					}
					goto jmp_ERROR_EXIT;

				case CXX_TOKEN_NUMBER_HEX:
					character=source->buffer[source->bufferpoint++];
					if('0'<=character && character<='7'){
						// Octal number
						while(source->bufferpoint<source->bufferlen){
							character=source->buffer[source->bufferpoint];
							if(('0'>character || character>'7') && character!='\''){
								switch(character){
									case ' ':
									case '\n':
									case ';':
									case '\t':
										element->type=CXX_TOKEN_NUMBER_OCTAL;
										goto jmp_OUT_OF_READING_WHILE;
									default:
										goto jmp_ERROR_EXIT;
								}
							}
							source->bufferpoint++;
						}
					}
					else switch(character){
						case 'X':
						case 'x':
							while(source->bufferpoint<source->bufferlen){
								character=source->buffer[source->bufferpoint];
								// Hex number digits are 0 to 9 and a to f both upper and lower.
								// Lower and upper case is done by modulating character by 32
								// since in ascii big and lower case letters are equal in mod 32.
								if(!(('0'<=character && character<='9') || character=='\'' || ('a'%32<=character%32 && character%32<='f'%32))){
									switch(character){
										case ' ':
										case '\n':
										case ';':
										case '\t':
											goto jmp_OUT_OF_READING_WHILE;
										default:
											goto jmp_ERROR_EXIT;
									}
								}
								source->bufferpoint++;
							}
							goto jmp_ERROR_EXIT;
						case 'B':
						case 'b':
							while(source->bufferpoint<source->bufferlen){
								character=source->buffer[source->bufferpoint];
								// Binary number is just 0 and 1.
								if(('0'>character || character>'1') && character!='\''){
									switch(character){
										case ' ':
										case '\n':
										case ';':
										case '\t':
											element->type=CXX_TOKEN_NUMBER_BIN;
											goto jmp_OUT_OF_READING_WHILE;
										default:
											goto jmp_ERROR_EXIT;
									}
								}
								source->bufferpoint++;
							}
					}
          // There seem to be chance it is just zero
          // which means that we have to check for ending.
          element->type=CXX_TOKEN_NUMBER_DEC;
					goto jmp_FAST_SWITCH_CASE;

					// Handle decimal number
					case CXX_TOKEN_NUMBER_DEC:
						while(source->bufferpoint<source->bufferlen){
							character=source->buffer[source->bufferpoint];
							if(('0'>character || character>'9') && character!='\''){

								jmp_FAST_SWITCH_CASE:
								switch(character){
									STOP_CASE
										goto jmp_OUT_OF_READING_WHILE;
									default:
										goto jmp_ERROR_EXIT;
								}
							}
							source->bufferpoint++;
						}
						goto jmp_ERROR_EXIT;

				// Read until string ending comes.
				case CXX_TOKEN_STRING:
          while(source->bufferpoint<source->bufferlen){
            if(source->buffer[source->bufferpoint++]=='"') goto jmp_OUT_OF_READING_WHILE;
          }
          goto jmp_ERROR_EXIT;

				// Read until character ending comes.
				case CXX_TOKEN_CHARACTER:
          while(source->bufferpoint<source->bufferlen){
            if(source->buffer[source->bufferpoint++]=='\'') goto jmp_OUT_OF_READING_WHILE;
          }
          goto jmp_ERROR_EXIT;

				// Based upon which type of comment ether wait for line end
				// or wait */ marker.
				case CXX_TOKEN_COMMENT:
					if(source->buffer[source->bufferpoint]=='/'){
						while(++source->bufferpoint<source->bufferlen){
							if(source->buffer[source->bufferpoint]=='\n') goto jmp_OUT_OF_READING_WHILE;
						}
					}
					else{
						while(source->bufferpoint<source->bufferlen){
							character=source->buffer[source->bufferpoint++];
							while(character=='*' && source->bufferpoint<source->bufferlen){
								character=source->buffer[source->bufferpoint++];
								if(character=='/') goto jmp_OUT_OF_READING_WHILE;
							}
						}
					}
					goto jmp_ERROR_EXIT;
				case CXX_TOKEN_PREPROCESS_DEFINTION:
					// Investigate further what preprocessing command is.
					// First make sure empty space is cone through.
					while(source->bufferpoint<source->bufferlen){
						character=source->buffer[source->bufferpoint++];
						// Ask have we cleared empty spaces.
						if(character!=' ' && character!='\t'){
							if(source->bufferpoint<source->bufferlen){
								switch(character){
									case 'i':
										switch(source->buffer[source->bufferpoint++]){
											case 'n':
												if(source->bufferpoint+5<source->bufferlen){
													const char includetest[]={'c','l','u','d','e'};
													if(strncmp(source->buffer+source->bufferpoint,includetest,sizeof(includetest))==0){
														source->bufferpoint+=5;
														element->type=CXX_TOKEN_INCLUDE;
														while(source->bufferpoint<source->bufferlen
														      && ((character=source->buffer[source->bufferpoint++])==' '
														      || character=='\t')
														);
														switch(character){
															case '<':
																character='>';
															case '"':
																while(source->bufferpoint<source->bufferlen){
																	if(source->buffer[source->bufferpoint++]==character){
																		goto jmp_OUT_OF_READING_WHILE;
																	}
																}
															}
														goto jmp_ERROR_EXIT;
													}
												}
												goto jmp_ERROR_EXIT;
											case 'f':
												goto jmp_ERROR_EXIT;
										}
									case 'd':
										// Check that define is
										if(source->bufferpoint+5<source->bufferlen){
											element->len+=5;
											const char definetest[]={'e','f','i','n','e'};
											if(strncmp(source->buffer+source->bufferpoint,definetest,sizeof(definetest))==0){
												source->bufferpoint+=sizeof(definetest);
                        goto jmp_OUT_OF_READING_WHILE;
											}
										}
										goto jmp_ERROR_EXIT;
									case 'e':
										if(source->buffer[source->bufferpoint++]=='\n');
									default:
										goto jmp_ERROR_EXIT;
								}
							}
						}
					}
					goto jmp_ERROR_EXIT;
					break;
			}
			#pragma GCC diagnostic pop
			// Previous switch statement may have more nested
			// switches or loops preventing us just breaking out.
			jmp_OUT_OF_READING_WHILE:

			// It faster to calculate elements length here.
			element->len=source->bufferpoint-start;
			return element;
		}

		// Something has gone wrong while parsing!
		jmp_ERROR_EXIT:
		if(element) free(element);

		return 0;
	}
	/*****************
	* See cxxlex.h   *
	*****************/
	CxxSyntaxError genCxxSyntaxTree(EtocSource *source,CxxAbstractSyntaxTreeNode **tree){
		// Temporarily holder of tokens so that we don't call
		// malloc unneeded.
		CxxToken *token;
		// Create trunk of the tree which is the file it self.
		CxxAbstractSyntaxTreeNode *ite=malloc(sizeof(CxxAbstractSyntaxTreeNode));
		*tree=ite;
		ite->parent=0;
		ite->siblingsolder=0;
		ite->siblingsyounger=0;
		ite->childlen=0;
		ite->token=0;
		ite->childrenoldest=0;
		ite->childrenyougest=(CxxAbstractSyntaxTreeNode*)&ite->childrenoldest;

		// Lex the file to create abstract syntax tree.
		while((token=cxxLexGetNext(source))){

			{
				// Allocate next child
				CxxAbstractSyntaxTreeNode *newchild=malloc(sizeof(CxxAbstractSyntaxTreeNode));
				ite->childrenyougest->siblingsyounger=newchild;
				newchild->siblingsolder=ite->childrenyougest;
				ite->childlen++;
				ite->childrenyougest=newchild;
				newchild->parent=ite;
				newchild->childlen=0;
				newchild->childrenoldest=0;
				newchild->childrenyougest=(CxxAbstractSyntaxTreeNode*)&newchild->childrenoldest;
				newchild->siblingsyounger=0;
				newchild->token=token;

				// Move to child.
				ite=newchild;
			}
			{
				uint32_t tokenstrlen;
				char *tokenstr=tokenTypeToStr(token,&tokenstrlen);
				printStrCat(STDOUT_FILENO,tokenstr," :\n",tokenstrlen,3);
				printStrCat(STDOUT_FILENO,token->characters,"\n",token->len,1);
			}
		}
		return CXX_SYNTAX_GOOD;
	}
  /****************
  * See cxxlex.h  *
  ****************/
  void freeCxxSyntaxTree(CxxAbstractSyntaxTreeNode *tree){
		CxxAbstractSyntaxTreeNode *ite=tree;
		while(ite){
			if(ite->childrenoldest){
					// To prevent infinite loop
					// we have to mark that there
					// is no children
					CxxAbstractSyntaxTreeNode *nochild=ite;
					ite=ite->childrenoldest;
					nochild->childrenoldest=0;
			}
			else if(ite->siblingsyounger){
				CxxAbstractSyntaxTreeNode *remove=ite;
				ite=ite->siblingsyounger;
				free(remove->token);
				free(remove);
			}
			else{
				CxxAbstractSyntaxTreeNode *remove=ite;
				ite=ite->parent;
				free(remove->token);
				free(remove);
			}
		}
  }
	/****************
  * See cxxlex.h  *
  ****************/
  char *tokenTypeToStr(const CxxToken *token,uint32_t *len){
		// We should cause a error here if enum type isn't handled
		// so that we know that we missed one.
		#pragma GCC diagnostic push
		#pragma GCC diagnostic error "-Wswitch-enum"

  	// Just switch on different string and length of the string.
		switch(token->type){
			case CXX_TOKEN_COMMENT:
				*len=sizeof("CXX_TOKEN_COMMENT")-1;
				return "CXX_TOKEN_COMMENT";
			case CXX_TOKEN_INCLUDE:
				*len=sizeof("CXX_TOKEN_INCLUDE")-1;
				return "CXX_TOKEN_INCLUDE";
			case CXX_TOKEN_PREPROCESS_DEFINTION:
				*len=sizeof("CXX_TOKEN_PREPROCESS_DEFINTION")-1;
				return "CXX_TOKEN_PREPROCESS_DEFINTION";
			case CXX_TOKEN_PREPROCESS_IF:
				*len=sizeof("CXX_TOKEN_PREPROCESS_IF")-1;
				return "CXX_TOKEN_PREPROCESS_IF";
			case CXX_TOKEN_PREPROCESS_IFDEF:
				*len=sizeof("CXX_TOKEN_PREPROCESS_IFDEF")-1;
				return "CXX_TOKEN_PREPROCESS_IFDEF";
			case CXX_TOKEN_PREPROCESS_ELSE:
				*len=sizeof("CXX_TOKEN_PREPROCESS_ELSE")-1;
				return "CXX_TOKEN_PREPROCESS_ELSE";
			case CXX_TOKEN_PREPROCESS_ENDIF:
				*len=sizeof("CXX_TOKEN_PREPROCESS_ENDIF")-1;
				return "CXX_TOKEN_PREPROCESS_ENDIF";
			case CXX_TOKEN_PRAGMA:
				*len=sizeof("CXX_TOKEN_PRAGMA")-1;
				return "CXX_TOKEN_PRAGMA";
			case CXX_TOKEN_BRACES:
				*len=sizeof("CXX_TOKEN_BRACES")-1;
				return "CXX_TOKEN_BRACES";
			case CXX_TOKEN_BRACKETS:
				*len=sizeof("CXX_TOKEN_BRACKETS")-1;
				return "CXX_TOKEN_BRACKETS";
			case CXX_TOKEN_PARENTHES:
				*len=sizeof("CXX_TOKEN_PARENTHES")-1;
				return "CXX_TOKEN_PARENTHES";
			case CXX_TOKEN_CHARACTER:
				*len=sizeof("CXX_TOKEN_CHARACTER")-1;
				return "CXX_TOKEN_CHARACTER";
			case CXX_TOKEN_STRING:
				*len=sizeof("CXX_TOKEN_STRING")-1;
				return "CXX_TOKEN_STRING";
			case CXX_TOKEN_COMMA:
				*len=sizeof("CXX_TOKEN_COMMA")-1;
				return "CXX_TOKEN_COMMA";
			case CXX_TOKEN_NUMBER_BIN:
				*len=sizeof("CXX_TOKEN_NUMBER_BIN")-1;
				return "CXX_TOKEN_NUMBER_BIN";
			case CXX_TOKEN_NUMBER_OCTAL:
				*len=sizeof("CXX_TOKEN_NUMBER_OCTAL")-1;
				return "CXX_TOKEN_NUMBER_OCTAL";
			case CXX_TOKEN_NUMBER_DEC:
				*len=sizeof("CXX_TOKEN_NUMBER_DEC")-1;
				return "CXX_TOKEN_NUMBER_DEC";
			case CXX_TOKEN_NUMBER_HEX:
				*len=sizeof("CXX_TOKEN_NUMBER_DEC")-1;
				return "CXX_TOKEN_NUMBER_DEC";
			case CXX_TOKEN_DECLERATION:
				*len=sizeof("CXX_TOKEN_DECLERATION")-1;
				return "CXX_TOKEN_DECLERATION";
			case CXX_TOKEN_IDENTIFIER:
				*len=sizeof("CXX_TOKEN_IDENTIFIER")-1;
				return "CXX_TOKEN_IDENTIFIER";
			case CXX_TOKEN_OPERATOR:
				*len=sizeof("CXX_TOKEN_OPERATOR")-1;
				return "CXX_TOKEN_OPERATOR";
			case CXX_TOKEN_IF:
				*len=sizeof("CXX_TOKEN_IF")-1;
				return "CXX_TOKEN_IF";
			case CXX_TOKEN_ELSE:
				*len=sizeof("CXX_TOKEN_ELSE")-1;
				return "CXX_TOKEN_ELSE";
			case CXX_TOKEN_ELSEIF:
				*len=sizeof("CXX_TOKEN_ELSEIF")-1;
				return "CXX_TOKEN_ELSEIF";
			case CXX_TOKEN_FUNCTION:
				*len=sizeof("CXX_TOKEN_FUNCTION")-1;
				return "CXX_TOKEN_FUNCTION";
			case CXX_TOKEN_STATEMENT_END:
				*len=sizeof("CXX_TOKEN_STATEMENT_END")-1;
				return "CXX_TOKEN_STATEMENT_END";
			case CXX_TOKEN_STRUCTURE:
				*len=sizeof("CXX_TOKEN_STRUCTURE")-1;
				return "CXX_TOKEN_STRUCTURE";
			case CXX_TOKEN_CLASS:
				*len=sizeof("CXX_TOKEN_CLASS")-1;
				return "CXX_TOKEN_CLASS";
			case CXX_TOKEN_TYPEDEF:
				*len=sizeof("CXX_TOKEN_TYPEDEF")-1;
				return "CXX_TOKEN_TYPEDEF";
			case CXX_TOKEN_ALIGNAS:
				*len=sizeof("CXX_TOKEN_ALIGNAS")-1;
				return "CXX_TOKEN_ALIGNAS";
			case CXX_TOKEN_ALIGNOF:
				*len=sizeof("CXX_TOKEN_ALIGNOF")-1;
				return "CXX_TOKEN_ALIGNOF";
			case CXX_TOKEN_AND:
				*len=sizeof("CXX_TOKEN_AND")-1;
				return "CXX_TOKEN_AND";
			case CXX_TOKEN_AND_EQ:
				*len=sizeof("CXX_TOKEN_AND_EQ")-1;
				return "CXX_TOKEN_AND_EQ";
			case CXX_TOKEN_ASM:
				*len=sizeof("CXX_TOKEN_ASM")-1;
				return "CXX_TOKEN_ASM";
			case CXX_TOKEN_AUTO:
				*len=sizeof("CXX_TOKEN_AUTO")-1;
				return "CXX_TOKEN_AUTO";
			case CXX_TOKEN_BOOL:
				*len=sizeof("CXX_TOKEN_BOOL")-1;
				return "CXX_TOKEN_BOOL";
			case CXX_TOKEN_CHAR:
				*len=sizeof("CXX_TOKEN_CHAR")-1;
				return "CXX_TOKEN_CHAR";
			case CXX_TOKEN_INLINE:
				*len=sizeof("CXX_TOKEN_INLINE")-1;
				return "CXX_TOKEN_INLINE";
			case CXX_TOKEN_INT:
				*len=sizeof("CXX_TOKEN_INT")-1;
				return "CXX_TOKEN_INT";
			case CXX_TOKEN_PRIVATE:
				*len=sizeof("CXX_TOKEN_PRIVATE")-1;
				return "CXX_TOKEN_PRIVATE";
			case CXX_TOKEN_PROTECTED:
				*len=sizeof("CXX_TOKEN_PROTECTED")-1;
				return "CXX_TOKEN_PROTECTED";
			case CXX_TOKEN_PUBLIC:
				*len=sizeof("CXX_TOKEN_PUBLIC")-1;
				return "CXX_TOKEN_PUBLIC";
			case CXX_TOKEN_THIS:
				*len=sizeof("CXX_TOKEN_THIS")-1;
				return "CXX_TOKEN_THIS";
			case CXX_TOKEN_STATIC:
				*len=sizeof("CXX_TOKEN_STATIC")-1;
				return "CXX_TOKEN_STATIC";
			default:
				return 0;
		}
		#pragma GCC diagnostic pop
  }
