// ModuleRegistry.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.daemon.Daemon", Info);

namespace util
{
    namespace daemon
    {
        namespace detail
        {

            ModuleRegistry::ModuleRegistry(
                Daemon & daemon)
                : daemon_(daemon)
                , first_module_(NULL)
                , last_module_(NULL)
                , is_started_(false)
            {
            }

            ModuleRegistry::~ModuleRegistry()
            {
                while (last_module_) {
                    Module * prev_module = last_module_->prev_;
                    delete last_module_;
                    last_module_ = prev_module;
                }
            }

            boost::system::error_code ModuleRegistry::startup()
            {
                boost::mutex::scoped_lock lock(mutex_);
                boost::system::error_code ec;
                Module * module = first_module_;
                while (module) {
                    LOG_INFO("starting module " << module->name());
                    ec = module->startup();
                    if (ec) {
                        LOG_WARN("start module " << module->name() << " failed: " << ec.message());
                        break;
                    }
                    module = module->next_;
                }
                if (module) {
                    module = module->prev_;
                    while (module) {
                        LOG_INFO("shutdowning module " << module->name());
                        module->shutdown();
                        module = module->prev_;
                    }
                } else {
                    is_started_ = true;
                }
                return ec;
            }

            void ModuleRegistry::shutdown()
            {
                boost::mutex::scoped_lock lock(mutex_);
                if (!is_started_)
                    return;
                Module * module = last_module_;
                while (module) {
                    LOG_INFO("shutdowning module " << module->name());
                    module->shutdown();
                    module = module->prev_;
                }
                is_started_ = false;
            }

        } // namespace detail
    } // namespace daemon
} // namespace util
