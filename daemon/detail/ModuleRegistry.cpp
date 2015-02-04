// ModuleRegistry.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.daemon.Daemon", framework::logger::Info);

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

            bool ModuleRegistry::startup(
                boost::system::error_code & ec)
            {
                boost::mutex::scoped_lock lock(mutex_);
                Module * module = first_module_;
                while (module) {
                    LOG_INFO("starting module " << module->name());
                    bool result = module->startup(ec);
                    if (!result) {
                        LOG_WARN("start module " << module->name() << " failed: " << ec.message());
                        break;
                    }
                    module = module->next_;
                }
                if (module) {
                    module = module->prev_;
                    while (module) {
                        LOG_INFO("shutdowning module " << module->name());
                        module->shutdown(ec);
                        module = module->prev_;
                    }
                } else {
                    is_started_ = true;
                }
                return is_started_;
            }

            bool ModuleRegistry::shutdown(
                boost::system::error_code & ec)
            {
                boost::mutex::scoped_lock lock(mutex_);
                if (!is_started_) {
                    ec.clear();
                    return true;
                }
                Module * module = last_module_;
                while (module) {
                    LOG_INFO("shutdowning module " << module->name());
                    module->shutdown(ec);
                    module = module->prev_;
                }
                is_started_ = false;
                return true;
            }

        } // namespace detail
    } // namespace daemon
} // namespace util
