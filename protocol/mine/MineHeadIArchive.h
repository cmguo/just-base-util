// MineHeadIArchive.h

#ifndef _UTIL_PROTOCOL_MINE_MINE_HEAD_IARCHIVE_H_
#define _UTIL_PROTOCOL_MINE_MINE_HEAD_IARCHIVE_H_

#include "util/archive/BasicIArchive.h"

#include <framework/string/Algorithm.h>
#include <framework/string/Parse.h>
#include <framework/string/ParseStl.h>

namespace util
{
    namespace protocol
    {
        class MineHeadIArchive;
    }

    namespace serialization
    {

        template <typename T>
        struct is_primitive<util::protocol::MineHeadIArchive, boost::optional<T> >
            : boost::true_type
        {
        };

        template <>
        struct is_primitive<util::protocol::MineHeadIArchive, util::protocol::MineHead::fields_map>
            : boost::true_type
        {
        };

    }

    namespace protocol
    {

        class MineHeadIArchive
            : public util::archive::BasicIArchive<MineHeadIArchive>
        {
            friend class util::archive::BasicIArchive<MineHeadIArchive>;
            friend struct LoadAccess;

            typedef MineHead::fields_map fields_map;
            typedef fields_map::iterator fields_map_iterator;

        public:
            MineHeadIArchive(
                std::istream & is, 
                bool clear_optional = true)
                : clear_optional_(clear_optional)
                , sub_level_(0)
                , sub_item_(0)
            {
                std::string field;
                while (std::getline(is, field) && field != "\r") {
                    framework::string::trim(field);
                    if (field.empty())
                        break;
                    std::string::size_type p = field.find(':');
                    if (p != std::string::npos) {
                        std::string n = field.substr(0, p);
                        framework::string::trim(n);
                        std::string v = field.substr(p + 1);
                        framework::string::trim(v);
                        fields_[n].push_back(v);
                    }
                }
                iterator_ = fields_.end();
            }

            MineHeadIArchive(
                fields_map const & fields, 
                bool clear_optional = false)
                : clear_optional_(clear_optional)
                , fields_(fields)
                , iterator_(fields_.end())
                , sub_level_(0)
                , sub_item_(0)
            {
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.size()) {
                        state(2);
                    } else if (iterator_->second.size() == 1) {
                        boost::system::error_code ec = framework::string::parse2(iterator_->second.front(), t);
                        if (ec)
                            state(2);
                    } else {
                        state(2);
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ == size_t(-1)) {
                            t = T(iterator_->second.size());
                            sub_item_ = 0;
                        } else if (sub_item_ < iterator_->second.size()) {
                            boost::system::error_code ec = framework::string::parse2(iterator_->second[sub_item_], t);
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

            void load(
                std::string & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.empty()) {
                        state(2);
                    } else if (iterator_->second.size() == 1) {
                        t = iterator_->second.front();
                    } else {
                        state(2);
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ < iterator_->second.size()) {
                            t = iterator_->second[sub_item_];
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
            void load(
                boost::optional<T> & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.empty()) {
                        t.reset();
                    } else if (iterator_->second.size() == 1) {
                        T t1;
                        boost::system::error_code ec = framework::string::parse2(iterator_->second.front(), t1);
                        if (ec) {
                            state(2);
                        } else {
                            t.reset(t1);
                        }
                    } else {
                        state(2);
                    }
                } else {
                    state(2);
                }
            }

            using BasicIArchive<MineHeadIArchive>::load;

            void load(
                fields_map & other)
            {
                if (state())
                    return;
                for (fields_map_iterator i = fields_.begin(); i != fields_.end(); ++i) {
                    other[i->first] = i->second;
                }
            }

            void load_start(
                std::string const & name)
            {
                if (sub_level_ == 1)
                    iterator_ = fields_.find(name);
            }

            void load_end(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        fields_.erase(iterator_);
                        iterator_ = fields_.end();
                    }
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
            bool clear_optional_;
            fields_map fields_;
            fields_map_iterator iterator_;
            size_t sub_level_;
            size_t sub_item_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_HEAD_IARCHIVE_H_
