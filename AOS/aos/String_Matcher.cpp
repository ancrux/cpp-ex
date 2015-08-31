#include "String_Matcher.h"

#include <assert.h>

namespace aos {
	
// class Char_Node

Char_Node::Char_Node(unsigned char ch, unsigned char flag)
:
ch_(ch),
flag_(flag),
n_node_(0),
node_(0)
{
}

Char_Node::~Char_Node()
{
	clear();
}

Char_Node*
Char_Node::find_(unsigned char ch, int& insert) const  // if node not found, return 0;
{
	if ( node_ == 0 ) return node_;
	int low = 0, mid, high = n_node_-1;
	Char_Node* pn;

	while( low <= high )
	{
		mid = (low + high)/2;
		pn = node_ + mid;

		if ( ch == pn->ch_ )
		{
			//insert = mid; // this value = (pn - this->root())
			return pn;
		}
		else if ( ch < pn->ch_ )
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}
	}

	insert = low; // insert position if not found
	return 0;
}

Char_Node*
Char_Node::set(unsigned char ch)
{
	int insert; // insert position if node not found
	Char_Node* pn = this->find_(ch, insert);
	if ( pn )
	{
		// if exists, do nothing and simply return pn;
	}
	else
	{
		// add a node
		if ( node_ )
		{
			node_ = (Char_Node*) ::realloc(node_, (n_node_+1) * sizeof_class());
			if ( node_ == 0 ) return node_;
			pn = (node_ + insert);

			// shift copy
			for(int i = n_node_; i > insert; --i)
			{
				::memcpy(node_+i, node_+i-1, sizeof_class());
			}
			// */

			// block copy, required extra temp memory for copy
			/*
			size_t n_block= (n_node_ - insert) * sizeof_class();
			void* block = ::malloc(n_block);
			if ( !block ) return 0;
			::memcpy(block, pn, n_block);
			::memcpy(pn+1, block, n_block);
			::free(block);
			// */
		}
		else
		{
			node_ = (Char_Node*) ::malloc(sizeof_class());
			if ( node_ == 0 ) return node_;
			pn = node_;
		}
		
		// if not exists, create a node without children
		pn->ch_ = ch;
		pn->flag_ = 0;
		pn->n_node_ = 0;
		pn->node_ = 0;

		++n_node_;
	}

	return pn;
}

void
Char_Node::unset(unsigned char ch)
{
	Char_Node* pn = this->find(ch);
	if ( pn )
	{
		// delete a node, and reset its child nodes
		if ( pn->n_node_ ) pn->clear();

		Char_Node* pend = node_+n_node_;
		while( pn < pend )
		{
			Char_Node* next = pn+1;
			::memcpy(pn, next, sizeof_class());
			pn = next;
		}
		// not free node right away for better efficiency?
		--n_node_;
	}
}

void
Char_Node::shrink()
{
	node_ = (Char_Node*) ::realloc(node_, (n_node_) * sizeof_class());
}

void
Char_Node::clear()
{
	// clear nodes in child nodes first
	Char_Node* pn = node_;
	Char_Node* pend = node_+n_node_;
	while( pn < pend )
	{
		pn->clear();
		++pn;
	}

	// clear nodes
	::free(node_);
	n_node_ = 0;
	node_ = 0;
	
	//::printf("[%c] cleared!\r\n", this->ch_); //@
}

void
Char_Node::dump() const
{
	if ( this->empty() )
	{
		::printf("[empty]\r\n");
		return;
	}
	else
	{
		Char_Node* pn = node_;
		for(int i = 0; i < n_node_; ++i, ++pn)
		{
			::printf("[%d] %c\r\n", (int) pn->ch_, pn->ch_);
		}
	}
}

int
Char_Indexer::count = 0;

void
Char_Indexer::insert_key(const char* cstr)
{
	Char_Node* node = &root_;
	while( *cstr )
	{
		node = node->set(*cstr);
		++cstr;
	}
	node->flag(Char_Node::Match);
}

void
Char_Indexer::insert_key(const char* buf, size_t size)
{
	Char_Node* node = &root_;
	const char* ptr = buf;
	const char* end = buf + size;
	while( ptr < end )
	{
		node = node->set(*ptr);
		++ptr;
	}
	node->flag(Char_Node::Match);
}

