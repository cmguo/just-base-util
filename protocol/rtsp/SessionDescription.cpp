// SessionDescription.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/SessionDescription.h"

#include <framework/string/Algorithm.h>

#include <algorithm>
#include <numeric>
#include <sstream>

namespace util
{
    namespace protocol
    {

        class Description::SectionDefine
        {
        public:
            SectionDefine(
                char const * sequc, 
                char const * flags)
                : nref_(0)
                , sequc_(sequc)
                , flags_(flags)
            {
                children_.resize(strlen(sequc));
            }

        public:
            size_t size() const
            {
                return children_.size();
            }

        public:
            void declare_child(
                boost::intrusive_ptr<SectionDefine> child)
            {
                char const * p = strchr(sequc_, child->sequc_[0]);
                size_t order = p - sequc_;
                children_[order] = child;
            }

            char key(
                size_t order) const
            {
                return sequc_[order];
            }

            char flag(
                size_t order) const
            {
                return flags_[order] - '0';
            }

            char flag(
                char key, 
                size_t & order) const
            {
                char const * p = strchr(sequc_, key);
                if (p == NULL)
                    return 8;
                order = p - sequc_;
                return flags_[order] - '0';
            }

            boost::intrusive_ptr<SectionDefine> child(
                size_t order)
            {
                return children_[order];
            }

        private:
            friend void intrusive_ptr_add_ref(
                SectionDefine * p)
            {
                ++p->nref_;
            }

            friend void intrusive_ptr_release(
                SectionDefine * p)
            {
                if (--p->nref_ == 0)
                    delete p;
            }

        private:
            size_t nref_;
            char const * sequc_;
            /* 
             * bit 1 - optional
             * bit 2 - mulitiple
             * bit 3 - child
             * bit 4 - parent, has parent, on first desc
             */
            char const * flags_;
            std::vector<boost::intrusive_ptr<SectionDefine> > children_;
        };

        struct Description::Item
        {
            // union
            std::string desc;
            boost::intrusive_ptr<Section> child;
        };

        class Description::Section
        {
        public:
            Section(
                boost::intrusive_ptr<SectionDefine> def)
                : nref_(0)
                , def_(def)
            {
                descs_.resize(def->size());
            }

        public:
            char add_line(
                char key, 
                std::string const & value)
            {
                size_t order = 0;
                char flag = def_->flag(key, order);
                if (flag & 8) {
                    return 8;
                }
                if (!descs_[order].empty() && (flag & 2) == 0) { // not multiple
                    return 2;
                }
                if (flag & 4) { // child
                    return 4;
                }
                Item item;
                item.desc = value;
                if (*--item.desc.end() == '\r')
                    item.desc.erase(item.desc.size() - 1);
                descs_[order].push_back(item);
                return 0;
            }

            boost::intrusive_ptr<Section> add_child(
                char key)
            {
                size_t order = 0;
                char flag = def_->flag(key, order);
                if ((flag & 4) == 0)
                    return NULL;
                Item item;
                item.child.reset(new Section(def_->child(order)));
                descs_[order].push_back(item);
                return item.child;
            }

            char add_child(
                char key, 
                boost::intrusive_ptr<Section> child)
            {
                size_t order = 0;
                char flag = def_->flag(key, order);
                if (flag & 8) {
                    return 8;
                }
                if (!descs_[order].empty() && (flag & 2) == 0) { // not multiple
                    return 2;
                }
                if ((flag & 4) == 0) { // child
                    return 4;
                }
                Item item;
                item.child = child;
                descs_[order].push_back(item);
                return 0;
            }

            char get_items(
                char key, 
                std::vector<Item> const * & items) const
            {
                size_t order = 0;
                char flag = def_->flag(key, order);
                if (flag & 8) {
                    return 8;
                }
                items = &descs_[order];
                return flag & 4;
            }

            void save(
                std::ostream & os) const
            {
                for (size_t i = 0; i < descs_.size(); ++i) {
                    for (size_t j = 0; j < descs_[i].size(); ++j) {
                        Item const & item = descs_[i][j];
                        if (def_->flag(i) & 4) {
                            item.child->save(os);
                        } else {
                            os << def_->key(i) << '=' << item.desc << std::endl;
                        }
                    }
                }
            }

            boost::intrusive_ptr<SectionDefine> def()
            {
                return def_;
            }

        private:
            friend void intrusive_ptr_add_ref(
                Section * p)
            {
                ++p->nref_;
            }

            friend void intrusive_ptr_release(
                Section * p)
            {
                if (--p->nref_ == 0)
                    delete p;
            }

        private:
            size_t nref_;
            boost::intrusive_ptr<SectionDefine> def_;
            typedef std::vector<Item> item_vec_t;
            std::vector<item_vec_t> descs_;
        };

        bool Description::desc_add(
            char key, 
            std::string const & value)
        {
            return 0 == root_->add_line(key, value);
        }

