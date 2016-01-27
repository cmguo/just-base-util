// FilterSource.cpp

#include "util/Util.h"
#include "util/stream/FilterSource.h"
#include "util/buffers/BufferSize.h"
#include "util/stream/StreamErrors.h"

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        class BufferSource
        {
        public:
            struct category
                : boost::iostreams::source_tag 
                , boost::iostreams::multichar_tag
            {};

            typedef char char_type;

        public:
            BufferSource(
                boost::asio::streambuf & data)
                : eof_(false)
                , m_data_(data)
            {
            }

            ~BufferSource()
            {
            }

        public:
            std::streamsize read(
                char * s, 
                std::streamsize n)
            {
                n = m_data_.sgetn(s, n);
                if (n == 0 && eof_)
                    return std::char_traits<char>::eof();
                return n;
            }

        public:
            void set_eof()
            {
                eof_ = true;
            }

            bool is_eof() const
            {
                return eof_;
            }

        private:
            bool eof_;
            boost::asio::streambuf & m_data_;
        };

        class read_handler
        {
        public:
            typedef void result_type;

        public:
            read_handler(
                FilterSource & source,
                StreamMutableBuffers const & buffers,
                StreamHandler const & handler)
                : source_(source)
                , buffers_(buffers)
                , handler_(handler)
            {}

            void start()
            {
                using namespace boost::asio;

                size_t szbuffer = util::buffers::buffer_size(buffers_);
                source_.source_.async_read_some(
                    source_.buf_.prepare(szbuffer), boost::bind(boost::ref(*this), _1, _2));
            }

            void operator()(
                boost::system::error_code ec,
                size_t bytes_transferred)
            {
                BufferSource & dummy = 
                    *source_.component<BufferSource>(source_.size() - 1);

                source_.buf_.commit(bytes_transferred);

                size_t bytes_read = 0;

                if (!ec || ec == boost::asio::error::eof) {
                    if (ec == boost::asio::error::eof) {
                        ec.clear();
                        dummy.set_eof();
                    }
                    bytes_read = source_.filter_read(buffers_, ec);
                }

                handler_(ec, bytes_read);

                delete this;
            }

        private:
            FilterSource & source_;
            StreamMutableBuffers const buffers_;
            StreamHandler const handler_;
        };

        FilterSource::FilterSource(
            Source & source)
            : Source(source.get_io_service())
            , source_(source)
        {
            set_device_buffer_size(0);
            set_filter_buffer_size(0);
        }
        FilterSource::~FilterSource()        {        }        void FilterSource::complete()
        {
            boost::iostreams::filtering_istream::push(BufferSource(buf_));
        }
        std::size_t FilterSource::filter_read(
            buffers_t const & buffers,
            boost::system::error_code & ec)
        {
            assert(is_complete() && size() > 1);

            std::size_t bytes_read = 0;

            BufferSource & dummy = 
                *component<BufferSource>(size() - 1);

            try {
                for (buffers_t::const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                    read(
                        boost::asio::buffer_cast<char *>(*iter), 
                        boost::asio::buffer_size(*iter));
                    bytes_read += gcount();
                    if (bytes_read == 0 && eof() && dummy.is_eof()) {
                        ec = boost::asio::error::eof;
                    }
                    if (fail()) {
                        clear();
                        break;
                    }
                }
            } catch (boost::system::system_error const & e) {
                ec = e.code();
            } catch ( ... ) {
                ec = util::stream::error::filter_source_error;
            }

            return bytes_read;
        }

        std::size_t FilterSource::private_read_some(
            buffers_t const & buffers,
            boost::system::error_code & ec)
        {
            size_t szbuffer = util::buffers::buffer_size(buffers);

            buf_.commit(source_.read_some(buf_.prepare(szbuffer), ec));

            if (ec && ec != boost::asio::error::eof) {
                return 0;
            }

            BufferSource & dummy = 
                *component<BufferSource>(size() - 1);

            if (ec == boost::asio::error::eof) {
                ec.clear();
                dummy.set_eof();
            }

            size_t bytes_read = filter_read(buffers, ec);

            return bytes_read;
        }

        // 内部filter设置类型为buffered_call
        void FilterSource::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            BufferSource & dummy = 
                *component<BufferSource>(size() - 1);

            if (dummy.is_eof()) {
                boost::system::error_code ec;
                size_t bytes_read = filter_read(buffers, ec);
                get_io_service().post(boost::bind(handler, ec, bytes_read));
            } else {
                read_handler * process_handler = 
                    new read_handler(*this, buffers, handler);
                process_handler->start();
            }
        }

    } // stream
} // util
