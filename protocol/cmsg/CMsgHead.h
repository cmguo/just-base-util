// CMsgHeadT.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_HEAD_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_HEAD_H_

#include "util/protocol/cmsg/CMsgField.h"

#include <istream>
#include <ostream>

namespace util
{
    namespace protocol
    {

        struct CMsgHead
            : public std::map<std::string, std::string>
        {
        public:
            CMsgHead();

            virtual ~CMsgHead();

        public:
            CMsgHead & operator=(
                CMsgHead const &);
            
            using std::map<std::string, std::string>::operator[];

            std::string const & operator[](
                std::string const & name) const;

        public:
            template <typename F>
            CMsgField<typename F::type> const operator[](
                F const & f) const
            {
                return CMsgField<typename F::type>(const_cast<CMsgHead &>(*this), f.name);
            }

            template <typename F>
            CMsgField<typename F::type> operator[](
                F const & f)
            {
                return CMsgField<typename F::type>(*this, f.name);
            }

        public:
            void get_content(
                std::ostream & os) const;

            void set_content(
                std::istream & is);

            void get_content(
                std::ostream & os, 
                boost::system::error_code & ec) const;

            void set_content(
                std::istream & is, 
                boost::system::error_code & ec);

        protected:
            virtual bool get_line(
                std::string & line) const = 0;

            virtual bool set_line(
                std::string const & line) = 0;

        private:
            CMsgHead(CMsgHead const &);
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_HEAD_H_