        Description::Description(
            char const * sequc,
            char const * flags)
        {
            boost::intrusive_ptr<SectionDefine> def(new SectionDefine(sequc, flags));
            root_.reset(new Section(def));
        }

        Description::Description(
            Description const & r)
            : root_(r.root_)
        {
        }

        Description::~Description()
        {
        }

        size_t Description::desc_cnt(
            char key) const
        {
            std::vector<Item> const * items;
            if (0 == root_->get_items(key, items)) {
                return items->size();
            } else {
                return 0;
            }
        }

        bool Description::desc_get(
            char key, 
            std::string & value, 
            size_t idx) const
        {
            std::vector<Item> const * items;
            if (0 == root_->get_items(key, items) && idx < items->size()) {
                value = items->at(idx).desc;
                return true;
            } else {
                return false;
            }
        }

        std::string const & Description::desc_get(
            char key, 
            size_t idx) const
        {
            std::vector<Item> const * items;
            if (0 == root_->get_items(key, items) && idx < items->size()) {
                return items->at(idx).desc;
            } else {
                static std::string empty;
                return empty;
            }
        }

        bool Description::attr_add(
            std::string const & key, 
            std::string const & value)
        {
            std::string value2 = key + ":" + value;
            return desc_add('a', value2);
        }

        struct Description::count_attr
        {
            count_attr(
                std::string const & key, 
                size_t idx)
                : key_(key)
                , index_(idx)
                , item_(NULL)
                , pitem_(&item_)
            {
                key_.append(1, ':');
            }

            size_t operator()(
                size_t total, 
                Item const & item)
            {
                if (item.desc.compare(0, key_.size(), key_) == 0) {
                    if (total == index_)
                        *pitem_ = &item;
                    ++total;
                }
                return total;
            }

            std::string value() const
            {
                return item_->desc.substr(key_.size());
            }

            size_t count(
                std::vector<Item> const & items)
            {
                return std::accumulate(items.begin(), items.end(), (size_t)0, *this);
            }

        private:
            std::string key_;
            size_t index_;
            Item const * item_;
            Item const ** pitem_; // when copy, pitem_ always point to the origin one
        };

        size_t Description::attr_cnt(
            std::string const & key) const
        {
            std::vector<Item> const * items;
            if (0 == root_->get_items('a', items)) {
                count_attr counter(key, (size_t)-1);
                return counter.count(*items);
            } else {
                return 0;
            }
        }

        bool Description::attr_get(
            std::string const & key, 
            std::string & value, 
            size_t idx) const
        {
            std::vector<Item> const * items;
            count_attr counter(key, idx);
            if (0 == root_->get_items('a', items) && idx < counter.count(*items)) {
                value = counter.value();
                return true;
            } else {
                return false;
            }
        }

        std::string Description::attr_get(
            std::string const & key, 
            size_t idx) const
        {
            std::vector<Item> const * items;
            count_attr counter(key, idx);
            if (0 == root_->get_items('a', items) && idx < counter.count(*items)) {
                return counter.value();
            } else {
                return std::string();
            }
        }

        bool Description::load(
            std::istream & is)
        {
            std::vector<boost::intrusive_ptr<Section> > stack;
            boost::intrusive_ptr<Section> s = root_;

            std::string line;
            while (std::getline(is, line)) {
                if (line.size() < 3 || line[1] != '=') {
                    is.setstate(std::ios::failbit);
                    break;
                }
                char key = line[0];
                std::string value = line.substr(2);
                char flag = s->add_line(key, value);
                if (flag) {
                    if (flag & 4) { // child
                        boost::intrusive_ptr<Section> child = s->add_child(key);
                        stack.push_back(s);
                        s = child;
                        flag = s->add_line(key, value);
                        assert(flag == 0);
                    } else if ((flag & 8) && !stack.empty()) { // not found
                        do {
                            s = stack.back();
                            stack.pop_back();
                            flag = s->add_line(key, value);
                        } while ((flag & 8) && stack.empty());
                    }
                    if (flag)
                        break;
                }
            } 
            if (is.eof())
                is.clear();

            return !!is;
        }
        
        bool Description::save(
            std::ostream & os) const
        {
            root_->save(os);
            return !!os;
        }

        boost::intrusive_ptr<Description::SectionDefine> Description::declare_child(
            char const * sequc, 
            char const * flags)
        {
            boost::intrusive_ptr<Description::SectionDefine> def(new SectionDefine(sequc, flags));
            root_->def()->declare_child(def);
            return def;
        }

        void Description::declare_child(
            boost::intrusive_ptr<SectionDefine> child)
        {
            root_->def()->declare_child(child);
        }

        Description::Description(
            boost::intrusive_ptr<Section> root)
            : root_(root)
        {
        }

        Description Description::add_child(
            char key)
        {
            return root_->add_child(key);
        }

