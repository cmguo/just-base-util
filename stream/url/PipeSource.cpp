// PipeSource.h

#include "util/Util.h"
#include "util/stream/url/PipeSource.h"
#include "util/stream/StreamErrors.h"

#include <framework/system/ErrorCode.h>
#include <framework/string/Parse.h>

namespace util
{
    namespace stream
    {

        PipeSource::PipeSource(
            boost::asio::io_service & io_svc)
            : UrlSource(io_svc)
            , descriptor_(io_svc)
            , is_open_(false)
        {
        }

        PipeSource::~PipeSource()
        {
        }

        bool PipeSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            if (is_open_)
                descriptor_.close(ec);
            if (beg != 0 && end != boost::uint64_t(-1)) {
                ec = framework::system::logic_error::invalid_argument;
                return false;
            }
            native_descriptor nd = framework::string::parse<native_descriptor>(url.path().substr(1));
            descriptor_.assign(nd);
            is_open_ = descriptor_.is_open();
            if (!is_open_) {
                ec = framework::system::last_system_error();
                if (!ec) {
                    ec = framework::system::logic_error::unknown_error;
                }
            }
            return !ec;
        }

        bool PipeSource::is_open(
            boost::system::error_code & ec)
        {
            return is_open_;
        }

        bool PipeSource::close(
            boost::system::error_code & ec)
        {
            descriptor_.close(ec);
            is_open_ = false;
            return !ec;
        }

        std::size_t PipeSource::private_read_some(
            buffers_t const & buffers,
            boost::system::error_code & ec)
        {
            return descriptor_.read_some(buffers, ec);
        }

        bool PipeSource::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
#ifndef BOOST_WINDOWS_API
            boost::asio::posix::descriptor_base::non_blocking_io cmd(non_block);
            return !descriptor_.io_control(cmd, ec);
#else
            return UrlSource::set_non_block(non_block, ec);
#endif    
        }

    } // namespace stream
} // namespace util
