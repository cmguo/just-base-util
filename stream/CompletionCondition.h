// CompletionCondition.h

#ifndef _UTIL_STREAM_COMPLETION_CONDITION_H_
#define _UTIL_STREAM_COMPLETION_CONDITION_H_

#include <framework/network/CompletionCondition.h>

namespace util
{
    namespace stream
    {

        typedef std::pair<std::size_t, std::size_t> transfer_size;

        namespace detail
        {

            class transfer_all_t
                : public framework::network::detail::transfer_all_t
            {
            public:
                typedef bool result_type;

                transfer_all_t()
                {
                }

                template <typename Arg>
                transfer_all_t(Arg arg)
                    : framework::network::detail::transfer_all_t(arg)
                {
                }

                template <typename Error>
                bool operator()(
                    const Error& err, 
                    std::size_t)
                {
                    return canceled() || (!!err);
                }

                template <typename Error>
                bool operator()(
                    bool is_read, 
                    const Error& err, 
                    transfer_size const &)
                {
                    return canceled() || (!!err);
                }
            };

            class transfer_at_least_t
                : public framework::network::detail::transfer_at_least_t
            {
            public:
                explicit transfer_at_least_t(
                    std::size_t minimum)
                    : framework::network::detail::transfer_at_least_t(minimum)
                {
                }

                template <typename Arg>
                explicit transfer_at_least_t(
                    std::size_t minimum, 
                    Arg arg)
                    : framework::network::detail::transfer_at_least_t(minimum, arg)
                {
                }

                template <typename Error>
                bool operator()(
                    bool is_read, 
                    const Error& err, 
                    transfer_size const & bytes_transferred)
                {
                    if (is_read)
                        return canceled() || (!!err) || bytes_transferred.first >= minimum_;
                    else
                        return canceled() || (!!err) || bytes_transferred.second >= minimum_;
                }
            };

        }

        inline detail::transfer_all_t transfer_all()
        {
            return detail::transfer_all_t();
        }

        template <typename Arg>
        inline detail::transfer_all_t transfer_all(
            Arg arg)
        {
            return detail::transfer_all_t(arg);
        }

        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum)
        {
            return detail::transfer_at_least_t(minimum);
        }

        template <typename Arg>
        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum, 
            Arg arg)
        {
            return detail::transfer_at_least_t(minimum, arg);
        }

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_COMPLETION_CONDITION_H_
