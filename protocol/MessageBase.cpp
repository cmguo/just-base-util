// MessageBase.cpp

#include "util/Util.h"
#include "util/protocol/MessageBase.h"

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
            StreamBuffer & buf, 
            void * ctx)
        {
            def_->from_data(this, buf, ctx);
        }

        void MessageBase::to_data(
            StreamBuffer & buf, 
           void * ctx) const
        {
            def_->to_data(this, buf, ctx);
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
