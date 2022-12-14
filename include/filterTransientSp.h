#ifndef INCLUDE_FILTER_TRANSIENT_SP_H_
#define INCLUDE_FILTER_TRANSIENT_SP_H_

/*
 * Fledge filter sets value back to 0 for transient status point
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <datapointUtility.h>
#include <configPlugin.h>

#include <config_category.h>
#include <filter.h>
#include <mutex>
#include <string>

class FilterTransientSp  : public FledgeFilter
{
public:  
    FilterTransientSp(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output);

    void ingest(READINGSET *readingSet);
    void reconfigure(const std::string& newConfig);

    void setJsonConfig(const std::string& jsonExchanged);

    ConfigPlugin getConfigPlugin() { return m_configPlugin;} 
    Reading *generateReadingTransient(Reading *dps);

private:
    std::mutex      m_configMutex;
    ConfigPlugin    m_configPlugin;
};

#endif  // INCLUDE_FILTER_TRANSIENT_SP_H_