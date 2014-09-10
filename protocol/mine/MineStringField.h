// MineStringField.h

#ifndef _UTIL_PROTOCOL_MINE_MINE_STRING_FIELD_H_
#define _UTIL_PROTOCOL_MINE_MINE_STRING_FIELD_H_

#include <memory>

namespace util
{
    namespace protocol
    {

        template <
            typename Head
        >
        class MineHeadT;

        class MineStringField
        {
        public:
            class Handler
            {
            public:
                virtual ~Handler() {};

            public:
                void set(
                    MineStringField const & field)
                {
                    set(field.empty_ , field.value_);
                }

                void get(
                    MineStringField & field)
                {
                    get(field.empty_ , field.value_);
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value) = 0;

                virtual void get(
                    bool & empty, 
                    std::string & value) const = 0;
            };

        public:
            MineStringField()
                : empty_(true)
            {
            }

            MineStringField(
                std::auto_ptr<Handler> & handler)
                : handler_(handler)
            {
                handler_->get(*this);
            }

            operator std::string() const
            {
                if (empty_)
                    return std::string();
                else
                    return value_;
            }

            MineStringField & operator=(
                std::string const & str)
            {
                empty_ = false;
                value_ = str;
                handler_->set(*this);
                return *this;
            }

            bool empty() const
            {
                return empty_;
            }

            void clear()
            {
                empty_ = true;
                handler_->set(*this);
            }

        private:
            friend class Handler;

            template <typename Head>
            friend class MineHeadT;

            bool empty_;
            std::string value_;
            std::auto_ptr<Handler> handler_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MINE_MINE_STRING_FIELD_H_
