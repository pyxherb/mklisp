#ifndef _MKLISP_LEXER_H_
#define _MKLISP_LEXER_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>
#include <deque>

#include "astnode.h"
#include "except.h"

namespace mklisp {
	enum class TokenId : int {
		End = -1,

		Unknown,

		LParenthese,
		RParenthese,

		Quote,

		IntLiteral,
		LongLiteral,
		UIntLiteral,
		ULongLiteral,
		ShortLiteral,
		UShortLiteral,
		ByteLiteral,
		UByteLiteral,
		CharLiteral,
		FloatLiteral,
		DoubleLiteral,
		StringLiteral,

		Id,

		Whitespace,
		NewLine,
		Comment,
	};

	const char *getTokenName(TokenId tokenId);

	struct TokenExtension {
		virtual ~TokenExtension() = default;
	};

	template <typename T>
	struct LiteralTokenExtension : public TokenExtension {
		T data;

		inline LiteralTokenExtension(T data) : data(data) {}
		virtual ~LiteralTokenExtension() = default;
	};

	using IntLiteralTokenExtension = LiteralTokenExtension<int32_t>;
	using UIntLiteralTokenExtension = LiteralTokenExtension<uint32_t>;
	using LongLiteralTokenExtension = LiteralTokenExtension<int64_t>;
	using ULongLiteralTokenExtension = LiteralTokenExtension<uint64_t>;
	using ShortLiteralTokenExtension = LiteralTokenExtension<int16_t>;
	using UShortLiteralTokenExtension = LiteralTokenExtension<uint16_t>;
	using ByteLiteralTokenExtension = LiteralTokenExtension<int8_t>;
	using UByteLiteralTokenExtension = LiteralTokenExtension<uint8_t>;
	using CharLiteralTokenExtension = LiteralTokenExtension<char32_t>;
	using FloatLiteralTokenExtension = LiteralTokenExtension<float>;
	using DoubleLiteralTokenExtension = LiteralTokenExtension<double>;
	using StringLiteralTokenExtension = LiteralTokenExtension<std::string>;

	struct MajorContext;

	struct Token {
		size_t index = SIZE_MAX;
		TokenId tokenId;
		SourceLocation location;
		std::string text;
		std::unique_ptr<TokenExtension> exData;
	};

	struct LexerContext {
		size_t prevIndex = 0;
		size_t curIndex = 0;
	};

	class Lexer {
	private:
		std::unique_ptr<Token> _endToken;

	public:
		LexerContext context;

		std::deque<std::unique_ptr<Token>> tokens;

		InternalExceptionPointer lex(std::pmr::memory_resource *memoryResource, std::string_view src);

		Token *nextToken(bool keepNewLine = false, bool keepWhitespace = false, bool keepComment = false);
		Token *peekToken(bool keepNewLine = false, bool keepWhitespace = false, bool keepComment = false);

		inline void reload() {
			context = {};
			_endToken = {};
			tokens.clear();
		}

		size_t getTokenByPosition(const SourcePosition &position);

		inline size_t getTokenIndex(Token *token) {
			return token->index;
		}
	};
}

#endif
