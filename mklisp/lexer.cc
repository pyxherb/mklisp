#include "lexer.h"
#include <algorithm>

using namespace mklisp;

const char *mklisp::getTokenName(TokenId tokenId) {
	switch (tokenId) {
		case TokenId::End:
			return "end of file";
		case TokenId::LParenthese:
			return "(";
		case TokenId::RParenthese:
			return ")";
		case TokenId::IntLiteral:
			return "integer literal";
		case TokenId::LongLiteral:
			return "long literal";
		case TokenId::UIntLiteral:
			return "unsigned integer literal";
		case TokenId::ULongLiteral:
			return "unsigned long literal";
		case TokenId::FloatLiteral:
			return "32-bit floating-point number literal";
		case TokenId::DoubleLiteral:
			return "64-bit floating-point number literal";
		case TokenId::StringLiteral:
			return "string literal";
		case TokenId::Id:
			return "identifier";
	}

	return "<unknown tokenId>";
}

Token *Lexer::nextToken(bool keepNewLine, bool keepWhitespace, bool keepComment) {
	size_t &i = context.curIndex;

	while (i < tokens.size()) {
		tokens[i]->index = i;

		switch (tokens[i]->tokenId) {
			case TokenId::NewLine:
				if (keepNewLine) {
					context.prevIndex = context.curIndex;
					return tokens[i++].get();
				}
				break;
			case TokenId::Whitespace:
				if (keepWhitespace) {
					context.prevIndex = context.curIndex;
					return tokens[i++].get();
				}
				break;
			case TokenId::Comment:
				if (keepComment) {
					context.prevIndex = context.curIndex;
					return tokens[i++].get();
				}
				break;
			default:
				context.prevIndex = context.curIndex;
				return tokens[i++].get();
		}

		++i;
	}

	return _endToken.get();
}

Token *Lexer::peekToken(bool keepNewLine, bool keepWhitespace, bool keepComment) {
	size_t i = context.curIndex;

	while (i < tokens.size()) {
		tokens[i]->index = i;

		switch (tokens[i]->tokenId) {
			case TokenId::NewLine:
				if (keepNewLine)
					return tokens[i].get();
				break;
			case TokenId::Whitespace:
				if (keepWhitespace)
					return tokens[i].get();
				break;
			case TokenId::Comment:
				if (keepComment)
					return tokens[i].get();
				break;
			default:
				return tokens[i].get();
		}

		++i;
	}

	return _endToken.get();
}

size_t Lexer::getTokenByPosition(const SourcePosition &position) {
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i]->location.beginPosition <= position && tokens[i]->location.endPosition >= position)
			return i;
	}

	return SIZE_MAX;
}
