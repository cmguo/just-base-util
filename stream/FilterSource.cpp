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
                size_t szbuffer = util::buffers::buffer_size(buffers_);
                if (szbuffer == 0) {
                    source_.get_io_service().post(boost::asio::detail::bind_handler(
                        handler_, boost::system::error_code(), 0));
                    return;
                }

                if (source_.is_complete() && source_.size() > 1) {
                    basic_dummy_filter<char> & dummy = 
                        *source_.template component<basic_dummy_filter<char> >(source_.size() - 2);
                    basic_source_wrapper<char> & device = 
                        *source_.template component<basic_source_wrapper<char> >(source_.size() - 1);
                    dummy.set_call_type(dummy.buffered_call);
                    device->async_read_some(
                        dummy.use_buffer().prepare(szbuffer), boost::bind(boost::ref(*this), _1, _2));
                } else {
                    boost::system::error_code ec = util::stream::error::chain_is_not_complete;
                    source_.get_io_service().post(
                        boost::asio::detail::bind_handler(handler_, ec, 0));
                    delete this;
                }
            }

            void operator()(
                boost::system::error_code const & ec,
                size_t bytes_transferred)
            {
                basic_dummy_filter<char> & dummy = 
                    *source_.template component< basic_dummy_filter<char> >(source_.size() - 2);
                dummy.use_buffer().commit(bytes_transferred);
                if (ec) {
                    source_.get_io_service().post(
                        boost::asio::detail::bind_handler(handler_, ec, 0));
                    delete this;
                } else {
                    std::size_t total_size = 0;
                    try {
                        using namespace boost::asio;

                        typedef StreamMutableBuffers::const_iterator const_iterator;
                        for (const_iterator iter = buffers_.begin(); iter != buffers_.end(); ++iter) {
                            //total_size += private_read_some(*iter, ec);
                            source_.read(
                                boost::asio::buffer_cast<char *>(*iter),
                                boost::asio::buffer_size(*iter));
                            total_size += source_.gcount();
                            if (!source_) {
                                source_.clear();
                                break;
                            }
                        }
                    } catch ( ... ) {
                        boost::system::error_code ec = util::stream::error::filter_source_error;
                        source_.get_io_service().post(
                            boost::asio::detail::bind_handler(handler_, ec, 0));
                        delete this;
                        return;
                    }
                    source_.get_io_service().post(
                        boost::asio::detail::bind_handler(handler_, ec, total_size));
                    delete this;
                }
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

            typedef StreamMutableBuffers::const_iterator const_iterator;
            std::size_t total_size = 0;

            if (is_complete() && size() > 1) {
                (component< basic_dummy_filter< char_type > >(size() - 2))->set_call_type(
                    basic_dummy_filter< char_type >::buffered_call);

                typedef StreamMutableBuffers::const_iterator const_iterator;
                for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                    //total_size += private_read_some(*iter, ec);
                    read(buffer_cast<char *>(*iter), buffer_size(*iter));
                    total_size += gcount();
                    if (fail()) {
                        clear();
                        break;
                    }
                }
            } else {
                ec = util::stream::error::chain_is_not_complete;
            }

            return total_size;
        }

        // 内部filter设置类型为buffered_call
        void FilterSource::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            read_handler * process_handler = 
                new read_handler(*this, buffers, handler);
            process_handler->start();
        }

    } // stream
} // util
