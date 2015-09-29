// CMsgFieldRange.h

#ifndef _UTIL_PROTOCOL_CMSG_CMSG_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_CMSG_CMSG_FIELD_RANGE_H_

#include <framework/string/Format.h>
#include <framework/string/Parse.h>
#include <framework/string/StringToken.h>
#include <framework/string/Slice.h>
#include <framework/string/Join.h>

#include <iterator>

namespace util
{
    namespace protocol
    {
        namespace cmsg_field
        {

            template <
                typename Traits
            >
            struct RangeUnitT
            {
             public:
                typedef typename Traits::type value_type;

            public:
                RangeUnitT()
                    : b_(0)
                    , e_(0)
                    , has_end_(false)
                {
                }

                RangeUnitT(
                    value_type b)
                    : b_(b)
                    , e_(0)
                    , has_end_(false)
                {
                }

                RangeUnitT(
                    value_type b, 
                    value_type e)
                    : b_(b)
                    , e_(e)
                    , has_end_(true)
                {
                }

            public:
                value_type begin() const
                {
                    return b_;
                }

                value_type end() const
                {
                    return e_;
                }

                bool has_end() const
                {
                    return has_end_;
                }

                value_type size() const
                {
                    return e_ - b_;
                }

            public:
                std::string to_string() const
                {
                    using namespace framework::string;

                    if (b_ >= 0) {
                        if (has_end()) {
                            return format(b_) + "-" + format(e_ - Traits::adjust_end);;
                        } else {
                            return format(b_) + "-";
                        }
                    } else {
                        return format(b_);
                    }
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    using namespace framework::string;
                    using namespace framework::system::logic_error;

                    boost::system::error_code ec = succeed;
                    std::string::size_type p = str.find('-');
                    if (p == 0) {
                        ec = parse2(str, b_);
                    } else if (p == str.size() - 1) {
                        ec = parse2(str.substr(0, p), b_);
                        has_end_ = false;
                    } else {
                        ec = parse2(str.substr(0, p), b_);
                        if (!ec) {
                            ec = parse2(str.substr(p + 1), e_);
                            e_ += Traits::adjust_end;
                        }
                        if (!ec) {
                            has_end_ = true;
                        }
                    }
                    return ec;
                }

                value_type b_;
                value_type e_;
                bool has_end_;
            };

            struct PrefixBytes;

            template <
                typename Traits
            >
            class RangeT
                : public std::deque<RangeUnitT<Traits> >
            {
            public:
                typedef typename Traits::type value_type;
                typedef RangeUnitT<Traits> Unit;

            public:
                RangeT()
                {
                }

                RangeT(
                    value_type b, 
                    value_type e)
                {
                    put(b, e);
                }

                RangeT(
                    value_type b)
                {
                    put(b);
                }

                void put(
                    value_type b)
                {
                    this->push_back(Unit(b));
                }

                void put(
                    value_type b, 
                    value_type e)
                {
                    this->push_back(Unit(b, e));
                }

                void pop()
                {
                    this->pop_front();
                }

            public:
                std::string to_string() const
                {
                    using namespace framework::string;
                    std::string prefix = traits_.prefix();
                    prefix.append(1, '=');
                    return join(this->begin(), this->end(), ",", prefix);
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    this->clear();
                    std::string prefix = traits_.prefix();
                    prefix.append(1, '=');
                    using namespace framework::string;
                    return slice<Unit>(str, std::inserter(*this, this->end()), ",", prefix);
                }

            private:
                Traits traits_;
            };

            template <
                typename Traits 
            >
            class ContentRangeT
            {
            public:
                typedef typename Traits::type value_type;
                typedef RangeUnitT<Traits> Unit;

            public:
                ContentRangeT(
                    value_type total = 0)
                    : total_(total)
                {
                }

                ContentRangeT(
                    value_type total, 
                    value_type b)
                    : total_(total)
                    , unit_(b, b - 1)
                {
                }

                ContentRangeT(
                    value_type total, 
                    value_type b, 
                    value_type e)
                    : total_(total)
                    , unit_(b, e)
                {
                }

                ContentRangeT(
                    value_type total, 
                    Unit const & unit)
                    : unit_(unit)
                {
                    if (!unit_.has_end()) {
                        unit_ = Unit(unit_.begin(), total);
                    }
                }

            public:
                Unit unit() const
                {
                    return unit_;
                }

                value_type total() const
                {
                    return total_;
                }

                value_type size() const
                {
                    return unit_.size();
                }

            public:
                std::string to_string() const
                {
                    return std::string(traits_.prefix()) + " " + unit_.to_string() + "/" + framework::string::format(total_);
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    std::string::size_type p = str.find('/');
                    std::string prefix = traits_.prefix();
                    prefix.append(1, ' ');
                    if (p == std::string::npos || p < prefix.size() || str.compare(0, prefix.size(), prefix)) {
                        return framework::system::logic_error::invalid_argument;
                    } else {
                        boost::system::error_code ec = 
                            unit_.from_string(str.substr(prefix.size(), p));
                        if (!ec)
                            ec = framework::string::parse2(str.substr(p + 1), total_);
                        return ec;
                    }
                }

            private:
                Traits traits_;
                value_type total_;
                Unit unit_;
            };

            struct RangeByteTraitsaits
            {
                typedef boost::uint64_t type;

                char const * prefix() const
                {
                    return "bytes";
                }
            };

            typedef RangeUnitT<boost::uint64_t> RangeUnit;
            typedef RangeT<RangeByteTraitsaits> Range;
            typedef ContentRangeT<RangeByteTraitsaits> ContentRange;

        } // namespace cmsg_field

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_CMSG_CMSG_FIELD_RANGE_H_
