#ifndef _MKLISP_EXCEPTBASE_H_
#define _MKLISP_EXCEPTBASE_H_

#include "basedefs.h"
#include "util.h"
#include <memory>
#include <memory_resource>
#include <cassert>

namespace mklisp {
	enum class InternalExceptionKind {
		CompilationError = 0
	};

	class InternalException {
	public:
		std::pmr::memory_resource *memoryResource;
		InternalExceptionKind exceptionKind;

		MKLISP_API InternalException(
			std::pmr::memory_resource *memoryResource,
			InternalExceptionKind exceptionKind);
		MKLISP_API virtual ~InternalException();

		virtual void dealloc() noexcept = 0;
	};

	class InternalExceptionPointer {
	private:
		std::unique_ptr<InternalException, DeallocableDeleter<InternalException>> _ptr;

	public:
		MKLISP_FORCEINLINE InternalExceptionPointer() = default;
		MKLISP_FORCEINLINE InternalExceptionPointer(InternalException *exception) : _ptr(exception) {
		}

		MKLISP_FORCEINLINE ~InternalExceptionPointer() {
			if (_ptr) {
				assert(("Unhandled internal exception: ", false));
			}
		}

		InternalExceptionPointer(const InternalExceptionPointer &) = delete;
		InternalExceptionPointer &operator=(const InternalExceptionPointer &) = delete;
		MKLISP_FORCEINLINE InternalExceptionPointer(InternalExceptionPointer &&other) {
			_ptr = std::move(other._ptr);
		}
		MKLISP_FORCEINLINE InternalExceptionPointer &operator=(InternalExceptionPointer &&other) {
			_ptr = std::move(other._ptr);
			return *this;
		}

		MKLISP_FORCEINLINE InternalException *get() {
			return _ptr.get();
		}
		MKLISP_FORCEINLINE const InternalException *get() const {
			return _ptr.get();
		}

		MKLISP_FORCEINLINE void reset() {
			_ptr.reset();
		}

		MKLISP_FORCEINLINE explicit operator bool() {
			return (bool)_ptr;
		}

		MKLISP_FORCEINLINE InternalException *operator->() {
			return _ptr.get();
		}

		MKLISP_FORCEINLINE const InternalException *operator->() const {
			return _ptr.get();
		}
	};
}

#define MKLISP_RETURN_IF_EXCEPT(expr)                  \
	if (mklisp::InternalExceptionPointer e = (expr); (bool)e) \
	return e
#define MKLISP_RETURN_IF_EXCEPT_WITH_LVAR(name, expr) \
	if ((bool)(name = (expr)))                       \
		return name;

#endif
