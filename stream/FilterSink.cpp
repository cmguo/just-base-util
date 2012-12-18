// FilterSink.cpp

#include "util/Util.h"
#include "util/stream/FilterSink.h"
#include "util/stream/StreamErrors.h"

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace util
{
    namespace stream
    {

        class BufferSink
        {
        public:
            struct category
                : boost::iostreams::sink_tag 
                , boost::iostreams::multichar_tag
            {};

            typedef char char_type;

        public:
            BufferSink(
                boost::asio::streambuf & data)
                : eof_(false)
                , m_data_(data)
            {
            }

            ~BufferSink()
            {
            }

        public:
            std::streamsize write(
                const char * s, 
                std::streamsize n)
            {
                n = m_data_.sputn(s, n);
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

        class write_handler
        {
        public:
            typedef void result_type;

        public:
            write_handler(
                FilterSink & sink,
                StreamConstBuffers const & buffers,
                StreamHandler const & handler)
                : sink_(sink)
                , buffers_(buffers)
                , handler_(handler)
                , bytes_write_(0)
            {
            }

            void start()
            {
                if (sink_.buf_.size() > 0) {
                    sink_.sink_.async_write_some(
                        sink_.buf_.data(), boost::bind(boost::ref(*this), _1, _2));
                } else {
                    boost::system::error_code ec;
                    bytes_write_ = sink_.filter_write(buffers_, ec);
                    sink_.get_io_service().post(
                        boost::bind(boost::ref(*this), ec, 0));
                    return;
                }
            }

            void operator()(
                boost::system::error_code ec,
                size_t bytes_transferred)
            {
                sink_.buf_.consume(bytes_transferred);

                if (ec) {
                    handler_(ec, bytes_write_);
                    return;
                }

                if (sink_.buf_.size() > 0) {
                    sink_.sink_.async_write_some(
                        sink_.buf_.data(), boost::bind(boost::ref(*this), _1, _2));
                    return;
                } else if (!has_write_) {
                    bytes_write_ = sink_.filter_write(buffers_, ec);
                    has_write_ = true;
                    (*this)(ec, 0);
                    return;
                }

                handler_(ec, bytes_write_);

                delete this;
            }

        private:
            FilterSink & sink_;
            StreamConstBuffers const buffers_;
            StreamHandler const handler_;
            size_t bytes_write_;
            bool has_write_;
        };

        FilterSink::FilterSink(
            Sink & sink)
            : Sink(sink.get_io_service())
            , sink_(sink)
        {
            set_device_buffer_size(0);
            set_filter_buffer_size(0);
        }

        FilterSink::~FilterSink()
        {
        }

        void FilterSink::complete()
        {
            boost::iostreams::filtering_ostream::push(BufferSink(buf_));
        }

        size_t FilterSink::filter_write(
            buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
            std::size_t bytes_write = 0;

            typedef StreamConstBuffers::const_iterator const_iterator;
            for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                try {
                    write(
                        boost::asio::buffer_cast<const char *>(*iter),
                        boost::asio::buffer_size(*iter));
                    bytes_write += boost::asio::buffer_size(*iter);
                } catch (boost::system::system_error const & e) {
                    ec = e.code();
                    break;
                } catch ( ... ) {
                    ec = util::stream::error::filter_sink_error;
                    break;
                }
            }

            return bytes_write;
        }

        size_t FilterSink::private_write_some(
            buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
            assert(is_complete() && size() > 1);

            using namespace boost::asio;

            ec.clear();
            while (!ec && buf_.size() > 0) {
                buf_.consume(sink_.write_some(buf_.data(), ec));
            }

            if (ec) {
                return 0;
            }

            std::size_t bytes_write = filter_write(buffers, ec);

            if (ec) {
                return 0;
            }

            while (!ec && buf_.size() > 0) {
                buf_.consume(sink_.write_some(buf_.data(), ec));
            }

            ec.clear(); // 如果sink_.write_some有错误，下次返回
            return bytes_write;
        }

        // 内部filter设置类型为buffered_call
        void FilterSink::private_async_write_some(
            buffers_t const & buffers, 
            handler_t const & handler)
        {
            assert(is_complete() && size() > 1);

            write_handler * process_handler =
                new write_handler(*this, buffers, handler);
            process_handler->start();
        }

    } // stream
} // util