        bool Description::add_child(
            char key, 
            Description const & desc)
        {
            return 0 == root_->add_child(key, desc.root_);
        }

        size_t Description::cnt_child(
            char key) const
        {
            std::vector<Item> const * items;
            if (4 == root_->get_items(key, items)) {
                return items->size();
            } else {
                return 0;
            }
        }

        Description Description::get_child(
            char key, 
            size_t idx) const
        {
            std::vector<Item> const * items;
            if (4 == root_->get_items(key, items) && idx < items->size()) {
                return items->at(idx).child;
            } else {
                return Description(NULL);
            }
        }

        SessionDescription::SessionDescription()
            : Description("vosiuepcbtzkam", "00011113361137")
        {
            declare_child("tr", "03");
            declare_child("micbka", "013313");
        }

        size_t SessionDescription::time_count() const
        {
            return cnt_child('t');
        }

        TimeDescription SessionDescription::time_add()
        {
            return add_child('t');
        }

        bool SessionDescription::time_add(
            TimeDescription const & desc)
        {
            return add_child('t', desc);
        }

        TimeDescription SessionDescription::time(
            size_t idx) const
        {
            return TimeDescription(get_child('t', idx));
        }

        size_t SessionDescription::media_count() const
        {
            return cnt_child('m');
        }

        MediaDescription SessionDescription::media_add()
        {
            return add_child('m');
        }

        bool SessionDescription::media_add(
            MediaDescription const & desc)
        {
            return add_child('m', desc);
        }

        MediaDescription SessionDescription::media(
            size_t idx) const
        {
            return MediaDescription(get_child('m', idx));
        }

        /* TimeDescription */

        TimeDescription::TimeDescription()
            : Description("tr", "03")
        {
        }

        TimeDescription::TimeDescription(
            Description const & desc)
            : Description(desc)
        {
        }

        /* MediaDescription */

        MediaDescription::MediaDescription()
            : Description("micbka", "013313")
        {
        }

        MediaDescription::MediaDescription(
            Description const & desc)
            : Description(desc)
        {
        }

        bool MediaDescription::set_rtp(
            std::string const & type,
            int port,
            int fmt)
        {
            std::ostringstream oss;
            oss << type << " " << port << " RTP/AVP " << fmt;
            return desc_add('m', oss.str());
        }

        bool MediaDescription::add_rtpmap(
            int fmt,
            std::string const & codec,
            int clock,
            std::string const & parameters)
        {
            std::ostringstream oss;
            oss << fmt << " " << codec << "/" << clock;
            if (!parameters.empty())
                oss << "/" << parameters;
            return attr_add("rtpmap", oss.str());
        }

        bool MediaDescription::add_fmtp(
            int fmt,
            std::string const & parameters)
        {
            std::ostringstream oss;
            oss << fmt << " " << parameters;
            return attr_add("fmtp", oss.str());
        }

        bool MediaDescription::get_rtp(
            std::string & type,
            int & port,
            int & fmt)
        {
            std::string value = desc_get('m');
            if (value.empty())
                return false;
            std::istringstream iss(value);
            std::string transport;
            iss >> type >> port >> transport >> fmt;
            if (transport != "RTP/AVP")
                return false;
            return !!iss;
        }

        bool MediaDescription::get_rtpmap(
            int fmt,
            std::string & codec,
            int & clock,
            std::string & parameters)
        {
            size_t cnt = attr_cnt("rtpmap");
            std::string value;
            if (fmt == 0 && cnt > 1) {
                return false;
            } else if (fmt == 0) {
                value = attr_get("rtpmap");
            } else {
                for (size_t i = 0; i < cnt; ++i) {
                    value = attr_get("rtpmap", i);
                    std::istringstream iss(value);
                    int fmt2;
                    iss >> fmt2;
                    if (fmt == fmt2) {
                        break;
                    }
                }
            }
            std::istringstream iss(value);
            iss >> fmt;
            iss >> std::ws;
            std::getline(iss, codec, '/');
            iss >> clock;
            if (!iss.eof()) {
                iss.ignore();
                std::getline(iss, parameters);
            }
            return !iss;
        }

        bool MediaDescription::get_fmtp(
            int fmt,
            std::string & parameters)
        {
            size_t cnt = attr_cnt("fmtp");
            std::string value;
            if (fmt == 0 && cnt > 1) {
                return false;
            } else if (fmt == 0) {
                value = attr_get("fmtp");
            } else {
                for (size_t i = 0; i < cnt; ++i) {
                    value = attr_get("fmtp", i);
                    std::istringstream iss(value);
                    int fmt2;
                    iss >> fmt2;
                    if (fmt == fmt2) {
                        break;
                    }
                }
            }
            std::istringstream iss(value);
            iss >> fmt;
            iss >> std::ws;
            std::getline(iss, parameters);
            return !!iss;
        }

    } // namespace protocol
} // namespace util

