#ifndef _MKLISP_UTIL_H_
#define _MKLISP_UTIL_H_

namespace mklisp {
	template <typename T>
	struct DeallocableDeleter {
		void operator()(T *ptr) {
			if (ptr)
				ptr->dealloc();
		}
	};

	template <typename Alloc>
	struct StatefulDeleter {
		Alloc allocator;

		StatefulDeleter(const Alloc &allocator) : allocator(allocator) {
		}
		StatefulDeleter(Alloc &&allocator) : allocator(allocator) {
		}
		void operator()(typename Alloc::value_type *ptr) {
			if (ptr)
				allocator.deallocate(ptr, 1);
		}
	};
}

#endif
