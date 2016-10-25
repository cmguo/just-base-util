// StaticInitor.h

#ifndef _UTIL_TOOLS_STATIC_INITOR_H_
#define _UTIL_TOOLS_STATIC_INITOR_H_

#include <boost/tti/has_static_member_function.hpp>

namespace util
{
    namespace tools
    {
   
        BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(static_init);

        template <
            typename Class, 
            typename Proto, 
            bool Has = has_static_member_function_static_init<Class, Proto>::value
        >
        struct StaticInitor
        {
            typedef void result_type;

            result_type operator()()
            {
                return Class::static_init();
            }

            template <
                typename Arg1
            >
            result_type operator()(
                Arg1 & arg1)
            {
                return Class::static_init(arg1);
            }

            template <
                typename Arg1,
                typename Arg2
            >
            result_type operator()(
                Arg1 & arg1, Arg2 & arg2)
            {
                return static_init(arg1, arg2);
            }

            template <
                typename Arg1,
                typename Arg2,
                typename Arg3
            >
            result_type operator()(
                Arg1 & arg1, Arg2 & arg2, Arg3 & arg3)
            {
                return static_init(arg1, arg2, arg3);
            }
        };

        template <
            typename Class, 
            typename Proto
        >
        struct StaticInitor<Class, Proto, false>
        {
            typedef void result_type;

            result_type operator()()
            {
            }

            template <
                typename Arg1
            >
            result_type operator()(
                Arg1 & arg1)
            {
            }

            template <
                typename Arg1,
                typename Arg2
            >
            result_type operator()(
                Arg1 & arg1, Arg2 & arg2)
            {
            }

            template <
                typename Arg1,
                typename Arg2,
                typename Arg3
            >
            result_type operator()(
                Arg1 & arg1, Arg2 & arg2, Arg3 & arg3)
            {
            }
        };

    } // namespace tools
} // namespace util

#endif // _UTIL_TOOLS_STATIC_INITOR_H_
