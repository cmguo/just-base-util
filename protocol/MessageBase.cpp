// MessageBase.cpp

#include "util/Util.h"
#include "util/protocol/Message.h"
#include "util/protocol/Message.hpp"

namespace util
{
    namespace protocol
    {

        MessageBase::MessageBase()
            : def_(NULL)
        {
        }

        MessageBase::~MessageBase()
        {
            reset();
        }

        MessageBase::MessageBase(
            MessageBase const & r)
            : def_(r.def_)
        {
            if (def_) {
                def_->copy(this, &r);
            }
        }

        MessageBase & MessageBase::operator=(
            MessageBase const & r)
        {
            reset();
            def_ = r.def_;
            if (def_) {
                def_->copy(this, &r);
            }
            return *this;
        }

        void MessageBase::from_data(
            boost::asio::streambuf & buf, 
            MessageParser & parser)
        {
            def_->from_data(this, buf, parser);
        }

        void MessageBase::to_data(
            boost::asio::streambuf & buf, 
            MessageParser & parser) const
        {
            def_->to_data(this, buf, parser);
        }

        void MessageBase::reset()
        {
            if (!empty()) {
                def_->destroy(this);
                def_ = NULL;
            }
        }

        void MessageBase::reset(
            MessageDefine const * def)
        {
            if (!empty()) {
                def_->destroy(this);
            }
            def->construct(this);
            def_ = def;
        }

        bool MessageBase::empty() const
        {
            return def_ == NULL;
        }

    } // namespace protocol
} // namespace util
