// MineFieldRange.h

#ifndef _UTIL_PROTOCOL_MINE_MINE_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_MINE_MINE_FIELD_RANGE_H_

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
        namespace mine_field
        {

            template <
                typename T
            >
            struct RangeUnit
            {
            public:
                RangeUnit()
                    : b_(0)
                    , e_(0)
                    , has_end_(false)
                {
                }

                RangeUnit(
                    T b)
                    : b_(b)
                    , e_(0)
                    , has_end_(false)
                {
                }

                RangeUnit(
                    T b, 
                    T e)
                    : b_(b)
                    , e_(e)
                    , has_end_(true)
                {
                }

            public:
                T begin() const
                {
                    return b_;
                }

                T end() const
                {
                    return e_;
                }

                bool has_end() const
                {
                    return has_end_;
                }

            public:
                std::string to_string() const
                {
                    using namespace framework::string;

                    if (b_ >= 0) {
                        if (has_end()) {
                            return format(b_) + "-" + format(e_);
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
                        }
                        if (!ec) {
                            has_end_ = true;
                        }
                    }
                    return ec;
                }

                T b_;
                T e_;
                bool has_end_;
            };

            struct PrefixBytes;

            template <
                typename T, 
                typename P = PrefixBytes// prefix
            >
            class Range
            {
            public:
                typedef RangeUnit<T> Unit;

            public:
                Range(
                    T b, 
                    T e)
                {
                    add_range(b, e);
                }

                Range(
                    T b = 0)
                {
                    add_range(b);
                }

                void add_range(
                    T b)
                {
                    units_.push_back(Unit(b));
                }

                void add_range(
                    T b, 
                    T e)
                {
                    units_.push_back(Unit(b, e));
                }

            public:
                std::string to_string() const
                {
                    using namespace framework::string;
                    std::string prefix = prefix_();
                    prefix.append(1, '=');
                    return join(units_.begin(), units_.end(), ",", prefix);
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    units_.clear();
                    std::string prefix = prefix_();
                    prefix.append(1, '=');
                    using namespace framework::string;
                    return slice<Unit>(str, std::inserter(units_, units_.end()), ",", prefix);
                }

            public:
                Unit & operator[](
                    size_t index)
                {
                    return units_[index];
                }

                Unit const & operator[](
                    size_t index) const
                {
                    return units_[index];
                }

            private:
                std::vector<Unit> units_;
                P prefix_;
            };

            template <
                typename T, 
                typename P = PrefixBytes// prefix
            >
            class ContentRange
            {
            public:
                typedef RangeUnit<T> Unit;

            public:
                ContentRange(
                    T total = 0)
                    : total_(total)
                {
                }

                ContentRange(
                    T total, 
                    T b)
                    : total_(total)
                    , unit_(b, b - 1)
                {
                }

                ContentRange(
                    T total, 
                    T b, 
                    T e)
                    : total_(total)
                    , unit_(b, e)
                {
                }

            public:
                Unit unit() const
                {
                    return unit_;
                }

                T total() const
                {
                    return total_;
                }

            public:
                std::string to_string() const
                {
                    return std::string(R::prefix()) + " " + unit_.to_string() + "/" + framework::string::format(total_);
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    std::string::size_type p = str.find('/');
                    std::string prefix = prefix_();
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
                P prefix_;
                T total_;
                Unit unit_;
            };

            struct PrefixBytes
            {
                char const * operator()() const
                {
                    return "bytes";
                }
            };

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_FIELD_RANGE_H_
