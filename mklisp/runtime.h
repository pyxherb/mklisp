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

	enum class EvalState {
		Initial = 0,
		EvalArgs,
		ReceivingEvaluatedArg,
		Call
	};

	union EvalStateExData {
		struct {
			size_t index;
			Value callTarget;
		} asEvalArgs;
		struct {
			Value callTarget;
		} asCall;

		MKLISP_API EvalStateExData();
	};

	struct Frame {
		ListObject *curEvalList;
		EvalState evalState;
		EvalStateExData evalStateExData;
		Value returnValue = Value(ValueType::Nil);
	};

	class Runtime;

	struct Context {
		Runtime *runtime;
		std::pmr::list<Frame> frameList;
		std::pmr::unordered_map<std::pmr::string, Object *> bindings;

		MKLISP_API Context(Runtime *runtime);
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
