#ifndef _ASIO_TCP_ASYNC_HANDLER_ALLOCATOR_H_
#define _ASIO_TCP_ASYNC_HANDLER_ALLOCATOR_H_

#define USE_NEDMALLOC 0
#if ( USE_NEDMALLOC == 1 )
//#define REPLACE_SYSTEM_ALLOCATOR
#include "nedmalloc.h"
#endif

namespace asio {

// Class to manage the memory to be used for handler-based custom allocation.
// It contains a single block of memory which may be returned for allocation
// requests. If the memory is in use when an allocation request is made, the
// allocator delegates allocation to the global heap.
class handler_allocator
{
public:
	handler_allocator()
	{
	}

	void* allocate(std::size_t size)
	{
#if ( USE_NEDMALLOC == 1 ) && !defined REPLACE_SYSTEM_ALLOCATOR
		return nedalloc::nedmalloc(size);
#else
		return ::operator new(size);
#endif
	}

	void deallocate(void* pointer)
	{
#if ( USE_NEDMALLOC == 1 ) && !defined REPLACE_SYSTEM_ALLOCATOR
		nedalloc::nedfree(pointer);
#else
		::operator delete(pointer);
#endif
	}
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler
{
public:
	custom_alloc_handler(handler_allocator& a, Handler h)
		: allocator_(a),
		handler_(h)
	{
	}

	template <typename Arg1>
	void operator()(Arg1 arg1)
	{
		handler_(arg1);
	}

	template <typename Arg1, typename Arg2>
	void operator()(Arg1 arg1, Arg2 arg2)
	{
		handler_(arg1, arg2);
	}

	friend void* asio_handler_allocate(std::size_t size,
		custom_alloc_handler<Handler>* this_handler)
	{
		return this_handler->allocator_.allocate(size);
	}

	friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/,
		custom_alloc_handler<Handler>* this_handler)
	{
		this_handler->allocator_.deallocate(pointer);
	}

private:
	handler_allocator& allocator_;
	Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
	handler_allocator& a, Handler h)
{
	return custom_alloc_handler<Handler>(a, h);
}

} // namespace asio

#endif // _ASIO_SSL_ASYNC_HANDLER_ALLOCATOR_H_
