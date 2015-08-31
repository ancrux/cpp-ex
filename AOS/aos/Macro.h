#ifndef _MACRO_H_
#define _MACRO_H_

namespace aos {

// use this macro to simply avoid warnings with gcc
#define AOS_UNUSE(v) do {} while (&v == 0)

} // namespace aos

#endif // _MACRO_H_
