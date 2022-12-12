#ifndef INCLUDE_CONFIG_PLUGIN_H_
#define INCLUDE_CONFIG_PLUGIN_H_

/*
 * Import confiugration Exchanged data
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */
#include <vector>
#include <string>

class ConfigPlugin {
public:  
    ConfigPlugin();

    void importExchangedData(const std::string & exchangeConfig);
    void deleteDataTransient();
    bool isTransient(const std::string& id);

    const std::vector<std::string> getDataTransient() { return m_dataTransient; };
    
private:
    std::vector<std::string> m_dataTransient;
};

#endif  // INCLUDE_CONFIG_PLUGIN_H_