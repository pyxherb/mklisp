#ifndef _MKLISP_RUNTIME_H_
#define _MKLISP_RUNTIME_H_

#include "basedefs.h"
#include "object.h"
#include <memory_resource>
#include <list>
#include <unordered_map>
#include <stack>

namespace mklisp {
	class CountablePoolResource : public std::pmr::memory_resource {
	public:
		std::pmr::memory_resource *upstream;
		size_t szAllocated = 0;

		MKLISP_API CountablePoolResource(std::pmr::memory_resource *upstream);
		MKLISP_API CountablePoolResource(const CountablePoolResource &) = delete;

		MKLISP_API virtual void *do_allocate(size_t bytes, size_t alignment) override;
		MKLISP_API virtual void do_deallocate(void *p, size_t bytes, size_t alignment) override;
		MKLISP_API virtual bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override;
	};

	struct Frame {
		ListObject *curEvalList;
		size_t curEvalIndex = 0;
		Value returnValue = Value(ValueType::Nil);
	};

	struct Context {
		std::pmr::list<Frame> frameList;
		std::pmr::unordered_map<std::pmr::string, Object *> bindings;
	};

	class Runtime {
	public:
		CountablePoolResource globalHeapResource;
		std::pmr::list<Object *> createdObjects;

		MKLISP_API Runtime(std::pmr::memory_resource *upstream);
		MKLISP_API ~Runtime();

		MKLISP_API Value evalList(Context *context);
		MKLISP_API Value eval(Value value, Context *context);
	};
}

#endif
