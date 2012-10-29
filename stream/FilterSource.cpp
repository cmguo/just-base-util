// FilterSource.cpp

#include "util/Util.h"
#include "util/stream/FilterSource.h"
#include "util/buffers/BufferSize.h"
#include "util/stream/StreamErrors.h"
#include "util/stream/DummyFilter.h"
#include "util/stream/DeviceWrapper.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/categories.hpp>

#include <iostream>

namespace util
{
    namespace stream
    {

        class FilterSource;

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

                basic_dummy_filter<char> & dummy = 
                    *source_.template component<basic_dummy_filter<char> >(source_.size() - 2);
                basic_source_wrapper<char> & device = 
                    *source_.template component<basic_source_wrapper<char> >(source_.size() - 1);
                size_t szbuffer = util::buffers::buffer_size(buffers_);
                device->async_read_some(
                    dummy.use_buffer().prepare(szbuffer), boost::bind(boost::ref(*this), _1, _2));
            }

            void operator()(
                boost::system::error_code const & ec,
                size_t bytes_transferred)
            {
                basic_dummy_filter<char> & dummy = 
                    *source_.template component< basic_dummy_filter<char> >(source_.size() - 2);
                dummy.use_buffer().commit(bytes_transferred);

                boost::system::error_code ec1 = ec;
                std::size_t bytes_read = 0;

                if (!ec1 || ec1 == boost::asio::error::eof) {
                    if (ec1 == boost::asio::error::eof) {
                        dummy.set_eof();
                    }
                    bytes_read = source_.read_some(buffers_, ec1);
                    if (ec1 == boost::asio::error::eof && ec != boost::asio::error::eof) {
                        ec1.clear();
                    }
                }

                dummy.end_async();

                handler_(ec1, bytes_read);

                delete this;
            }

        private:
            FilterSource & source_;
            StreamMutableBuffers const buffers_;
            StreamHandler const handler_;
        };

        FilterSource::FilterSource(
            boost::asio::io_service & ios)
            : Source(ios)
        {
            set_device_buffer_size(0);
            set_filter_buffer_size(0);
        }
        FilterSource::~FilterSource()        {        }        void FilterSource::push(
            Source & t)
        {
            using namespace boost::iostreams;
            filtering_istream::push(
                basic_dummy_filter<char>());
            filtering_istream::push(basic_source_wrapper<char>(t));
        }
        // 内部filter设置类型为buffered_call
        std::size_t FilterSource::private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            using namespace boost::asio;

            assert(is_complete() && size() > 1);

            ec.clear();
            std::size_t bytes_read = 0;

            typedef StreamMutableBuffers::const_iterator const_iterator;
            try {
                for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                    read(buffer_cast<char *>(*iter), buffer_size(*iter));
                    bytes_read += gcount();
                    if (eof()) {
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

        // 内部filter设置类型为buffered_call
        void FilterSource::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            assert(is_complete() && size() > 1);

            basic_dummy_filter<char> & dummy = 
                *component<basic_dummy_filter<char> >(size() - 2);
            dummy.begin_async();

            if (dummy.is_eof()) {
                boost::system::error_code ec;
                size_t bytes_read = private_read_some(buffers, ec);
                get_io_service().post(boost::bind(handler, ec, bytes_read));
                dummy.end_async();
            } else {
                read_handler * process_handler = 
                    new read_handler(*this, buffers, handler);
                process_handler->start();
            }
        }

    } // stream
} // util
