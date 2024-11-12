#ifndef _MKLISP_PARSER_H_
#define _MKLISP_PARSER_H_

#include "lexer.h"
#include "value.h"
#include "object.h"

namespace mklisp {
	class Parser {
	public:
		Runtime *associatedRuntime;

		Parser(Runtime *associatedRuntime);

		InternalExceptionPointer expectToken(Token *token);
		InternalExceptionPointer expectToken(Token *token, TokenId tokenId);

		InternalExceptionPointer parseExpr(Lexer *lexer, Value &valueOut, HostRefHolder &hostRefHolder);
		InternalExceptionPointer parse(Lexer *lexer, HostObjectRef<ListObject> &listOut, HostRefHolder &hostRefHolder);
	};
}

#endif
