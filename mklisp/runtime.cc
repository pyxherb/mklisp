#include "runtime.h"

using namespace mklisp;

MKLISP_API CountablePoolResource::CountablePoolResource(std::pmr::memory_resource *upstream) : upstream(upstream) {}

MKLISP_API void *CountablePoolResource::do_allocate(size_t bytes, size_t alignment) {
	void *p = upstream->allocate(bytes, alignment);

	szAllocated += bytes;

	return p;
}

MKLISP_API void CountablePoolResource::do_deallocate(void *p, size_t bytes, size_t alignment) {
	upstream->deallocate(p, bytes, alignment);

	szAllocated -= bytes;
}

MKLISP_API bool CountablePoolResource::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
	return this == &other;
}

MKLISP_API Context::Context(Runtime* runtime) : runtime(runtime), frameList(&runtime->globalHeapResource), bindings(&runtime->globalHeapResource) {
}

MKLISP_API Runtime::Runtime(std::pmr::memory_resource *upstream)
	: globalHeapResource(upstream),
	  createdObjects(&globalHeapResource) {
}

MKLISP_API Runtime::~Runtime() {
}

MKLISP_API Value Runtime::evalList(Context *context) {
recurse:
	Frame &frame = context->frameList.back();
	ListObject *curEvalListObject = frame.curEvalList;

	Value evaluatedValue;

	// Evaluate all elements first.
	if (frame.curEvalIndex < curEvalListObject->elements.size()) {
		Value &curElement = curEvalListObject->elements[frame.curEvalIndex];

		switch (curElement.valueType) {
			case ValueType::Nil:
			case ValueType::Int:
			case ValueType::UInt:
			case ValueType::Long:
			case ValueType::ULong:
			case ValueType::Short:
			case ValueType::UShort:
			case ValueType::Byte:
			case ValueType::UByte:
			case ValueType::Char:
				evaluatedValue = curElement;
				++frame.curEvalIndex;
				goto recurse;
			case ValueType::QuotedObject:
				evaluatedValue = curElement;
				evaluatedValue.valueType = ValueType::Object;
				++frame.curEvalIndex;
				goto recurse;
			case ValueType::Object: {
				Object *object = curElement.exData.asObject;
				switch (object->getObjectType()) {
					case ObjectType::String:
						evaluatedValue = curElement;
						++frame.curEvalIndex;
						goto recurse;
					case ObjectType::Symbol:
						evaluatedValue = curElement;
						++frame.curEvalIndex;
						goto recurse;
					case ObjectType::List: {
						context->frameList.push_back({ (ListObject *)object });
						goto recurse;
					}
				}
			}
		}
	}

	if (curEvalListObject->elements.size()) {
		Value firstElement = curEvalListObject->elements[0];

		switch (firstElement.valueType) {
			case ValueType::Object: {
				Object *object = firstElement.exData.asObject;
				switch (object->getObjectType()) {
					case ObjectType::Symbol: {
						SymbolObject *symbolObject = (SymbolObject *)object;

						if (auto it = context->bindings.find(symbolObject->name); it != context->bindings.end()) {
							switch (it->second->getObjectType()) {
								case ObjectType::List:
									context->frameList.push_back({ (ListObject *)it->second });
									goto recurse;
								case ObjectType::NativeFn:
									((NativeFnObject *)it->second)->callback(context);
									evaluatedValue = frame.returnValue;
									break;
								default:
									terminate();
							}
						} else {
							terminate();
						}
						break;
					}
					default:
						terminate();
				}
				break;
			}
			default:
				// Target is uncallable.
				terminate();
		}
	}

	context->frameList.pop_back();

	if (context->frameList.size()) {
		auto &lastFrame = context->frameList.back();
		lastFrame.curEvalList->elements[lastFrame.curEvalIndex] = evaluatedValue;
		++lastFrame.curEvalIndex;
		goto recurse;
	}

	return evaluatedValue;
}

MKLISP_API Value Runtime::eval(Value value, Context *context) {
	Value returnValue;

	switch (value.valueType) {
		case ValueType::Nil:
		case ValueType::Int:
		case ValueType::UInt:
		case ValueType::Long:
		case ValueType::ULong:
		case ValueType::Short:
		case ValueType::UShort:
		case ValueType::Byte:
		case ValueType::UByte:
		case ValueType::Char:
		case ValueType::QuotedObject:
			returnValue = value;
			break;
		case ValueType::Object: {
			Object *object = value.exData.asObject;
			switch (object->getObjectType()) {
				case ObjectType::String:
					returnValue = value;
					break;
				case ObjectType::Symbol:
					returnValue = value;
					break;
				case ObjectType::List: {
					ListObject *listObject = (ListObject *)object;

					context->frameList.push_back({ listObject });
					returnValue = evalList(context);
				}
			}
		}
	}

	return returnValue;
}
