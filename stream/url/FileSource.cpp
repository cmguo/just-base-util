// FileSource.h

#include <util/Util.h>
#include <util/stream/url/FileSource.h>

#include <framework/system/ErrorCode.h>

#include <boost/filesystem/path.hpp>

namespace util
{
    namespace stream
    {

        FileSource::FileSource(
            boost::asio::io_service & io_svc)
            : UrlSource(io_svc)
            , is_open_(false)
        {
        }

        FileSource::~FileSource()
        {
        }

        bool FileSource::open(
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
                if (beg > 0)
                    file_.seek(file_.beg, beg, ec);
                assert(!ec);
                assert(file_.tell(ec) == beg);
            }
            return !ec;
        }

        bool FileSource::is_open(
            boost::system::error_code & ec)
        {
            return is_open_;
        }

        bool FileSource::close(
            boost::system::error_code & ec)
        {
            file_.close(ec);
            is_open_ = false;
            return !ec;
        }

        boost::uint64_t FileSource::total(
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

        std::size_t FileSource::private_read_some(
            boost::asio::mutable_buffer const & buffer,
            boost::system::error_code & ec)
        {
            return file_.read_some(buffer, ec);
        }

    } // namespace stream
} // namespace util
