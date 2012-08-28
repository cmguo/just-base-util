// DeviceWrapper.h

#ifndef _UTIL_STREAM_DEVICE_WRAPPER_H_
#define _UTIL_STREAM_DEVICE_WRAPPER_H_

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/positioning.hpp>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "util/stream/Dual.h"

namespace util
{
    namespace stream
    {

        template<typename Ch, typename Mode>
        class basic_async_device
        {
        public:
            typedef Ch char_type;
            struct category
                : public Mode,
                public boost::iostreams::device_tag,
                public boost::iostreams::closable_tag
            {};
        public:
            basic_async_device() {}

            std::streamsize read(char_type*, std::streamsize) { return 0; }
            std::streamsize write(const char_type*, std::streamsize n) { return n; }
            std::streampos seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir,
                BOOST_IOS::openmode = 
                BOOST_IOS::in | BOOST_IOS::out ) 
            { return -1; }
            void close() { }
            void close(BOOST_IOS::openmode) { }

        };

        template < typename Ch>
        class basic_source_wrapper
            : private basic_async_device< Ch, boost::iostreams::input >
        {
        public:
            typedef Ch                              char_type;
            typedef boost::iostreams::source_tag    category;

        public:
            basic_source_wrapper(Source & src)
                : m_source_(src)
            {
            }

            Source * operator->()
            {
                return &m_source_;
            }

            std::streamsize read(char_type * s, std::streamsize n)
            {
                return m_source_.read_some(
                    boost::asio::buffer(s, n));
            }

        private:
            Source & m_source_;
        };

        typedef basic_source_wrapper<char>    source_wrapper_ctype;
        typedef basic_source_wrapper<wchar_t> source_wrapper_wctype;

        template < typename Ch >
        class basic_sink_wrapper
            : private basic_async_device< Ch, boost::iostreams::output >
        {
        public:
            typedef Ch                          char_type;
            typedef boost::iostreams::sink_tag  category;

        public:
            basic_sink_wrapper(Sink & snk)
                : m_sink_(snk)
            {
            }

            Sink * operator->()
            {
                return &m_sink_;
            }

            std::streamsize write(const char_type * s, std::streamsize n)
            {
                return m_sink_.write_some(boost::asio::buffer(s, n));
            }

        private:
            Sink & m_sink_;
        };

        typedef basic_sink_wrapper<char>    sink_wrapper_ctype;
        typedef basic_sink_wrapper<wchar_t> sink_wrapper_wctype;

    } // namespace streams
} // namespace util

#endif // _UTIL_STREAM_DEVICE_WRAPPER_H_
