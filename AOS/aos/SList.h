// Begin of the file
#ifndef _SLIST_H_
#define _SLIST_H_

// This file will be included only once by the compiler in a build.
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cassert>
#include <string>

namespace aos {

template< class T >
struct SList_Node_
{
	SList_Node_< T >* next;
	T data;

	inline SList_Node_< T >* insert_next(SList_Node_< T >* node)
	{
		node->next = this->next;
		this->next = node;
		return node;
	};
};

template< class T >
class SList
{
public:
	struct iterator
	{
		SList_Node_< T >* curr;
		SList_Node_< T >* prev;

		iterator(SList_Node_< T >* curr_node, SList_Node_< T >* prev_node = 0)
		{
			this->curr = curr_node;
			this->prev = prev_node;
		};
		iterator(const iterator& iter)
		{
			this->curr = iter.curr;
			this->prev = iter.prev;
		};
		inline void incr()
		{
			prev = curr;
			curr = curr->next;
		};
		inline iterator& operator++() // pre increment
		{
			incr();
			return *this;
		};
		inline iterator operator++(int) // post increment
		{
			iterator old = *this;
			incr();
			return old;
		};
		inline int operator==(const iterator& iter)  const
		{
			return curr == iter.curr;
		};
		inline int operator!=(const iterator& iter)  const
		{
			return curr != iter.curr;
		};
		inline T& operator*() const
		{
			return curr->data;
		};
		inline T* operator->() const
		{
			return &(operator*()); // equals &(curr->data)
		};
	};

public:
	SList()
	:
	size_(0),
	head_(0)
	{
	};
	~SList()
	{
		clear();
	};

public:
	// insert before iterator, iterator holds its original position
	inline void insert(iterator& iter, const T& data)
	{
		SList_Node_< T >* node = new (std::nothrow) SList_Node_< T >; // SList_Node_< T >* node = (SList_Node_< T >*) ::malloc(sizeof(SList_Node_< T >));
		assert(node);
		node->data = data;

		if ( iter.curr == head_ ) 
		{
			node->next = head_;
			head_ = node;
		}
		else
		{
			node->next = iter.curr;
			(iter.prev)->next = node;
		}
		++size_;
	};
	// erase iterator, iterator moves to the next position
	// Not compliant with STL-style SList::erase(iter++); will possibly cause iter->prev point to a delete object
	// Use SList::erase(iter); and iterator will automatically move to the next item
	inline void erase(iterator& iter)
	{
		SList_Node_< T >* node = iter.curr;
		assert(node);

		if ( iter.curr == head_ ) 
		{
			iter.curr = head_ = node->next;
		}
		else
		{
			iter.curr = (iter.prev)->next = node->next;
		}
		--size_;
		delete node; // ::free(node);
	};
	inline T& front()
	{
		return head_->data;
	};
	inline void pop_front() // equals: this->erase(this->begin());
	{
		assert(head_);
		SList_Node_< T >* node = head_;
		head_ = head_->next;

		--size_;
		delete node; // ::free(node);
	};
	inline void push_front(const T& data) // equals: this->insert(this->begin(), data);
	{
		SList_Node_< T >* node = new (std::nothrow) SList_Node_< T >; // SList_Node_< T >* node = (SList_Node_< T >*) ::malloc(sizeof(SList_Node_< T >));
		assert(node);
		node->data = data;

		node->next = head_;
		head_ = node;
		++size_;
	};
	inline void clear()
	{
		while( head_ ) pop_front();
	};
	inline int empty()
	{
		return ( head_ == 0 );
	};
	inline size_t size()
	{
		return size_;
	};

public:
	inline iterator begin()
	{
		return iterator(head_, head_);
	};
	inline iterator end()
	{
		return iterator(0);
	};

#if defined AOS_DEBUG
protected:
	bool is_end_null()
	{
		SList_Node_< T >* node = head_;
		for(size_t i = 0; i < size_; ++i)
		{
			node = node->next;
		}
		if ( node == 0 ) return true;
		else return false;
	};
#endif


protected:
	size_t size_;
	SList_Node_< T >* head_;
};

} // namespace aos

#endif // _SLIST_H_
