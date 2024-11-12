#include "except.h"

using namespace mklisp;

MKLISP_API CompilationError::CompilationError(
	std::pmr::memory_resource *memoryResource,
	CompilationErrorCode errorCode) : InternalException(memoryResource, InternalExceptionKind::CompilationError), errorCode(errorCode) {
}

MKLISP_API CompilationError::~CompilationError() {
}

MKLISP_API LexicalError::LexicalError(
	std::pmr::memory_resource *memoryResource,
	const SourcePosition &sourcePosition) : CompilationError(memoryResource, CompilationErrorCode::LexicalError), sourcePosition(sourcePosition) {
}

MKLISP_API LexicalError::~LexicalError() {
}

MKLISP_API void LexicalError::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<LexicalError>;
	Alloc allocator(memoryResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API LexicalError *LexicalError::alloc(
	std::pmr::memory_resource *memoryResource,
	const SourcePosition &sourcePosition) {
	using Alloc = std::pmr::polymorphic_allocator<LexicalError>;
	Alloc allocator(memoryResource);

	std::unique_ptr<LexicalError, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), memoryResource, sourcePosition);

	return ptr.release();
}

MKLISP_API SyntaxError::SyntaxError(
	std::pmr::memory_resource *memoryResource,
	std::pmr::string &&message) : CompilationError(memoryResource, CompilationErrorCode::SyntaxError), message(message) {
}

MKLISP_API SyntaxError::~SyntaxError() {
}

MKLISP_API void SyntaxError::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<SyntaxError>;
	Alloc allocator(memoryResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API SyntaxError *SyntaxError::alloc(
	std::pmr::memory_resource *memoryResource,
	std::pmr::string &&message) {
	using Alloc = std::pmr::polymorphic_allocator<SyntaxError>;
	Alloc allocator(memoryResource);

	std::unique_ptr<SyntaxError, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), memoryResource, std::move(message));

	return ptr.release();
}
