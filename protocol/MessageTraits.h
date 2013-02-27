// MessageTraits.h

#ifndef _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
#define _UTIL_PROTOCOL_MESSAGE_TRAITS_H_

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {

        class MessageBase;
        class MessageParser;

        struct MessageDefine
        {
            enum ClassEnum
            {
                control_message, 
                data_message, 
            };

            typedef void (* constuct_t)(
                MessageBase *);
            typedef void (* copy_t)(
                MessageBase *, 
                MessageBase const *);
            typedef void (* from_data_t)(
                MessageBase *, 
                boost::asio::streambuf &, 
                MessageParser &);
            typedef void (* to_data_t)(
                MessageBase const *, 
                boost::asio::streambuf &, 
                MessageParser &);
            typedef void (* destroy_t)(
                MessageBase *);

            MessageDefine()
                : cls(control_message)
                , construct(NULL)
                , copy(NULL)
                , from_data(NULL)
                , to_data(NULL)
                , destroy(NULL)
            {
            }

            ClassEnum cls;
            constuct_t construct;
            copy_t copy;
            from_data_t from_data;
            to_data_t to_data;
            destroy_t destroy;
        };

        class MessageTraits
        {
        public:
            typedef boost::uint32_t id_type; // 可以重新定义

            typedef void header_type; // 必须重新定义

            typedef void i_archive_t; // 必须重新定义

            typedef void o_archive_t; // 必须重新定义

            static size_t const max_size = 0; // 必须重新定义
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
