#include "runtime.h"
#include <cassert>

using namespace mklisp;

MKLISP_API Object::Object(Runtime *associatedRuntime) : associatedRuntime(associatedRuntime) {
}

MKLISP_API Object::~Object() {
}

MKLISP_API HostRefHolder::HostRefHolder(std::pmr::memory_resource *memoryResource)
	: holdedObjects(memoryResource) {
}

MKLISP_API HostRefHolder::~HostRefHolder() {
	for (auto i : holdedObjects)
		--i->hostRefCount;
}

MKLISP_API void HostRefHolder::addObject(Object *object) {
	if (!holdedObjects.count(object)) {
		holdedObjects.insert(object);
		++object->hostRefCount;
	}
}

MKLISP_API void HostRefHolder::removeObject(Object *object) noexcept {
	assert(holdedObjects.count(object));
	holdedObjects.erase(object);
	--object->hostRefCount;
}

MKLISP_API StringObject::StringObject(Runtime *runtime, std::pmr::string &&data)
	: Object(runtime), data(data) {
}

MKLISP_API StringObject::~StringObject() {
}

MKLISP_API ObjectType StringObject::getObjectType() const noexcept {
	return ObjectType::String;
}

MKLISP_API void StringObject::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<StringObject>;
	Alloc allocator(&associatedRuntime->globalHeapResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API HostObjectRef<StringObject> StringObject::alloc(Runtime *runtime, std::pmr::string &&data) {
	using Alloc = std::pmr::polymorphic_allocator<StringObject>;
	Alloc allocator(&runtime->globalHeapResource);

	std::unique_ptr<StringObject, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), runtime, std::move(data));

	return ptr.release();
}

MKLISP_API SymbolObject::SymbolObject(Runtime *runtime, std::pmr::string &&name)
	: Object(runtime), name(name) {
}

MKLISP_API SymbolObject::~SymbolObject() {
}

MKLISP_API ObjectType SymbolObject::getObjectType() const noexcept {
	return ObjectType::Symbol;
}

MKLISP_API void SymbolObject::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<SymbolObject>;
	Alloc allocator(&associatedRuntime->globalHeapResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API HostObjectRef<SymbolObject> SymbolObject::alloc(Runtime *runtime, std::pmr::string &&name) {
	using Alloc = std::pmr::polymorphic_allocator<SymbolObject>;
	Alloc allocator(&runtime->globalHeapResource);

	std::unique_ptr<SymbolObject, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), runtime, std::move(name));

	return ptr.release();
}

MKLISP_API ListObject::ListObject(Runtime *runtime)
	: Object(runtime) {
}

MKLISP_API ListObject::~ListObject() {
}

MKLISP_API ObjectType ListObject::getObjectType() const noexcept {
	return ObjectType::List;
}

MKLISP_API void ListObject::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<ListObject>;
	Alloc allocator(&associatedRuntime->globalHeapResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API HostObjectRef<ListObject> ListObject::alloc(Runtime *runtime) {
	using Alloc = std::pmr::polymorphic_allocator<ListObject>;
	Alloc allocator(&runtime->globalHeapResource);

	std::unique_ptr<ListObject, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), runtime);

	return ptr.release();
}

MKLISP_API NativeFnObject::NativeFnObject(Runtime *runtime, NativeFnCallback callback)
	: Object(runtime), callback(callback) {
}

MKLISP_API NativeFnObject::~NativeFnObject() {
}

MKLISP_API ObjectType NativeFnObject::getObjectType() const noexcept {
	return ObjectType::NativeFn;
}

MKLISP_API void NativeFnObject::dealloc() noexcept {
	using Alloc = std::pmr::polymorphic_allocator<NativeFnObject>;
	Alloc allocator(&associatedRuntime->globalHeapResource);

	std::destroy_at(this);
	allocator.deallocate(this, 1);
}

MKLISP_API HostObjectRef<NativeFnObject> NativeFnObject::alloc(Runtime *runtime, NativeFnCallback callback) {
	using Alloc = std::pmr::polymorphic_allocator<NativeFnObject>;
	Alloc allocator(&runtime->globalHeapResource);

	std::unique_ptr<NativeFnObject, StatefulDeleter<Alloc>> ptr(
		allocator.allocate(1),
		StatefulDeleter<Alloc>(allocator));
	allocator.construct(ptr.get(), runtime, callback);

	return ptr.release();
}
