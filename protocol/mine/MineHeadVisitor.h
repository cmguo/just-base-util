// MineHeadVisitor.hpp

#include "util/serialization/NVPair.h"
#include "util/archive/BasicIArchive.h"

#include <framework/string/Parse.h>
#include <framework/string/Format.h>

namespace util
{
    namespace protocol
    {
        class MineHeadVisitor;
    }

    namespace serialization
    {

        template <>
        struct is_primitive<util::protocol::MineHeadVisitor, std::map<std::string, std::vector<std::string> > >
            : boost::true_type
        {
        };

    }

    namespace protocol
    {

        class MineHeadVisitor
            : public util::archive::BasicIArchive<MineHeadVisitor>
        {
            friend class util::archive::BasicIArchive<MineHeadVisitor>;
            friend struct util::archive::LoadAccess;

        public:
            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::iterator fields_map_iterator;
            typedef fields_map::const_iterator fields_map_const_iterator;

        public:
            MineHeadVisitor(
                MineStringField & field)
                : field_(field)
            {
            }

            template <typename Head>
            void visit(Head & head)
            {
                this->operator >> (head);
            }

            std::auto_ptr<MineStringField::Handler> & handler()
            {
                return handler_;
            }

        private:
            /// 从流中读出变量
            template <typename T>
            class RegularHandler
                : public MineStringField::Handler
            {
            public:
                RegularHandler(
                    T & t)
                    : t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    if (!empty)
                        framework::string::parse2(value, t_);
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    empty = false;
                    framework::string::format2(value, t_);
                }

            private:
                T & t_;
            };

            template <typename T>
            class OptionalHandler
                : public MineStringField::Handler
            {
            public:
                OptionalHandler(
                    boost::optional<T> & t)
                    : t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    std::string str;
                    if (empty)
                        t_.reset();
                    else
                        t_.reset(framework::string::parse<T>(value));
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    if (t_.is_initialized()) {
                        empty = false;
                        framework::string::format2(value, t_.get());
                    } else {
                        empty = true;
                        value.clear();
                    }
                }

            private:
                boost::optional<T> & t_;
            };

            class OtherHandler
                : public MineStringField::Handler
            {
            public:
                OtherHandler(
                    std::string const & name, 
                    fields_map & t)
                    : name_(name)
                    , t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    std::string str;
                    if (empty) {
                        t_.erase(name_);
                    } else if (value[0] == '{') {
                        framework::string::parse2(value, t_[name_]);
                    } else {
                        t_[name_].resize(1);
                        framework::string::parse2(value, t_[name_][0]);
                    }
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    fields_map_const_iterator i = t_.find(name_);
                    if (i == t_.end()) {
                        empty = true;
                        value.clear();
                    } else {
                        empty = false;
                        framework::string::format2(value, i->second);
                    }
                }

            private:
                std::string name_;
                fields_map & t_;
            };

        private:
            template<class T>
            void load_wrapper(
                util::serialization::NVPair<T> & nvp)
            {
                if (nvp.name() == field_.name()) {
                    handler_.reset(get_handler(nvp.data()));
                }
            }

            void load(
                fields_map & other)
            {
                if (!handler_.get())
                    handler_.reset(new OtherHandler(field_.name(), other));
            }

            template<class T>
            MineStringField::Handler * get_handler(
                T & t)
            {
                return new RegularHandler<T>(t);
            }

            template<class T>
            MineStringField::Handler * get_handler(
                boost::optional<T> & t)
            {
                return new OptionalHandler<T>(t);
            }

        private:
            MineStringField & field_;
            std::auto_ptr<MineStringField::Handler> handler_;
        };

    } // namespace protocol
} // namespace util
