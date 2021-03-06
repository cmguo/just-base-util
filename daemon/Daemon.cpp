// Daemon.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Daemon", framework::logger::Info);

namespace util
{
    namespace daemon
    {

        static inline std::vector<Daemon *> & daemon_set()
        {
            static std::vector<Daemon *> vec;
            return vec;
        }
        
        void Daemon::register_daemon(
            Daemon & daemon)
        {
            daemon_set().push_back(&daemon);
        }

        void Daemon::unregister_daemon(
            Daemon & daemon)
        {
            daemon_set().erase(std::find(daemon_set().begin(), daemon_set().end(), &daemon));
        }

        Daemon & Daemon::from_io_svc(
            boost::asio::io_service & io_svc)
        {
            Daemon * d = (Daemon *)&io_svc;
            d = (Daemon * )((char *)d - ((char *)&d->io_svc_ - (char *)d));
            if (std::find(daemon_set().begin(), daemon_set().end(), d) == daemon_set().end())
                d = NULL;
            return *d;
        }

        Daemon::Daemon()
            : io_work_(NULL)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            //logger_.load_config(config_);
            register_daemon(*this);
        }

        Daemon::Daemon(
            std::string const & conf)
            : io_work_(NULL)
            , config_(conf)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            register_daemon(*this);
            //logger_.load_config(config_);
        }

        Daemon::~Daemon()
        {
            unregister_daemon(*this);
            quick_stop();
            delete module_registry_;
        }

        int Daemon::parse_cmdline(
            int argc, 
            char const * argv[])
        {
            char const ** p = argv;
            for (int i = 0; i < argc; ++i) {
                if (argv[i][0] == '-' && argv[i][1] == '-') {
                    config_.profile().pre_set(argv[i] + 2);
                } else if (argv[i][0] == '+' && argv[i][1] == '+') {
                    config_.profile().post_set(argv[i] + 2);
                } else if (argv[i][0] == ':') {
                    char const * clone = strchr(argv[i] + 1, ':');
                    char const * equal = strchr(clone + 1, '=');
                    char const * key = strrchr(clone, '.');
                    config_.set_ext_config(std::string(argv[i] + 1, clone), 
                        std::string(clone + 1, key), 
                        std::string(key + 1, equal), equal + 1);
                } else {
                    *p++ = argv[i];
                }
            }
            return p - argv;
        }

        static void finish_notify(
            bool & result2, 
            boost::mutex & mutex, 
            boost::condition_variable & cond, 
            bool result)
        {
            boost::mutex::scoped_lock lock(mutex);
            result2 = result;
            cond.notify_all();
        }

        bool Daemon::start(
            boost::system::error_code & ec)
        {
            return start(0, ec);
        }

        bool Daemon::start(
            size_t concurrency, 
            boost::system::error_code & ec)
        {
            io_work_ = new boost::asio::io_service::work(io_svc_);
            bool result = false;
            LOG_INFO("[start] beg");
            if (concurrency == 0) {
                result = module_registry_->startup(ec);
            } else {
                boost::mutex mutex;
                boost::condition_variable cond;
                boost::mutex::scoped_lock lock(mutex);
                io_svc_.post(boost::bind(finish_notify, 
                    boost::ref(result), 
                    boost::ref(mutex), 
                    boost::ref(cond), 
                    boost::bind(&detail::ModuleRegistry::startup, module_registry_, boost::ref(ec))));
                for (size_t i = 0; i < concurrency; ++i) {
                    th_grp_.create_thread(boost::bind(&boost::asio::io_service::run, &io_svc_));
                }
                cond.wait(lock);
            }
            LOG_INFO("[start] end");
            if (!result) {
                LOG_INFO("[stop] beg");
                delete io_work_;
                io_work_ = NULL;
                result = run(ec);
            }
            return result;
        }

        bool Daemon::start(
            start_call_back_type const & start_call_back, 
            boost::system::error_code & ec)
        {
            LOG_INFO("[start] beg");
            io_work_ = new boost::asio::io_service::work(io_svc_);
            bool result = module_registry_->startup(ec);
            LOG_INFO("[start] end");
            start_call_back(ec);
            if (!result) {
            	LOG_INFO("[stop] beg");
                delete io_work_;
                io_work_ = NULL;
            }
            result = run(ec);
            return result;
        }

        bool Daemon::run(
            boost::system::error_code & ec)
        {
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
            } else {
                io_svc_.run(ec);
                io_svc_.reset();
            }
            LOG_INFO("[stop] end");
            return !ec;
        }

        bool Daemon::stop(
            boost::system::error_code & ec)
        {
            return stop(true, ec);
        }

        bool Daemon::stop(
            bool wait, 
            boost::system::error_code & ec)
        {
            bool result = false;
            delete io_work_;
            io_work_ = NULL;
            LOG_INFO("[stop] beg");
            if (th_grp_.size()) {
                boost::mutex mutex;
                boost::condition_variable cond;
                boost::mutex::scoped_lock lock(mutex);
                io_svc_.post(boost::bind(finish_notify, 
                    boost::ref(result), 
                    boost::ref(mutex), 
                    boost::ref(cond), 
                    boost::bind(&detail::ModuleRegistry::shutdown, module_registry_, boost::ref(ec))));
                cond.wait(lock);
            } else {
                result = module_registry_->shutdown(ec);
            }
            if (wait) {
                result = run(ec);
            }
            return result;
        }

        void Daemon::post_stop()
        {
            io_svc_.post(boost::bind(&Daemon::stop, this, false, boost::system::error_code()));
        }

        void Daemon::quick_stop()
        {
            if (io_work_) {
                delete io_work_;
                io_work_ = NULL;
            }
            io_svc_.stop();
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
            }
        }

        void Daemon::dump()
        {
            module_registry_->dump();
        }

        bool Daemon::is_started() const
        {
            return module_registry_->is_started();
        }

    } // namespace daemon
} // namespace util
