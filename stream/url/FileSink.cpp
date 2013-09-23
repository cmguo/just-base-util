// FileSink.h

#include <util/Util.h>
#include <util/stream/url/FileSink.h>

#include <framework/system/ErrorCode.h>

#include <boost/filesystem/path.hpp>

namespace util
{
    namespace stream
    {

        FileSink::FileSink(
            boost::asio::io_service & io_svc)
            : UrlSink(io_svc)
            , is_open_(false)
        {
        }

        FileSink::~FileSink()
        {
        }

        bool FileSink::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            if (is_open_)
                file_.close(ec);
            is_open_ = file_.open(url.path(), ec);
            if (!is_open_) {
                ec = framework::system::last_system_error();
                if (!ec) {
                    ec = framework::system::logic_error::unknown_error;
                }
            } else {
                if (beg > 0) {
                    if (beg == boost::uint64_t(-1)) {
                        file_.seek(file_.end, 0, ec);
                        beg = file_.tell(ec);
                    } else {
                        file_.seek(file_.beg, beg, ec);
                    }
                }
                assert(!ec);
                assert(file_.tell(ec) == beg);
            }
            return !ec;
        }

        bool FileSink::is_open(
            boost::system::error_code & ec)
        {
            return is_open_;
        }

        bool FileSink::close(
            boost::system::error_code & ec)
        {
            file_.close(ec);
            is_open_ = false;
            return !ec;
        }

        boost::uint64_t FileSink::total(
            boost::system::error_code & ec)
        {
            boost::uint64_t cur;
            boost::uint64_t file_length;
            cur = file_.tell(ec);
            file_.seek(file_.end, 0, ec);

            file_length = file_.tell(ec);
            file_.seek(file_.beg, cur, ec);

            return file_length;
        }

        std::size_t FileSink::private_write_some(
            boost::asio::const_buffer const & buffer,
            boost::system::error_code & ec)
        {
            return file_.write_some(buffer, ec);
        }

    } // namespace stream
} // namespace util
