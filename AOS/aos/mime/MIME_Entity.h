#ifndef _MIME_ENTITY_H_
#define _MIME_ENTITY_H_

#include "aos/Config.h"

#include <string>
#include <list>

namespace aos {

class MIME_Entity;

typedef std::list< std::string* > MIME_Header_List;
typedef std::list< MIME_Entity* > MIME_Entity_List;

class AOS_API MIME_Entity
{
	friend class MIME_Util;

public:
	static const char HIDDEN_HEADER_CHAR = ':';
	static const std::string NULL_STRING;

public: // MIME parser state
	class Parse
	{
	public:
		enum
		{
			HEADER = 0,
			HEADER_CRLF,
			HEADER_DONE,
			BOUNDARY,
			BOUNDARY_END
		};
	};

public: // MIME status (encode/decode)
	class Status
	{
	public:
		enum 
		{ 
			BODY_ENCODED = 0x01
		};
	};

public: // MIME process flag
	class Flag
	{
	public:
		enum
		{
			ALL = 0,
			HEADER,
			NO_BODY // Structure Only
		};
	};

public: // ctor/dtor
	MIME_Entity();
	virtual ~MIME_Entity();
	void reset();

public: // header 
	MIME_Header_List& header() { return header_; };

	// use iterator.base() to convert from ReverseIterator to Iterator
	bool empty_header() const { return header_.empty(); };
	bool has_header() const { return !empty_header(); };
	size_t n_header() const { return header_.size(); };

	// raw header functions
	void insert_header(std::string* field, MIME_Header_List::iterator it) { if ( field ) header_.insert(it, field); };
	void head_header(std::string* field) { if ( field ) header_.push_front(field); };
	void tail_header(std::string* field) { if ( field ) header_.push_back(field); };
	void append_header(std::string* field) { this->tail_header(field); };
	void remove_header(MIME_Header_List::iterator it) { delete *it; header_.erase(it); };
	void clear_header();

	// header functions
	MIME_Header_List::iterator find_header(const char* name, MIME_Header_List::iterator pos, int case_sensitive = 0);
	void add_first_header(std::string* field);
	void add_last_header(std::string* field);
	void add_header(std::string* field, MIME_Header_List::iterator it) { insert_header(field, it); }

public: // header utils
	int is_header_completed();
	std::string get_parameter(const std::string& header, const char* name);
	std::string get_content_type();
	std::string get_primary_type();
	std::string get_sub_type();
	std::string get_boundary();
	std::string get_charset();
	std::string get_encoding();

	int set_charset(const char* charset);
	int set_encoding(const char* encoding);

public: // preamble
	std::string& preamble()
	{
		if ( !preamble_  ) preamble_ = new (std::nothrow) std::string();
		return ( preamble_ )?( *preamble_ ):(std::string&) NULL_STRING; // return *preamble_;
	};

public: // body
	std::string& body()
	{ 
		if ( !body_ ) body_ = new (std::nothrow) std::string();
		return ( body_ )?( *body_ ):(std::string&) NULL_STRING; // return *body_;
	};
	void attach_body(std::string* body)
	{
		if ( body )
		{
			delete body_;
			body_ = body;
		}
	};
	std::string* detach_body()
	{
		std::string* body = 0;
		if ( body_ )
		{
			body = body_;
			body_ = 0;
		}

		return body;
	};
	int decode_body();
	int encode_body();
	void copy_decoded_body(std::string& str);
	void copy_encoded_body(std::string& str);

public: // epilogue
	std::string& epilogue()
	{
		if ( !epilogue_ ) epilogue_ = new (std::nothrow) std::string();
		return ( epilogue_ )?( *epilogue_ ):(std::string&) NULL_STRING; // return *epilogue_;
	};

public: // child
	MIME_Entity_List& child() { return child_; };

	// use iterator.base() to convert from ReverseIterator to Iterator
	bool empty_child() const { return child_.empty(); };
	bool has_child() const { return !empty_child(); };
	size_t n_child() const { return child_.size(); };

	void insert_child(MIME_Entity* entity, MIME_Entity_List::iterator it) { if ( entity ) { child_.insert(it, entity); entity->parent_ = this; } };
	void head_child(MIME_Entity* entity) { if ( entity ) { child_.push_front(entity); entity->parent_ = this; } };
	void tail_child(MIME_Entity* entity) { if ( entity ) { child_.push_back(entity); entity->parent_ = this; } };
	void append_child(MIME_Entity* entity) { this->tail_child(entity); };
	void remove_child(MIME_Entity_List::iterator it) { delete *it; child_.erase(it); };
	void clear_child();

public: // parent
	MIME_Entity* parent() const { return this->parent_; };
	void parent(MIME_Entity* parent) { this->parent_ = parent; };

public: // size
	size_t size(bool no_hidden_header = true);

public: // MIME Import
	int import_data(const char* buf, size_t len, int flag = Flag::ALL, bool no_hidden_header = true);
	int import_file(const char* filename, int flag = Flag::ALL, bool no_hidden_header = true);
	
public: // MIME Export
	int dump_data(std::string& data, int flag = Flag::ALL, bool no_hidden_header = true);
	int export_data(std::string& data, int flag = Flag::ALL, bool no_hidden_header = true);
	int dump_file(FILE* fp, int flag = Flag::ALL, bool no_hidden_header = true);
	int export_file(const char* filename, int flag = Flag::ALL, bool no_hidden_header = true);

public: // MIME Dump
	void dump();
	void dump_header();
	void dump_body();

public: // MIME Parser
	int parse_line(std::string& line, bool no_hidden_header = true, bool with_body = true);
	int parse_header_line(std::string& line, bool no_hidden_header = true);
	int parse_content_line(std::string& line, bool no_hidden_header = true, bool with_body = true);

protected: // method
	void init_(); // class initialization
	void fini_(); // class finalization
	int status() const { return (int) status_; };
	void status(int status) const { status_ = (unsigned char) status; };

protected:
	MIME_Header_List header_;
	std::string* preamble_;
	std::string* body_;
	std::string* epilogue_;
	MIME_Entity_List child_;
	MIME_Entity* parent_;
	mutable unsigned char status_; // status: Status::BODY_ENCODED //? maybe add: isContentLoaded, isFullyLoaded, isPartialLoaded

// MIME parser
protected:
	int pstate_;
	std::string separator_; // boundary separator
};

} // namespace aos

#endif // _MIME_ENTITY_H_
