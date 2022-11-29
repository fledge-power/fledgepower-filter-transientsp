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
    ~ConfigPlugin();

    void importExchangedData(const std::string & exchangeConfig);
    void deleteDataTransient();

    std::vector<std::string> getDataTransient() { return m_dataTransient; };

    bool isTransient(std::string id);
private:
    std::vector<std::string> m_dataTransient = std::vector<std::string>();
};

#endif  // INCLUDE_CONFIG_PLUGIN_H_