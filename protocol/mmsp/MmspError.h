// MmspError.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_ERROR_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_ERROR_H_

#include "boost/system/error_code.hpp"

namespace util
{
    namespace protocol
    {

        namespace mmsp_error {

            enum errors
            {
                succeed             = 0,
                busy_work, 
                unkown_command, 
                format_error, 
            };

            namespace detail {

                class mmsp_category
                    : public boost::system::error_category
                {
                public:
                    const char* name() const BOOST_SYSTEM_NOEXCEPT
                    {
                        return "mmsp";
                    }

                    std::string message(int value) const
                    {
                        switch (value) {
                        case busy_work:
                            return "mmsp: busy work";
                        case unkown_command:
                            return "mmsp: unkown command";
                        case format_error:
                            return "mmsp: format error";
                        default:
                            return "mmsp error";
                        }
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::mmsp_category instance;
                return instance;
            }

            static boost::system::error_category const & mmsp_category = get_category();

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace mms_error

    } // namespace protocol
} // namespace util

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<util::protocol::mmsp_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using util::protocol::mmsp_error::make_error_code;
#endif

    }
}

#endif // _UTIL_PROTOCOL_MMSP_MMSP_ERROR_H_
