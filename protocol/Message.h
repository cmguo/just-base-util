// Message.h

#ifndef _UTIL_PROTOCOL_MESSAGE_H_
#define _UTIL_PROTOCOL_MESSAGE_H_

#include "util/protocol/MessageBase.h"

namespace util
{
    namespace protocol
    {

        template <
            typename MsgT
        >
        class Message
            : public MessageBase
            , public MsgT::header_type
        {
        public:
            typedef typename MsgT::id_type id_type;
            typedef typename MsgT::header_type header_type;

        public:
            Message();

            Message(
                Message const & r);

        public:
            template <typename T>
            T & get();

            template <typename T>
            void set(
                T const & t);

            template <typename T>
            bool is() const;

            template <typename T>
            T const & as() const;

            template <typename T>
            T & as();

        public:
            template <typename T>
            static void reg_msg(
                MessageDefine & def);

            static MessageDefine const * find_msg(
                id_type id)
            {
                return msg_defs()[id];
            }

        private:
            template <typename T>
            static void s_construct(
                MessageBase *);

            template <typename T>
            static void s_copy(
                MessageBase *, 
                MessageBase const *);

            template <typename T>
            static void s_from_data(
                MessageBase *, 
                StreamBuffer & buf, 
                void *);

            template <typename T>
            static void s_to_data(
                MessageBase const *, 
                StreamBuffer & buf, 
                void *);

            template <typename T>
            static void s_destroy(
                MessageBase *);

        private:
            static std::map<id_type, MessageDefine const *> & msg_defs();

        private:
            char data_[MsgT::max_size];
        };

        template <
            typename MsgT, 
            typename T
        >
        struct MessageDefineT
            : MessageDefine
        {
            MessageDefineT()
            {
                Message<MsgT>::reg_msg<T>(*this);
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_H_
