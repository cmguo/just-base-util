// MessageTraits.h

#ifndef _UTIL_PROTOCOL_MESSAGE_DEFINE_H_
#define _UTIL_PROTOCOL_MESSAGE_DEFINE_H_

#include <util/buffers/StreamBuffer.h>

namespace util
{
    namespace protocol
    {

        class MessageBase;

        typedef util::buffers::StreamBuffer<boost::uint8_t> StreamBuffer;

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
                StreamBuffer &, 
                void *);
            typedef void (* to_data_t)(
                MessageBase const *, 
                StreamBuffer &, 
                void *);
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

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_DEFINE_H_
