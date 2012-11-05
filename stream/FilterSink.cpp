// FilterSink.cpp

#include "util/Util.h"
#include "util/stream/FilterSink.h"
#include "util/stream/StreamErrors.h"
#include "util/stream/DummyFilter.h"
#include "util/stream/DeviceWrapper.h"

namespace util
{
    namespace stream
    {

        class write_handler
        {
        public:
            typedef void result_type;

        public:
            write_handler(
                FilterSink & source,
                StreamConstBuffers const & buffers,
                StreamHandler const & handler)
                : sink_(source)
                , buffers_(buffers)
                , handler_(handler)
                , bytes_write_(0)
            {
            }

            void start()
            {
                using namespace boost::asio;

                basic_dummy_filter<char> & dummy = 
                    *sink_.component<basic_dummy_filter<char> >(sink_.size() - 2);
                basic_sink_wrapper<char> & device = 
                    *sink_.component<basic_sink_wrapper<char> >(sink_.size() - 1);

                boost::system::error_code ec;
                bytes_write_ = sink_.write_some(buffers_, ec);
                if (ec) {
                    sink_.get_io_service().post(
                        boost::asio::detail::bind_handler(handler_, ec, bytes_write_));
                } else {
                    device->async_write_some(
                        dummy.get_buffer().data(), boost::bind(boost::ref(*this), _1, _2));
                }
            }

            void operator()(
                boost::system::error_code const & ec,
                size_t bytes_transferred)
            {
                basic_dummy_filter<char> & dummy = 
                    *sink_.component<basic_dummy_filter<char> >(sink_.size() - 2);
                dummy.use_buffer().consume(bytes_transferred);

                dummy.end_async();

                handler_(ec, bytes_write_);

                delete this;
            }

        private:
            FilterSink & sink_;
            StreamConstBuffers const buffers_;
            StreamHandler const handler_;
            size_t bytes_write_;
        };

        FilterSink::FilterSink(
            boost::asio::io_service & ios)
            : Sink(ios)
        {
            set_device_buffer_size(0);
            set_filter_buffer_size(0);
        }

        FilterSink::~FilterSink()
        {
        }

        void FilterSink::push(
            Sink & t)
        {
            using namespace boost::iostreams;
            filtering_ostream::push(
                basic_dummy_filter< char_type >());
            filtering_ostream::push(basic_sink_wrapper< char_type >(t));
        }

        size_t FilterSink::private_write_some(
            StreamConstBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            assert(is_complete() && size() > 1);

            using namespace boost::asio;

            std::size_t bytes_write = 0;

            typedef StreamConstBuffers::const_iterator const_iterator;
            for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                try {
                    write(
                        (const char *)boost::asio::detail::buffer_cast_helper(*iter),
                        boost::asio::detail::buffer_size_helper(*iter));
                    flush();
                } catch (boost::system::system_error const & e) {
                    ec = e.code();
                } catch ( ... ) {
                    ec = util::stream::error::filter_sink_error;
                    break;
                }
                bytes_write += boost::asio::detail::buffer_size_helper(*iter);
            }

            return bytes_write;
        }

        // 内部filter设置类型为buffered_call
        void FilterSink::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            assert(is_complete() && size() > 1);

            basic_dummy_filter<char> & dummy = 
                *component<basic_dummy_filter<char> >(size() - 2);
            dummy.begin_async();

            write_handler * process_handler =
                new write_handler(*this, buffers, handler);
            process_handler->start();
        }

    } // stream
} // util