void
Char_Indexer::search(const char* cstr)
{
char* c = (char*) cstr;
while( *c )
{
	Char_Node* new_match = root_.find(*c);
	if ( !new_match && match_.empty() ) continue;

	Matches::iterator iter = match_.begin();
	while( iter != match_.end() )
	{
		Char_Node* next_match = (*iter)->curr->find(*c);
		if ( next_match )
		{
			(*iter)->curr = next_match;
			if ( ((*iter)->curr->flag() & Char_Node::Match) == Char_Node::Match )
			{
				on_key_matched((*iter)->head, c - (*iter)->head + 1);
			}
			++iter;
		}
		else
		{
			delete (*iter);
			match_.erase(iter++); // for std::list
			//match_.erase(iter); // for aos::SList
		}
	}

	if ( new_match )
	{
		Result* new_result = new (std::nothrow) Result;
		new_result->curr = new_match;
		new_result->head = c;
		match_.push_front(new_result);

		if ( (new_result->curr->flag() & Char_Node::Match) == Char_Node::Match )
		{
			on_key_matched(new_result->head, c - new_result->head + 1);
		}
	}

	c++;
}
}

void
Char_Indexer::search(const char* buf, size_t len)
{
char* c = (char*) buf;
char* c_end = c+len;
while( c < c_end )
{
	Char_Node* new_match = root_.find(*c);
	if ( !new_match && match_.empty() ) continue;

	Matches::iterator iter = match_.begin();
	while( iter != match_.end() )
	{
		Char_Node* next_match = (*iter)->curr->find(*c);
		if ( next_match )
		{
			(*iter)->curr = next_match;
			if ( ((*iter)->curr->flag() & Char_Node::Match) == Char_Node::Match )
			{
				on_key_matched((*iter)->head, c - (*iter)->head + 1);
			}
			++iter;
		}
		else
		{
			delete (*iter);
			match_.erase(iter++); // for std::list
			//match_.erase(iter); // for aos::SList
		}
	}

	if ( new_match )
	{
		Result* new_result = new (std::nothrow) Result;
		new_result->curr = new_match;
		new_result->head = c;
		match_.push_front(new_result);

		if ( (new_result->curr->flag() & Char_Node::Match) == Char_Node::Match )
		{
			on_key_matched(new_result->head, c - new_result->head + 1);
		}
	}

	c++;
}
}

Char_Node*
Char_Indexer::move(const char* c)
{
	Char_Node* new_match = root_.find(*c);
	if ( !new_match && match_.empty() ) return 0;

	Matches::iterator iter = match_.begin();
	while( iter != match_.end() )
	{
		Char_Node* next_match = (*iter)->curr->find(*c);
		if ( next_match )
		{
			(*iter)->curr = next_match;
			if ( ((*iter)->curr->flag() & Char_Node::Match) == Char_Node::Match )
			{
				on_key_matched((*iter)->head, c - (*iter)->head + 1);
			}
			++iter;
		}
		else
		{
			delete (*iter);
			match_.erase(iter++); // for std::list
			//match_.erase(iter); // for aos::SList
		}
	}

	if ( new_match )
	{
		Result* new_result = new (std::nothrow) Result;
		new_result->curr = new_match;
		new_result->head = c;
		match_.push_front(new_result);

		if ( (new_result->curr->flag() & Char_Node::Match) == Char_Node::Match )
		{
			on_key_matched(new_result->head, c - new_result->head + 1);
		}
	}

	return 0;
}

Char_Node*
Char_Indexer::move2(const char* c)
{
	Char_Node* match0 = root_.find(*c);
	if ( !match0 && match_.empty() ) return 0;

	if ( match0 && ((match0->flag() & Char_Node::Match) == Char_Node::Match) )
	{
		on_key_matched(c, 1);
	}

	Matches::iterator iter = match_.begin();
	while( iter != match_.end() )
	{
		Char_Node* next_match = (*iter)->curr->find(*(c+1));
		if ( next_match )
		{
			(*iter)->curr = next_match;
			if ( ((*iter)->curr->flag() & Char_Node::Match) == Char_Node::Match )
			{
				on_key_matched((*iter)->head, (c+1) - (*iter)->head + 1);
			}
			++iter;
		}
		else
		{
			delete (*iter);
			match_.erase(iter++); // for std::list
			//match_.erase(iter); // for aos::SList
		}
	}

	if ( match0 )
	{
		Char_Node* match1 = match0->find(*(c+1));

		if ( match1 )
		{
			Result* new_result = new Result;
			new_result->curr = match1;
			new_result->head = c;
			match_.push_front(new_result);

			if ( (new_result->curr->flag() & Char_Node::Match) == Char_Node::Match )
			{
				on_key_matched(new_result->head, 2);
			}
		}
	}

	return 0;
}

} // namespace aos
