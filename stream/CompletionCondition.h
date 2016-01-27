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
                explicit  transfer_all_t(
                    bool * b = NULL)
                    : framework::network::detail::transfer_all_t(b)
                {
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
                    std::size_t minimum, 
                    bool * b = NULL)
                    : framework::network::detail::transfer_at_least_t(minimum, b)
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

        inline detail::transfer_all_t transfer_all(
            bool * b = NULL)
        {
            return detail::transfer_all_t(b);
        }

        inline detail::transfer_at_least_t transfer_at_least(
            std::size_t minimum, 
            bool * b = NULL)
        {
            return detail::transfer_at_least_t(minimum, b);
        }

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_COMPLETION_CONDITION_H_
