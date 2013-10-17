// Deleter.h

#ifndef _UTIL_TOOLS_DELETER_H_
#define _UTIL_TOOLS_DELETER_H_

namespace util
{
    namespace tools
    {

        template <
            typename Class
        >
        struct Deleter
        {
            typedef void result_type; //函数对象，需要对外定义函数返回类型

            void operator()(Class * obj)
            {
                delete obj;
            }
        };

    } // namespace tools
} // namespace util

#endif // _UTIL_TOOLS_DELETER_H_
