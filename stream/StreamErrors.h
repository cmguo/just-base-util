// StreamErrors.h

#ifndef _UTIL_STREAM_ERRORS_H_
#define _UTIL_STREAM_ERRORS_H_

namespace util
{
    namespace stream
    {

        namespace error {

            enum errors
            {
                filter_sink_error = 1,
                filter_source_error, 
                unknown_url_proto, 
            };

            namespace detail {

                class stream_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "stream";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                            case filter_sink_error:
                                return "stream: filter sink error";
                            case filter_source_error:
                                return "stream: filter source error";
                            case unknown_url_proto:
                                return "stream: unknown url proto";
                        }
                        return "stream: unknown";
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::stream_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace error

    } // namespace stream
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::stream::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::stream::error::make_error_code;
#endif

    }
}

#endif // _UTIL_STREAM_ERRORS_H_
