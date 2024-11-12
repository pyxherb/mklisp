#ifndef _MKLISP_OBJECTS_H_
#define _MKLISP_OBJECTS_H_

#include "value.h"
#include "util.h"
#include <string>
#include <memory_resource>
#include <list>
#include <atomic>
#include <set>
#include <deque>

namespace mklisp {
	enum class ObjectType : uint8_t {
		String = 0,
		Symbol,
		List,
		NativeFn
	};

	class Runtime;

	class Object {
	public:
		Runtime *associatedRuntime;
		std::atomic_size_t hostRefCount;

		MKLISP_API Object(Runtime *associatedRuntime);
		MKLISP_API virtual ~Object();

		virtual ObjectType getObjectType() const noexcept = 0;
		virtual void dealloc() noexcept = 0;
	};

	template <typename T = Object>
	class HostObjectRef final {
	public:
		T *_value = nullptr;

		MKLISP_FORCEINLINE void reset() {
			if (_value) {
				--_value->hostRefCount;
				_value = nullptr;
			}
		}

		MKLISP_FORCEINLINE T *release() {
			T *v = _value;
			--_value->hostRefCount;
			_value = nullptr;
			return v;
		}

		MKLISP_FORCEINLINE void discard() noexcept { _value = nullptr; }

		MKLISP_FORCEINLINE HostObjectRef(const HostObjectRef<T> &x) : _value(x._value) {
			if (x._value) {
				++_value->hostRefCount;
			}
		}
		MKLISP_FORCEINLINE HostObjectRef(HostObjectRef<T> &&x) noexcept : _value(x._value) {
			if (x._value) {
				x._value = nullptr;
			}
		}
		MKLISP_FORCEINLINE HostObjectRef(T *value = nullptr) noexcept : _value(value) {
			if (_value) {
				++_value->hostRefCount;
			}
		}
		MKLISP_FORCEINLINE ~HostObjectRef() {
			reset();
		}

		MKLISP_FORCEINLINE const T *get() const { return _value; }
		MKLISP_FORCEINLINE T *get() { return _value; }
		MKLISP_FORCEINLINE const T *operator->() const { return _value; }
		MKLISP_FORCEINLINE T *operator->() { return _value; }

		MKLISP_FORCEINLINE HostObjectRef<T> &operator=(const HostObjectRef<T> &x) {
			reset();

			if ((_value = x._value)) {
				++_value->hostRefCount;
			}

			return *this;
		}
		MKLISP_FORCEINLINE HostObjectRef<T> &operator=(HostObjectRef<T> &&x) noexcept {
			reset();

			if ((_value = x._value)) {
				x._value = nullptr;
			}

			return *this;
		}

		MKLISP_FORCEINLINE HostObjectRef<T> &operator=(T *other) {
			reset();

			if ((_value = other)) {
				++_value->hostRefCount;
			}

			return *this;
		}

		MKLISP_FORCEINLINE bool operator<(const HostObjectRef<T> &rhs) const noexcept {
			return _value < rhs._value;
		}
		MKLISP_FORCEINLINE bool operator>(const HostObjectRef<T> &rhs) const noexcept {
			return _value > rhs._value;
		}
		MKLISP_FORCEINLINE bool operator==(const HostObjectRef<T> &rhs) const noexcept {
			return _value == rhs._value;
		}

		MKLISP_FORCEINLINE operator bool() const {
			return _value;
		}
	};

	class HostRefHolder final {
	public:
		std::pmr::set<Object *> holdedObjects;

		MKLISP_API HostRefHolder(
			std::pmr::memory_resource *memoryResource =
				std::pmr::get_default_resource());
		MKLISP_API ~HostRefHolder();

		MKLISP_API void addObject(Object *object);
		MKLISP_API void removeObject(Object *object) noexcept;
	};

	class StringObject : public Object {
	public:
		std::pmr::string data;

		MKLISP_API StringObject(Runtime *runtime, std::pmr::string &&data);
		MKLISP_API virtual ~StringObject();

		MKLISP_API virtual ObjectType getObjectType() const noexcept override;
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static HostObjectRef<StringObject> alloc(Runtime *runtime, std::pmr::string &&data);
	};

	class SymbolObject : public Object {
	public:
		std::pmr::string name;

		MKLISP_API SymbolObject(Runtime *runtime, std::pmr::string &&name);
		MKLISP_API virtual ~SymbolObject();

		MKLISP_API virtual ObjectType getObjectType() const noexcept override;
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static HostObjectRef<SymbolObject> alloc(Runtime *runtime, std::pmr::string &&name);
	};

	class ListObject : public Object {
	public:
		std::pmr::deque<Value> elements;

		MKLISP_API ListObject(Runtime *runtime);
		MKLISP_API virtual ~ListObject();

		MKLISP_API virtual ObjectType getObjectType() const noexcept override;
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static HostObjectRef<ListObject> alloc(Runtime *runtime);
	};

	struct Context;

	typedef void (*NativeFnCallback)(Context *context);
	class NativeFnObject : public Object {
	public:
		NativeFnCallback callback;

		MKLISP_API NativeFnObject(Runtime *runtime, NativeFnCallback callback);
		MKLISP_API virtual ~NativeFnObject();

		MKLISP_API virtual ObjectType getObjectType() const noexcept override;
		MKLISP_API virtual void dealloc() noexcept override;

		MKLISP_API static HostObjectRef<NativeFnObject> alloc(Runtime *runtime, NativeFnCallback callback);
	};
}

#endif
