
#pragma once

#include <mutex>
#include <string>

#include "TemplateHelpers.h"
#include "cb/CbStrategyType.h"
#include "ev/EvStrategyType.h"

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_INFO(x)                                                                                                    \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

namespace psi::comm {

class BasicStrategy
{
public:
    BasicStrategy(const std::string &strategyName, const std::string &logPrefix)
        : m_strategyName(strategyName)
        , m_logPrefix(logPrefix)
    {
    }

protected:
    void logInfo(const std::string &msg)
    {
        if (!m_logPrefix.empty()) {
            LOG_INFO("[" << m_logPrefix << " " << m_strategyName << "] " << msg);
        }
    }

protected:
    const std::string m_strategyName;
    const std::string m_logPrefix;
    std::recursive_mutex m_mutex;

private:
    BasicStrategy(BasicStrategy &) = delete;
    BasicStrategy &operator=(BasicStrategy &) = delete;
};

template <CbStrategyType, typename CbArgs, typename InputComparable = int>
class CbStrategy;

template <EvStrategyType, typename EvArgs, typename CbArgs, typename InputArgs = TypeList<>>
class EvStrategy;

} // namespace psi::comm
