// Base.cpp

#include "util/Util.h"
#include "util/stream/Base.h"

#include <framework/system/LogicError.h>

namespace util
{
    namespace stream
    {

        Base::Base()
            : io_svc_(NULL)
        {
        }

        Base::~Base()
        {
        }

        bool Base::cancel(
            boost::system::error_code & ec)
        {
            ec = framework::system::logic_error::not_supported;
            return false;
        }

        bool Base::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            ec = framework::system::logic_error::not_supported;
            return false;
        }

        bool Base::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            ec = framework::system::logic_error::not_supported;
            return false;
        }

    } // namespace stream
} // namespace util
