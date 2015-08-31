// Begin of the file
#ifndef _STRING_MATCHER_H_
#define _STRING_MATCHER_H_

// This file will be included only once by the compiler in a build.
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <list>

#include "aos/String.h"
#include "aos/SList.h"

namespace aos {
	
class Char_Node
{
public:
	enum
	{
		Match = 0x01
	};

public:
	Char_Node(unsigned char ch = 0, unsigned char flag = 0);
	~Char_Node();

public:
	int sizeof_class() const
	{
		return sizeof(Char_Node);
	};
	char ch() const
	{
		return ch_;
	};
	unsigned flag() const
	{
		return flag_;
	};
	int size() const
	{
		return n_node_;
	};
	int empty() const
	{
		return (this->size() == 0);
	};
	Char_Node* root() const // first element/root, 0 for empty
	{
		return node_;
	};
	Char_Node* find(unsigned char ch) const // if node not found, return 0;
	{
		int insert;
		return this->find_(ch, insert);
	};

public:
	Char_Node* set(unsigned char ch);
	void unset(unsigned char ch);
	void clear();
	void flag(unsigned char flag)
	{
		flag_ = flag;
	};
	void shrink(); // optimize size after unset();
	// serialize() / deserialize()

public:
	void dump() const;

protected:
	Char_Node* find_(unsigned char ch, int& insert) const; // if node not found, return 0;

protected:
	unsigned char ch_;
	unsigned char flag_;
	short n_node_;
	Char_Node* node_;
};

template< class T >
class Char_Node_T
{
public:
	Char_Node_T(unsigned char ch = 0, unsigned char flag = 0);
	~Char_Node_T();

public:
	int sizeof_class() const
	{
		return sizeof(Char_Node_T< T >);
	};
	char ch() const
	{
		return ch_;
	};
	unsigned flag() const
	{
		return flag_;
	};
	int size() const
	{
		return n_node_;
	};
	int empty() const
	{
		return (this->size() == 0);
	};
	Char_Node_T< T >* root() const // first element/root, 0 for empty
	{
		return node_;
	};
	Char_Node_T< T >* find(unsigned char ch) const // if node not found, return 0;
	{
		int insert;
		return this->find_(ch, insert);
	};

public:
	Char_Node_T< T >* set(unsigned char ch);
	void unset(unsigned char ch);
	void clear();
	void flag(unsigned char flag)
	{
		flag_ = flag;
	};
	void shrink(); // optimize size after unset();
	// serialize() / deserialize()

public:
	const T& data() const
	{
		return data_;
	};
	void data(const T& data)
	{
		data_ = data;
	};

public:
	void dump() const;

protected:
	Char_Node_T< T >* find_(unsigned char ch, int& insert) const; // if node not found, return 0;

protected:
	unsigned char ch_;
	unsigned char flag_;
	short n_node_;
	Char_Node_T< T >* node_;
	T data_;
};

// class Char_Node_T

template< class T >
Char_Node_T< T >::Char_Node_T(unsigned char ch, unsigned char flag)
:
ch_(ch),
flag_(flag),
n_node_(0),
node_(0)
{
}

template< class T >
Char_Node_T< T >::~Char_Node_T()
{
	clear();
}

template< class T >
Char_Node_T< T >*
Char_Node_T< T >::find_(unsigned char ch, int& insert) const  // if node not found, return 0;
{
	if ( node_ == 0 ) return node_;
	int low = 0, mid, high = n_node_-1;
	Char_Node_T< T >* pn;

	while( low <= high )
	{
		mid = (low + high)/2;
		pn = node_ + mid;

		if ( ch == pn->ch_ )
		{
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

	insert = low;
	return 0;
}

template< class T >
Char_Node_T< T >*
Char_Node_T< T >::set(unsigned char ch)
{
	int insert;
	Char_Node_T< T >* pn = this->find_(ch, insert);
	if ( pn )
	{
	}
	else
	{
		if ( node_ )
		{
			node_ = (Char_Node_T< T >*) ::realloc(node_, (n_node_+1) * sizeof_class());
			if ( node_ == 0 ) return node_;
			pn = (node_ + insert);

			for(int i = n_node_; i > insert; --i)
			{
				::memcpy(node_+i, node_+i-1, sizeof_class());
			}
		}
		else
		{
			node_ = (Char_Node_T< T >*) ::malloc(sizeof_class());
			if ( node_ == 0 ) return node_;
			pn = node_;
		}
		
		pn->ch_ = ch;
		pn->flag_ = 0;
		pn->n_node_ = 0;
		pn->node_ = 0;

		++n_node_;
	}

	return pn;
}

template< class T >
void
Char_Node_T< T >::unset(unsigned char ch)
{
	Char_Node_T< T >* pn = this->find(ch);
	if ( pn )
	{
		if ( pn->n_node_ ) pn->clear();

		Char_Node* pend = node_+n_node_;
		while( pn < pend )
		{
			Char_Node* next = pn+1;
			::memcpy(pn, next, sizeof_class());
			pn = next;
		}
		--n_node_;
	}
}

template< class T >
void
Char_Node_T< T >::shrink()
{
	node_ = (Char_Node_T< T >*) ::realloc(node_, (n_node_) * sizeof_class());
}

template< class T >
void
Char_Node_T< T >::clear()
{
	Char_Node_T< T >* pn = node_;
	Char_Node_T< T >* pend = node_+n_node_;
	while( pn < pend )
	{
		pn->clear();
		++pn;
	}

	::free(node_);
	n_node_ = 0;
	node_ = 0;
}

template< class T >
void
Char_Node_T< T >::dump() const
{
	if ( this->empty() )
	{
		::printf("[empty]\r\n");
		return;
	}
	else
	{
		Char_Node_T< T >* pn = node_;
		for(int i = 0; i < n_node_; ++i, ++pn)
		{
			::printf("[%d] %c\r\n", (int) pn->ch_, pn->ch_);
		}
	}
}

// hint: can keep the shortest key length (must >= 2) for faster match

class Char_Matcher //? rename to String/Char_Searcher/Matcher
{
public:
	struct Result
	{
		Char_Node* curr;
		const char* head;
	};
	typedef std::list< Char_Matcher::Result* > Matches;
	//typedef aos::SList< Char_Indexer::Result* > Match;

protected:
	Char_Node cnode_[256];
	Matches matches_;
	// size_t min_key_size_; //@ shortest key length;
};

/*//

Char_Indexer cindexer;

for(int n = 0; n < 10000000; ++n)
{
	char buf[256];
	_itoa(n, buf, 10);
	cindexer.insert_key(buf);
}

cindexer.insert_key("ABCDE");
cindexer.insert_key("ABCD");
cindexer.insert_key("BCD");
cindexer.insert_key("BCE");
cindexer.insert_key("SX");

int i = 0;
while( *s )
{
	cindexer.move(s);
	s++;
	i++;
}

//*/

class Char_Indexer
{
public:
	static int count;
	class Result
	{
	public:
		Char_Node* curr;
		const char* head;
	};

	typedef std::list< Char_Indexer::Result* > Matches;
	//typedef aos::SList< Char_Indexer::Result* > Matches;

public:
	void search(const char* cstr);
	void search(const char* buf, size_t len);
	Char_Node* move(const char* c);
	Char_Node* move2(const char* c);
	void insert_key(const char* cstr);
	void insert_key(const char* buf, size_t size);

	virtual void on_key_matched(const char* str, size_t size)
	{
		std::string matched(str, size);
		Char_Indexer::count++;
		//::printf("index str: %s\r\n", matched.c_str()); //@
	};

protected:
	Char_Node root_;
	Matches match_;
};

} // namespace aos

#endif // _STRING_MATCHER_H_
