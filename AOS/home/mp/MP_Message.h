#ifndef _MP_MESSAGE_H_
#define _MP_MESSAGE_H_

#include "aos/String.h"

class MP_Message : public std::string
{
public:
	explicit MP_Message();
	MP_Message(const std::string& str, int type = 0);
	MP_Message(const char* cstr, int type = 0);
	~MP_Message();

public:
	int type() const { return type_; };
	void type(int type) { type_ = type; };

public:
	void* obj() const { return obj_; };
	void obj(void* obj) { obj_ = obj; };

protected:
	int type_;
	void* obj_;
};

#endif // _MP_MESSAGE_H_