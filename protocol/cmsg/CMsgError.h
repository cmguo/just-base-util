// CMsgError.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_ERROR_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_ERROR_H_

namespace util
{
    namespace protocol
    {

        namespace cmsg_error {

            enum errors
            {
                format_error = 1,   // cmsgÕ∑∏Ò Ω¥ÌŒÛ

            };

            namespace detail {

                class cmsg_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "cmsg";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                        case cmsg_error::format_error:
                            return "MINE packet format error";
                        default:
                            return "cmsg error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::cmsg_category instance;
                return instance;
            }

            static boost::system::error_category const & cmsg_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace cmsg_error

    } // namespace protocol
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::protocol::cmsg_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::cmsg_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_CMSG_CMSG_ERROR_H_
