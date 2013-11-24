// // JsonIArchive.h

#ifndef _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_XML_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Optional.h"

#include <framework/string/Parse.h>

#include <iterator>

extern "C"
{
#include <thirdparty/jsmn/jsmn.h>
};

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class JsonIArchive
            : public StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>;
        public:
            JsonIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
            {
                load_json();
            }

            JsonIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
                load_json();
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                size_t i = token_stack_.back();
                jsmntok_t & v = tokens_[i];
                switch (v.type) {
                    case JSMN_PRIMITIVE:
                        if (framework::string::parse2(token_text(v), t)) {
                            this->state(2);
                        }
                        break;
                    case JSMN_ARRAY:
                        t = (T)v.end;
                        break;
                    default:
                        assert(false);
                        this->state(2);
                        break;
                }
            }

            /// 从流中读出bool
            void load(
                bool & t)
            {
                size_t i = token_stack_.back();
                jsmntok_t & v = tokens_[i];
                char const * value = NULL;
                if (v.type == JSMN_PRIMITIVE) {
                    if (token_text(v) == "true") {
                        t = true;
                    } else if (token_text(v) == "false") {
                        t = false;
                    } else {
                        this->state(2);
                    }
                } else {
                    this->state(2);
                }
            }

            /// 从流中读出标准库字符串
            void load(
                std::string & t)
            {
                size_t i = token_stack_.back();
                jsmntok_t & v = tokens_[i];
                char const * value = NULL;
                if (v.type == JSMN_STRING) {
                    t = token_text(v);
                } else {
                    this->state(2);
                }
            }

            /// 从流中读出可选值
            template<class T>
            void load(
                boost::optional<T> & t)
            {
                size_t i = token_stack_.back();
                if (i == (size_t)-1) {
                    t.reset();
                } else {
                    t.reset(T());
                    (*this) >> t.get();
                }
            }

            /// 从流中读出可选值
            template<class T>
            void load(
                util::serialization::optional_ref<T> & t)
            {
                size_t i = token_stack_.back();
                if (i == (size_t)-1) {
                    t.reset();
                } else {
                    t.serialize(*this);
                }
            }

            void load_start(
                std::string const & name)
            {
                size_t ip = token_stack_.back();
                jsmntok_t & vp = tokens_[ip];
                size_t ie = ip + vp.size;
                if (vp.type == JSMN_OBJECT) {
                    size_t i = ip + 1;
                    assert(ie <= tokens_.size());
                    while (i < ie) {
                        jsmntok_t & v = tokens_[i];
                        if (v.type != JSMN_STRING) {
                            assert(false);
                            this->state(2);
                            token_stack_.push_back(size_t(-1));
                            break;
                        }
                        if (str_.compare(v.start, v.end - v.start, name) == 0) {
                            // found
                            token_stack_.push_back(i + 1);
                            break;
                        }
                        ++i;
                        if (i < ie)
                            i += tokens_[i].size;
                    }
                    if (i >= ie) {
                        // not found
                        token_stack_.push_back(size_t(-1));
                    }
                } else if (vp.type == JSMN_ARRAY) {
                    if (name == "count") {
                        size_t c = 0;
                        size_t i = ip + 1;
                        while (i < ie && i + tokens_[i].size <= ie) {
                            ++c;
                            i += tokens_[i].size;
                        }
                        vp.start = ip + 1;
                        vp.end = c;
                        token_stack_.push_back(ip);
                    } else {
                        assert(name == "item");
                        size_t i = vp.start;
                        vp.start += tokens_[i].size;
                        assert(i < ie);
                        token_stack_.push_back(i);
                    }
                } else {
                    assert(false);
                    this->state(2);
                    token_stack_.push_back(size_t(-1));
                }
            }

            void load_end(
                std::string const & name)
            {
                token_stack_.pop_back();
            }

        private:
            void load_json()
            {
                std::basic_istream<_Elem, _Traits> is(this->buf_);
                std::istream_iterator<char, _Elem, _Traits> beg(is);
                std::istream_iterator<char, _Elem, _Traits> end;
                is >> std::noskipws;
                std::copy(beg, end, std::back_inserter(str_));

                jsmn_parser parser;
                jsmn_init(&parser);
                tokens_.resize(256);
                int result = jsmn_parse(&parser, str_.c_str(), &tokens_.front(), tokens_.size());
                while (result == JSMN_ERROR_NOMEM) {
                    tokens_.resize(tokens_.size() * 2);
                    result = jsmn_parse(&parser, str_.c_str(), &tokens_.front(), tokens_.size());
                }

                if (result != 0) {
                    this->state(1);
                } else {
                    tokens_.resize(parser.toknext);
                    adjust(&tokens_.front());
                    token_stack_.push_back(0);
                }
            }

            void adjust(
                jsmntok_t * t)
            {
                switch (t->type) {
                    case JSMN_OBJECT:
                    case JSMN_ARRAY:
                        {
                            int n = 0;
                            jsmntok_t * c = t + 1;
                            for (int i = 0; i < t->size; ++i) {
                                // name string without quote
                                if (t->type == JSMN_OBJECT && ((i & 1) == 0)) {
                                    if (c->type == JSMN_PRIMITIVE) {
                                        c->type = JSMN_STRING;
                                    }
                                }
                                adjust(c);
                                n += c->size; // add child size
                                c += c->size;
                            }
                            t->size = n;
                        }
                    default:
                        ++t->size; // add self
                }
            }

            std::string token_text(
                jsmntok_t const & t)
            {
                return std::string(str_.c_str() + t.start, t.end - t.start);
            }

        private:
            std::string str_;
            std::vector<jsmntok_t> tokens_;
            std::vector<size_t> token_stack_;
        };

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {

        template<
            typename _Elem, 
            typename _Traits, 
            class _T
        >
        struct is_primitive<util::archive::JsonIArchive<_Elem, _Traits>, boost::optional<_T> >
            : boost::true_type
        {
        };

        template<
            class _T
        >
        struct optional_ref;

        template<
            typename _Elem, 
            typename _Traits, 
            class _T
        >
        struct is_primitive<util::archive::JsonIArchive<_Elem, _Traits>, optional_ref<_T> >
            : boost::true_type
        {
        };

    }
}

#endif // _UTIL_ARCHIVE_XML_I_ARCHIVE_H_
