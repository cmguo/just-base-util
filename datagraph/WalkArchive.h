// WalkArchive.h

#ifndef _UTIL_DATA_GRAPH_WALK_ARCHIVE_H_
#define _UTIL_DATA_GRAPH_WALK_ARCHIVE_H_

#include "util/archive/BasicOArchive.h"

#include <framework/string/Format.h>
#include <framework/string/FormatStl.h>
#include <framework/string/Parse.h>
#include <framework/string/Slice.h>

#include <iterator>

namespace util
{
    namespace datagraph
    {

        struct EqualChild
            : public util::archive::BasicOArchive<EqualChild>
        {
        public:
            EqualChild(
                std::string const & condition)
                : found_(false)
            {
                std::string::size_type p = condition.find('=');
                name_ = condition.substr(1, p - 1);
                value_ = condition.substr(p + 1);
            }

            operator bool () const
            {
                return found_;
            }

        protected:
            friend class util::archive::SaveAccess;

            typedef util::archive::BasicOArchive<EqualChild> super;

            using super::This;

            template <typename T>
            void save_wrapper(
                T const & t)
            {
                if (name_.empty()) return;
                if (wrapper_name(t) == name_) {
                    if (framework::string::format(wrapper_data(t)) == value_)
                        found_ = true;
                    else
                        this->fail();
                }
            }

        private:
            std::string name_;
            std::string value_;
            bool found_;
        };

        template <typename Archive>
        class WalkArchive
            : public util::archive::BasicOArchive<Archive>
        {
        public:
            WalkArchive(
                std::string const & path = "")
                : level_(0)
                , found_(false)
            {
                set_path(path);
            }

            void set_path(
                std::string const & path)
            {
                framework::string::slice<std::string>(path, std::back_inserter(path_), "/", "/");
                level_ = 0;
                found_ = false;
            }

        protected:
            friend class util::archive::SaveAccess;

            typedef util::archive::BasicOArchive<Archive> super;

            using super::This;
            using super::save_catalog;

            template <typename T>
            void save_wrapper(
                T const & t)
            {
                if (found_) return;
                if (path_[level_] != wrapper_name(t)) return;
                ++level_;
                if (level_ == path_.size()) {
                    found_ = true;
                    This()->found(wrapper_data(t));
                } else {
                    This()->operator <<(wrapper_data(t));
                    if (!found_)
                        this->fail();
                }
                --level_;
            }

            template <class T, class C>
            void save_catalog(
                T const & t, C * c)
            {
                if (path_.size() == 0)
                    This()->found(t);
                else
                    super::save_catalog(t, c);
            }

            template <class T>
            void save_catalog(
                T const & t, util::archive::catalog_collection *)
            {
                if (path_[level_][0] != '@') {
                    this->failed();
                    return;
                }
                if (path_[level_].find('=') != std::string::npos) {
                    typename T::const_iterator iter(t.begin());
                    for (; iter != t.end(); ++iter) {
                        EqualChild eq(path_[level_]);
                        if (eq << *iter) {
                            save_wrapper(util::serialization::make_nvp(path_[level_].c_str(), *iter));
                            return;
                        }
                        eq.clear();
                    }
                } else {
                    size_t n = framework::string::parse<size_t>(path_[level_].substr(1));
                    if (n >= t.size()) {
                        this->fail();
                        return;
                    }
                    typename T::const_iterator iter(t.begin());
                    std::advance(iter, n);
                    save_wrapper(util::serialization::make_nvp(path_[level_].c_str(), *iter));
                }
            }

            template <class T>
            void found(
                T const & t)
            {
            }

        private:
            std::vector<std::string> path_;
            size_t level_;
            bool found_;
        };

    } // namespace datagraph
} // namespace util

#endif // _UTIL_DATA_GRAPH_WALK_ARCHIVE_H_
