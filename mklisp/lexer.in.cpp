#include <mklisp/lexer.h>
#include <algorithm>

using namespace mklisp;

enum LexCondition {
	yycInitialCondition = 0,

	yycStringCondition,
	yycEscapeCondition,

	yycCommentCondition,
	yycLineCommentCondition,
};

InternalExceptionPointer mklisp::Lexer::lex(std::pmr::memory_resource *memoryResource, std::string_view src) {
	const char *YYCURSOR = src.data(), *YYMARKER = YYCURSOR, *YYLIMIT = src.data() + src.size();
	const char *prevYYCURSOR = YYCURSOR;

	LexCondition YYCONDITION = yycInitialCondition;

#define YYSETCONDITION(cond) (YYCONDITION = (yyc##cond))
#define YYGETCONDITION() (YYCONDITION)

	std::unique_ptr<Token> token;

	while (true) {
		std::string strLiteral;

		token = std::make_unique<Token>();

		while (true) {
			/*!re2c
				re2c:yyfill:enable = 0;
				re2c:define:YYCTYPE = char;

				<InitialCondition>"///"		{ YYSETCONDITION(LineCommentCondition); token->tokenId = TokenId::Comment; continue; }
				<InitialCondition>"//"		{ YYSETCONDITION(LineCommentCondition); token->tokenId = TokenId::Comment; continue; }
				<InitialCondition>"/*"		{ YYSETCONDITION(CommentCondition); token->tokenId = TokenId::Comment; continue; }

				<InitialCondition>"("		{ token->tokenId = TokenId::LParenthese; break; }
				<InitialCondition>")"		{ token->tokenId = TokenId::RParenthese; break; }

				<InitialCondition>"0"[0-7]+ {
					token->tokenId = TokenId::UIntLiteral;
					token->exData = std::make_unique<UIntLiteralTokenExtension>(strtoul(prevYYCURSOR, nullptr, 8));
					break;
				}

				<InitialCondition>[0-9]+ {
					token->tokenId = TokenId::IntLiteral;
					token->exData = std::make_unique<IntLiteralTokenExtension>(strtol(prevYYCURSOR, nullptr, 10));
					break;
				}

				<InitialCondition>"0"[xX][0-9a-fA-F]+ {
					token->tokenId = TokenId::UIntLiteral;
					token->exData = std::make_unique<UIntLiteralTokenExtension>(strtoul(prevYYCURSOR, nullptr, 16));
					break;
				}

				<InitialCondition>"0"[bB][01]+ {
					token->tokenId = TokenId::UIntLiteral;
					token->exData = std::make_unique<UIntLiteralTokenExtension>(strtoul(prevYYCURSOR, nullptr, 2));
					break;
				}

				<InitialCondition>[0-9]+"."[0-9]+[fF] {
					token->tokenId = TokenId::FloatLiteral;
					token->exData = std::make_unique<FloatLiteralTokenExtension>(strtof(prevYYCURSOR, nullptr));
					break;
				}

				<InitialCondition>[0-9]+"."[0-9]+ {
					token->tokenId = TokenId::DoubleLiteral;
					token->exData = std::make_unique<DoubleLiteralTokenExtension>(strtod(prevYYCURSOR, nullptr));
					break;
				}

				<InitialCondition>"'"		{ token->tokenId = TokenId::Quote; break; }
				<InitialCondition>"\""		{ YYSETCONDITION(StringCondition); continue; }

				<InitialCondition>"\n"		{ token->tokenId = TokenId::NewLine; break; }
				<InitialCondition>"\000"	{ goto end; }

				<InitialCondition>[ \r\t]+	{ token->tokenId = TokenId::Whitespace; break; }

				<InitialCondition>[^ \r\t\n\000()'\"]+ {
					token->tokenId = TokenId::Id;
					break;
				}
				<InitialCondition>[^] {
					size_t beginIndex = prevYYCURSOR - src.data(), endIndex = YYCURSOR - src.data();
					std::string_view strToBegin = src.substr(0, beginIndex), strToEnd = src.substr(0, endIndex);

					size_t index = prevYYCURSOR - src.data();
					auto pos = src.find_last_of('\n', index);
					if(pos == std::string::npos)
						pos = 0;
					pos = index - pos;

					// Invalid token.
					return LexicalError::alloc(memoryResource, SourcePosition { (size_t)std::count(strToBegin.begin(), strToBegin.end(), '\n'), pos });
				}

				<StringCondition>"\""		{
					YYSETCONDITION(InitialCondition);
					token->tokenId = TokenId::StringLiteral;
					token->exData = std::make_unique<StringLiteralTokenExtension>(strLiteral);
					strLiteral.clear();
					break;
				}
				<StringCondition>"\\\n"		{ continue; }
				<StringCondition>"\\"		{ YYSETCONDITION(EscapeCondition); continue; }
				<StringCondition>"\n"		{
					size_t beginIndex = prevYYCURSOR - src.data(), endIndex = YYCURSOR - src.data();
					std::string_view strToBegin = src.substr(0, beginIndex), strToEnd = src.substr(0, endIndex);

					size_t index = prevYYCURSOR - src.data();
					auto pos = src.find_last_of('\n', index);
					if(pos == std::string::npos)
						pos = 0;
					pos = index - pos;

					// Unexpected end of line.
					return LexicalError::alloc(memoryResource, SourcePosition { (size_t)std::count(strToBegin.begin(), strToBegin.end(), '\n'), pos });
				}
				<StringCondition>"\000"	{
					size_t beginIndex = prevYYCURSOR - src.data(), endIndex = YYCURSOR - src.data();
					std::string_view strToBegin = src.substr(0, beginIndex), strToEnd = src.substr(0, endIndex);

					size_t index = prevYYCURSOR - src.data();
					auto pos = src.find_last_of('\n', index);
					if(pos == std::string::npos)
						pos = 0;
					pos = index - pos;

					// Prematured end of file.
					return LexicalError::alloc(memoryResource, SourcePosition { (size_t)std::count(strToBegin.begin(), strToBegin.end(), '\n'), pos });
				}
				<StringCondition>[^]		{ strLiteral += YYCURSOR[-1]; continue; }

				<EscapeCondition>"\'"	{ YYSETCONDITION(StringCondition); strLiteral += "\'"; continue; }
				<EscapeCondition>"\""	{ YYSETCONDITION(StringCondition); strLiteral += "\""; continue; }
				<EscapeCondition>"\?"	{ YYSETCONDITION(StringCondition); strLiteral += "\?"; continue; }
				<EscapeCondition>"\\"	{ YYSETCONDITION(StringCondition); strLiteral += "\\"; continue; }
				<EscapeCondition>"a"	{ YYSETCONDITION(StringCondition); strLiteral += "\a"; continue; }
				<EscapeCondition>"b"	{ YYSETCONDITION(StringCondition); strLiteral += "\b"; continue; }
				<EscapeCondition>"f"	{ YYSETCONDITION(StringCondition); strLiteral += "\f"; continue; }
				<EscapeCondition>"n"	{ YYSETCONDITION(StringCondition); strLiteral += "\n"; continue; }
				<EscapeCondition>"r"	{ YYSETCONDITION(StringCondition); strLiteral += "\r"; continue; }
				<EscapeCondition>"t"	{ YYSETCONDITION(StringCondition); strLiteral += "\t"; continue; }
				<EscapeCondition>"v"	{ YYSETCONDITION(StringCondition); strLiteral += "\v"; continue; }
				<EscapeCondition>[0-7]{1,3}	{
					YYSETCONDITION(StringCondition);

					size_t size = YYCURSOR - prevYYCURSOR;

					char c = 0;
					for(uint_fast8_t i = 0; i < size; ++i) {
						c *= 8;
						c += prevYYCURSOR[i] - '0';
					}

					strLiteral += "\0";
				}
				<EscapeCondition>[xX][0-9a-fA-F]{1,2}	{
					YYSETCONDITION(StringCondition);

					size_t size = YYCURSOR - prevYYCURSOR;

					char c = 0, j;

					for(uint_fast8_t i = 1; i < size; ++i) {
						c *= 16;

						j = prevYYCURSOR[i];
						if((j >= '0') && (j <= '9'))
							c += prevYYCURSOR[i] - '0';
						else if((j >= 'a') && (j <= 'f'))
							c += prevYYCURSOR[i] - 'a';
						else if((j >= 'A') && (j <= 'F'))
							c += prevYYCURSOR[i] - 'A';
					}

					strLiteral += "\0";
				}

				<CommentCondition>"*"[/]	{ YYSETCONDITION(InitialCondition); break; }
				<CommentCondition>[^]		{ continue; }

				<LineCommentCondition>"\n"	{ YYSETCONDITION(InitialCondition); break; }
				<LineCommentCondition>[^]	{ continue; }
			*/
		}

		size_t beginIndex = prevYYCURSOR - src.data(), endIndex = YYCURSOR - src.data();

		std::string_view strToBegin = src.substr(0, beginIndex), strToEnd = src.substr(0, endIndex);

		token->text = std::string(prevYYCURSOR, YYCURSOR - prevYYCURSOR);

		size_t idxLastBeginNewline = src.find_last_of('\n', beginIndex),
			   idxLastEndNewline = src.find_last_of('\n', endIndex);

		token->location.beginPosition = {
			(size_t)std::count(strToBegin.begin(), strToBegin.end(), '\n'),
			(idxLastBeginNewline == std::string::npos
					? beginIndex
					: beginIndex - idxLastBeginNewline - 1)
		};
		token->location.endPosition = {
			(size_t)std::count(strToEnd.begin(), strToEnd.end(), '\n'),
			(idxLastEndNewline == std::string::npos
					? endIndex
					: endIndex - idxLastEndNewline - 1)
		};
		tokens.push_back(std::move(token));

		prevYYCURSOR = YYCURSOR;
	}

end:

	_endToken = std::make_unique<Token>();
	*(_endToken.get()) = {
		tokens.size(),
		TokenId::End,
		SourceLocation { token->location.endPosition, token->location.endPosition },
		""
	};

	return {};
}
