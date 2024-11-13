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

MKLISP_API EvalStateExData::EvalStateExData() {
}

MKLISP_API Context::Context(Runtime *runtime) : runtime(runtime), frameList(&runtime->globalHeapResource), bindings(&runtime->globalHeapResource) {
}

MKLISP_API Runtime::Runtime(std::pmr::memory_resource *upstream)
	: globalHeapResource(upstream),
	  createdObjects(&globalHeapResource) {
}

MKLISP_API Runtime::~Runtime() {
}

MKLISP_API Value Runtime::evalList(Context *context) {
	while (true) {
		Frame &curFrame = context->frameList.back();
		switch (curFrame.evalState) {
			case EvalState::Initial: {
				Value callTarget = curFrame.curEvalList->elements[0];

				switch (callTarget.valueType) {
					case ValueType::Object: {
						Object *object = callTarget.exData.asObject;
						switch (object->getObjectType()) {
							case ObjectType::Symbol: {
								SymbolObject *symbolObject = (SymbolObject *)object;

								if (symbolObject->name == "if") {
								} else {
									// Evaluate all arguments first.
									curFrame.evalStateExData.asEvalArgs.index = 1;
									curFrame.evalStateExData.asEvalArgs.callTarget = context->bindings.at(symbolObject->name);
									curFrame.evalState = EvalState::EvalArgs;
									continue;
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
				break;
			}
			case EvalState::EvalArgs: {
				size_t &curIndex = curFrame.evalStateExData.asEvalArgs.index;
				if (curIndex < curFrame.curEvalList->elements.size()) {
					Value &curElement = curFrame.curEvalList->elements[curIndex];

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
							curFrame.curEvalList->elements[curIndex] = curElement;
							++curIndex;
							continue;
						case ValueType::QuotedObject:
							curElement.valueType = ValueType::Object;
							curFrame.curEvalList->elements[curIndex] = curElement;
							++curIndex;
							continue;
						case ValueType::Object: {
							Object *object = curElement.exData.asObject;
							switch (object->getObjectType()) {
								case ObjectType::String:
									curFrame.curEvalList->elements[curIndex] = curElement;
									++curIndex;
									continue;
								case ObjectType::Symbol:
									curFrame.curEvalList->elements[curIndex] = curElement;
									++curIndex;
									continue;
								case ObjectType::List: {
									curFrame.evalState = EvalState::ReceivingEvaluatedArg;
									Frame newFrame;
									newFrame.curEvalList = (ListObject *)object;
									newFrame.evalState = EvalState::Initial;
									context->frameList.push_back(newFrame);
									continue;
								}
							}
						}
					}
				} else {
					curFrame.evalState = EvalState::Call;
					curFrame.evalStateExData.asCall.callTarget = curFrame.evalStateExData.asEvalArgs.callTarget;
				}
				break;
			}
			case EvalState::ReceivingEvaluatedArg: {
				size_t &curIndex = curFrame.evalStateExData.asEvalArgs.index;

				ListObject *listObject = (ListObject*)curFrame.curEvalList;

				listObject->elements[curIndex] = curFrame.returnValue;

				++curIndex;

				curFrame.evalState = EvalState::EvalArgs;
				break;
			}
			case EvalState::Call: {
				Value callTarget = curFrame.evalStateExData.asCall.callTarget;
				switch (callTarget.valueType) {
					case ValueType::Object: {
						Object *object = callTarget.exData.asObject;
						switch (object->getObjectType()) {
							case ObjectType::NativeFn: {
								((NativeFnObject *)object)->callback(context);
								break;
							}
							default:
								terminate();
						}
						break;
					}
					default:
						terminate();
				}

				Value returnValue = curFrame.returnValue;
				context->frameList.pop_back();
				if (context->frameList.size()) {
					context->frameList.back().returnValue = returnValue;
				} else {
					return returnValue;
				}
			}
		}
	}
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
					Frame newFrame;
					newFrame.curEvalList = (ListObject *)object;
					newFrame.evalState = EvalState::Initial;

					context->frameList.push_back(newFrame);
					returnValue = evalList(context);
				}
			}
		}
	}

	return returnValue;
}
