// FilterSink.h

#ifndef _UTIL_TREAM_FILTER_SINK_H_
#define _UTIL_TREAM_FILTER_SINK_H_

#include "util/stream/Sink.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/categories.hpp>

namespace util
{
    namespace stream
    {

        class FilterSink
            : public boost::iostreams::filtering_ostream
            , public Sink
        {
        public:
            FilterSink(
                boost::asio::io_service & ios);

            virtual ~FilterSink();

            template < typename T >
            void push(const T & t,
                typename T::category * = NULL)
            {
                using namespace boost::iostreams;
                typedef typename boost::iostreams::detail::unwrap_ios<T>::type    policy_type;
                if (is_device<policy_type>::value) {
                    filtering_ostream::push(
                        basic_dummy_filter< char_type >());
                }
                filtering_ostream::push(t);
            }

            void push(
                Sink & t);

        private:
            virtual size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec);

            // 内部filter设置类型为buffered_call
            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);
        };

    } // stream
} // util

#endif // _UTIL_TREAM_FILTER_SINK_H_
