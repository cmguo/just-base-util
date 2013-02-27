// Message.hpp

#ifndef _UTIL_PROTOCOL_MESSAGE_HPP_
#define _UTIL_PROTOCOL_MESSAGE_HPP_

#include "util/protocol/Message.h"

namespace util
{
    namespace protocol
    {

        template <
            typename MsgT
        >
        std::map<typename Message<MsgT>::id_type, MessageDefine const *> Message<MsgT>::msg_defs_;

        template <typename MsgT>
        template <typename T>
        T & Message<MsgT>::get()
        {
            if (!is<T>()) {
                reset(&T::msg_def);
                header_type::id(T::static_id);
            }
            return as<T>();
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::set(
            T const & t)
        {
            reset();
            header_type::id(T::static_id);
            def_ = &T::msg_def;
        }

        template <typename MsgT>
        template <typename T>
        bool Message<MsgT>::is() const
        {
            return def_ == &T::msg_def;
        }

        template <typename MsgT>
        template <typename T>
        T const & Message<MsgT>::as() const
        {
            assert(is<T>());
            return *(T const *)data_;
        }

        template <typename MsgT>
        template <typename T>
        T & Message<MsgT>::as()
        {
            assert(is<T>());
            return *(T *)data_;
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::reg_msg(
            MessageDefine & def)
        {
            assert(&def == &T::msg_def);
            def.construct = &s_construct<T>;
            def.copy = &s_copy<T>;
            def.from_data = &s_from_data<T>;
            def.to_data = &s_to_data<T>;
            def.destroy = &s_destroy<T>;
            msg_defs_[T::static_id] = &def;
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_construct(
            MessageBase * mb)
        {
            Message * m = static_cast<Message *>(mb);
            assert(sizeof(T) <= sizeof(m->data_));
            new (m->data_) T;
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_copy(
            MessageBase * mb, 
            MessageBase const * mbr)
        {
            Message * m = static_cast<Message *>(mb);
            Message const * mr = static_cast<Message const *>(mbr);
            new (m->data_) T(mr->as<T>());
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_from_data(
            MessageBase * mb, 
            boost::asio::streambuf & buf, 
            MessageParser & parser)
        {
            Message * m = static_cast<Message *>(mb);
            typename MsgT::i_archive_t ia(buf);
            void * ctx = ia.context();
            ia.context(&parser);
            ia >> (header_type &)(m);
            ia >> m->as<T>();
            ia.context(ctx);
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_to_data(
            MessageBase const * mb, 
            boost::asio::streambuf & buf, 
            MessageParser & parser)
        {
            Message const * m = static_cast<Message const *>(mb);
            typename MsgT::o_archive_t oa(buf);
            void * ctx = oa.context();
            oa.context(&parser);
            oa << (header_type const &)(m);
            oa << m->as<T>();
            oa.context(ctx);
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_destroy(
            MessageBase * mb)
        {
            Message * m = static_cast<Message *>(mb);
            ((T *)m->data_)->~T();
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_HPP_
