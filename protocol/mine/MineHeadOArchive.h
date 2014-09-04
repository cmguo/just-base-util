// MineHeadOArchive.h

#ifndef _UTIL_PROTOCOL_MINE_MINE_HEAD_OARCHIVE_H_
#define _UTIL_PROTOCOL_MINE_MINE_HEAD_OARCHIVE_H_

#include "util/archive/BasicOArchive.h"

#include <framework/string/Algorithm.h>
#include <framework/string/Format.h>
#include <framework/string/FormatStl.h>

namespace util
{
    namespace protocol
    {
        class MineHeadOArchive;
    }

    namespace serialization
    {

        template <typename T>
        struct is_primitive<util::protocol::MineHeadOArchive, boost::optional<T> >
            : boost::true_type
        {
        };

        template <>
        struct is_primitive<util::protocol::MineHeadOArchive, util::protocol::MineHead::fields_map>
            : boost::true_type
        {
        };

    }

    namespace protocol
    {

        class MineHeadOArchive
            : public util::archive::BasicOArchive<MineHeadOArchive>
        {
            friend class util::archive::BasicOArchive<MineHeadOArchive>;
            friend struct LoadAccess;

            typedef MineHead::fields_map fields_map;
            typedef fields_map::iterator fields_map_iterator;
            typedef fields_map::const_iterator fields_map_const_iterator;

        public:
            MineHeadOArchive(
                std::ostream & os)
                : iterator_(fields_.end())
                , os_(os)
                , sub_level_(0)
                , sub_item_(0)
            {
            }

            ~MineHeadOArchive()
            {
                if (state())
                    return;
                for (fields_map_iterator i = fields_.begin(); i != fields_.end(); ++i) {
                    for (std::vector<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
                        os_ << i->first << ": " << *j << "\r\n";
                }
            }

        public:
            /// 从流中读出变量
            template<class T>
            void save(
                T const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            std::string str;
                            boost::system::error_code ec = framework::string::format2(str, t);
                            if (ec) {
                                state(2);
                            } else {
                                iterator_->second.push_back(str);
                            }
                        } else {
                            state(2);
                        }
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ == size_t(-1)) {
                            iterator_->second.resize(size_t(t));
                            sub_item_ = 0;
                        } else if (sub_item_ < iterator_->second.size()) {
                            boost::system::error_code ec = framework::string::format2(iterator_->second[sub_item_], t);
                            if (ec)
                                state(2);
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            void save(
                std::string const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            iterator_->second.push_back(t);
                        } else {
                            state(2);
                        }
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ < iterator_->second.size()) {
                            iterator_->second[sub_item_] = t;
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            template<class T>
            void save(
                boost::optional<T> const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            if (t.is_initialized()) {
                                iterator_->second.resize(1);
                                boost::system::error_code ec = framework::string::format2(iterator_->second.front(), t.get());
                                if (ec) {
                                    state(2);
                                }
                            }
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            void save(
                fields_map const & other)
            {
                for (fields_map_const_iterator i = other.begin(); state() == 0 && i != other.end(); ++i) {
                    if (!fields_.insert(*i).second) {
                        state(2);
                    }
                }
            }

            using BasicOArchive<MineHeadOArchive>::save;

            void save_start(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    iterator_ = fields_.find(name);
                    if (iterator_ == fields_.end())
                        iterator_ = fields_.insert(std::make_pair(name, std::vector<std::string>())).first;
                }
            }

            void save_end(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    iterator_ = fields_.end();
                }
            }

            void sub_start()
            {
                if (++sub_level_ == 2) {
                    sub_item_ = size_t(-1);
                }
            }

            void sub_end()
            {
                --sub_level_;
            }

        private:
            fields_map fields_;
            fields_map_iterator iterator_;
            std::ostream & os_;
            size_t sub_level_;
            size_t sub_item_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_HEAD_OARCHIVE_H_
