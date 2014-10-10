// CMsgFieldDef.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_FIELD_DEF_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_FIELD_DEF_H_

#ifndef CMSG_DEFINE_FIELD
#  define CMSG_DECLARE_FIELD(variable, name, type) \
    extern util::protocol::CMsgFieldDef<type> f_ ## variable
#else
#  define CMSG_DECLARE_FIELD(variable, name, type) \
    util::protocol::CMsgFieldDef<type> f_ ## variable = {name}
#endif

namespace util
{
    namespace protocol
    {

        template <typename T>
        struct CMsgFieldDef
        {
            typedef T type;
            char const * name;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_FIELD_DEF_H_
