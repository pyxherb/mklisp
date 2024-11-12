#ifndef _MKLISP_EXCEPT_H_
#define _MKLISP_EXCEPT_H_

#include "except_base.h"
#include "astnode.h"

namespace mklisp {
	enum class CompilationErrorCode {
		LexicalError = 0,
		SyntaxError
	};

	class CompilationError : public InternalException {
	public:
		CompilationErrorCode errorCode;

		MKLISP_API CompilationError(
			std::pmr::memory_resource *memoryResource,
			CompilationErrorCode errorCode);
		MKLISP_API virtual ~CompilationError();
	};

	class LexicalError : public CompilationError {
	public:
		SourcePosition sourcePosition;

		MKLISP_API LexicalError(
			std::pmr::memory_resource *memoryResource,
			const SourcePosition &sourcePosition);
		MKLISP_API virtual ~LexicalError();
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static LexicalError *alloc(
			std::pmr::memory_resource *memoryResource,
			const SourcePosition &sourcePosition);
	};

	class SyntaxError : public CompilationError {
	public:
		SourceLocation sourceLocation;
		std::pmr::string message;

		MKLISP_API SyntaxError(
			std::pmr::memory_resource *memoryResource,
			std::pmr::string &&message);
		MKLISP_API virtual ~SyntaxError();
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static SyntaxError *alloc(
			std::pmr::memory_resource *memoryResource,
			std::pmr::string &&message);
	};
}

#endif
