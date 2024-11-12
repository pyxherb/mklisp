#include "parser.h"
#include "runtime.h"

using namespace mklisp;

Parser::Parser(Runtime *associatedRuntime) : associatedRuntime(associatedRuntime) {
}

InternalExceptionPointer Parser::expectToken(Token *token) {
	if (token->tokenId == TokenId::End) {
		std::pmr::string msg(&associatedRuntime->globalHeapResource);

		msg = "Unexpected end of string";

		return SyntaxError::alloc(&associatedRuntime->globalHeapResource, std::move(msg));
	}

	return {};
}

InternalExceptionPointer Parser::expectToken(Token *token, TokenId tokenId) {
	if (token->tokenId != tokenId) {
		std::pmr::string msg(&associatedRuntime->globalHeapResource);

		msg = "Expecting ";
		msg += getTokenName(tokenId);

		return SyntaxError::alloc(&associatedRuntime->globalHeapResource, std::move(msg));
	}

	return {};
}

InternalExceptionPointer Parser::parseExpr(Lexer *lexer, Value &valueOut, HostRefHolder &hostRefHolder) {
	Token *token;

	MKLISP_RETURN_IF_EXCEPT(expectToken((token = lexer->peekToken())));

	switch (token->tokenId) {
		case TokenId::Quote: {
			lexer->nextToken();
			MKLISP_RETURN_IF_EXCEPT(parseExpr(lexer, valueOut, hostRefHolder));
			if (valueOut.valueType == ValueType::Object)
				valueOut.valueType = ValueType::QuotedObject;
			break;
		}
		case TokenId::LParenthese: {
			lexer->nextToken();
			auto listObj = ListObject::alloc(associatedRuntime);
			hostRefHolder.addObject(listObj.get());

			while (true) {
				Value curValue;

				token = lexer->peekToken();
				switch (token->tokenId) {
					case TokenId::RParenthese:
						lexer->nextToken();
						goto end;
					case TokenId::End: {
						std::pmr::string msg(&associatedRuntime->globalHeapResource);

						msg = "Unexpected end of string";

						return SyntaxError::alloc(&associatedRuntime->globalHeapResource, std::move(msg));
						break;
					}
					default:
						MKLISP_RETURN_IF_EXCEPT(parseExpr(lexer, curValue, hostRefHolder));
				}

				listObj->elements.push_back(curValue);
			}

		end:
			valueOut = Value(listObj.get());
			break;
		}
		case TokenId::IntLiteral:
			lexer->nextToken();
			valueOut = Value((int32_t)((IntLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::UIntLiteral:
			lexer->nextToken();
			valueOut = Value((uint32_t)((UIntLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::LongLiteral:
			lexer->nextToken();
			valueOut = Value((int64_t)((LongLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::ULongLiteral:
			lexer->nextToken();
			valueOut = Value((uint64_t)((ULongLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::ShortLiteral:
			lexer->nextToken();
			valueOut = Value((int16_t)((ShortLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::UShortLiteral:
			lexer->nextToken();
			valueOut = Value((uint16_t)((UShortLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::ByteLiteral:
			lexer->nextToken();
			valueOut = Value((int8_t)((ByteLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::UByteLiteral:
			lexer->nextToken();
			valueOut = Value((uint8_t)((UByteLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::CharLiteral:
			lexer->nextToken();
			valueOut = Value((char32_t)((CharLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::FloatLiteral:
			lexer->nextToken();
			valueOut = Value((uint8_t)((FloatLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::DoubleLiteral:
			lexer->nextToken();
			valueOut = Value((uint8_t)((DoubleLiteralTokenExtension *)token->exData.get())->data);
			break;
		case TokenId::StringLiteral: {
			lexer->nextToken();
			const std::string &src = ((StringLiteralTokenExtension *)token->exData.get())->data;
			std::pmr::string s(&associatedRuntime->globalHeapResource);

			s = src;

			auto strObj = StringObject::alloc(associatedRuntime, std::move(s));
			hostRefHolder.addObject(strObj.get());

			valueOut = Value(strObj.get());
			break;
		}
		case TokenId::Id: {
			lexer->nextToken();
			const std::string &src = token->text;
			std::pmr::string s(&associatedRuntime->globalHeapResource);

			s.resize(src.size());
			memcpy(s.data(), src.data(), src.size());

			auto symObj = SymbolObject::alloc(associatedRuntime, std::move(s));
			hostRefHolder.addObject(symObj.get());

			valueOut = Value(symObj.get());
			break;
		}
		default: {
			std::pmr::string msg(&associatedRuntime->globalHeapResource);

			msg = "Unrecognized token";

			return SyntaxError::alloc(&associatedRuntime->globalHeapResource, std::move(msg));
		}
	}

	return {};
}

InternalExceptionPointer Parser::parse(Lexer *lexer, HostObjectRef<ListObject> &listOut, HostRefHolder &hostRefHolder) {
	Token *beginningToken;
	Value v;

	listOut = ListObject::alloc(associatedRuntime);

	while (true) {
		if ((beginningToken = lexer->peekToken())->tokenId == TokenId::End)
			break;

		MKLISP_RETURN_IF_EXCEPT(parseExpr(lexer, v, hostRefHolder));

		listOut->elements.push_back(v);
	}

	return {};
}
