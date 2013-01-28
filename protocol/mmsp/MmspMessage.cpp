// MmspMessage.cpp

#include "util/Util.h"
#include "util/protocol/mmsp/MmspMessage.h"

namespace util
{
    namespace protocol
    {

        MmspMessage::MmspMessage()
            : destroy_(NULL)
        {
        }

        MmspMessage::~MmspMessage()
        {
            reset();
        }

        void MmspMessage::reset()
        {
            if (!empty()) {
                (this->*destroy_)();
            }
        }

        bool MmspMessage::empty() const
        {
            return destroy_ == NULL;
        }

    } // namespace protocol
} // namespace util
