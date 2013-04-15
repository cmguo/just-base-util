// MmspString.h

#ifndef _UTIL_PROTOCOL_MMSP_MMSP_STRING_H_
#define _UTIL_PROTOCOL_MMSP_MMSP_STRING_H_

#include "util/serialization/SplitMember.h"
#include "util/serialization/Array.h"
#include "util/serialization/Collection.h"

namespace util
{
    namespace protocol
    {

        class MmspString
        {
        public:
            MmspString()
                : size_(boost::uint32_t(-1))
            {
            }

            MmspString(
                std::string const & str)
                : size_(boost::uint32_t(-1))
            {
                str_.assign(str.begin(), str.end());
                str_.push_back(0);
            }

            template <size_t n>
            MmspString(
                wchar_t const (&str)[n])
                : size_(boost::uint32_t(-1))
            {
                str_.assign(str, str + n);
            }

            template <size_t n>
            MmspString(
                char const (&str)[n])
                : size_(boost::uint32_t(-1))
            {
                str_.assign(str, str + n);
            }

        public:
            boost::uint32_t size()
            {
                return str_.size();
            }

            void size(
                boost::uint32_t n)
            {
                size_ = n;
            }

        public:

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                str_.clear();
                if (size_ == boost::uint32_t(-1)) {
                    boost::uint16_t c;
                    while (ar >> c) {
                        str_.push_back(c);
                        if (c == 0)
                            break;
                    }
                } else {
                    util::serialization::load_collection(ar, str_, size_);
                }
            }

            template <typename Archive>
            void save(
                Archive & ar) const
            {
                if (!str_.empty()) {
                    ar & framework::container::make_array(&str_.front(), str_.size());
                }
            }

            std::string to_string() const
            {
                std::string str(str_.begin(), str_.end());
                std::string::size_type pos = str.find_last_not_of((char)0);
                if (pos == std::string::npos) {
                    str.clear();
                } else {
                    str.erase(pos + 1);
                }
                return str;
            }

        private:
            std::vector<boost::uint16_t> str_;
            boost::uint32_t size_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMSP_MMSP_STRING_H_
