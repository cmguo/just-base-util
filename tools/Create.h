// Create.h

#ifndef _UTIL_TOOLS_CREATE_H_
#define _UTIL_TOOLS_CREATE_H_

#include <boost/function.hpp>

namespace util
{
    namespace tools
    {

        template <
            typename Class
        >
        struct Creator
        {
            typedef Class * result_type;//函数对象，需要对外定义函数返回类型

            Class * operator()()
            {
                return new Class;
            }

            template <
                typename Arg1
            >
            Class * operator()(
                Arg1 & arg1)
            {
                return new Class(arg1);
            }

            template <
                typename Arg1,
                typename Arg2
            >
            Class * operator()(
                Arg1 & arg1, Arg2 & arg2)
            {
                return new Class(arg1, arg2);
            }

            template <
                typename Arg1,
                typename Arg2,
                typename Arg3
            >
            Class * operator()(
                Arg1 & arg1, Arg2 & arg2, Arg3 & arg3)
            {
                return new Class(arg1, arg2, arg3);
            }
        };

    } // namespace tools
} // namespace util

#endif // _UTIL_TOOLS_CREATE_H_
