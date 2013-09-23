// Call.h

#ifndef _UTIL_TOOLS_CALL_H_
#define _UTIL_TOOLS_CALL_H_

namespace util
{
    namespace tools
    {

        struct Call
        {
            template <
                typename Func, 
                typename Arg1
            >
            Call(
                Func func, 
                Arg1 arg1)
            {
                func(arg1);
            }

            template <
                typename Func, 
                typename Arg1, 
                typename Arg2
            >
            Call(
                Func func, 
                Arg1 arg1, 
                Arg2 arg2)
            {
                func(arg1, arg2);
            }

            template <
                typename Func, 
                typename Arg1, 
                typename Arg2,
                typename Arg3
            >
            Call(
                Func func, 
                Arg1 arg1, 
                Arg2 arg2,
                Arg3 arg3)
            {
                func(arg1, arg2, arg3);
            }
        };

    } // namespace tools
} // namespace util

#endif // _UTIL_TOOLS_CALL_H_
