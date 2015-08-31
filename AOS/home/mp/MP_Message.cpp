#include "MP_Message.h"

MP_Message::MP_Message()
:
type_(0)
{
}

MP_Message::MP_Message(const std::string& str, int type)
:
std::string(str),
type_(type)
{
}

MP_Message::MP_Message(const char* cstr, int type)
:
std::string(cstr),
type_(type)
{
}

MP_Message::~MP_Message()
{
}