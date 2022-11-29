#include "rapidjson/document.h"

#include <configPlugin.h>
#include <constantsTransient.h>

#include <logger.h>
#include <cctype>
#include <algorithm>

using namespace std;
using namespace rapidjson;

/**
 * Constructor
*/
ConfigPlugin::ConfigPlugin() {
    m_dataTransient.clear();
}

/**
 * Destructor
*/
ConfigPlugin::~ConfigPlugin() {
    deleteDataTransient();
}

/**
 * Deletion of loaded data
*/
void ConfigPlugin::deleteDataTransient() {
    m_dataTransient.clear();
}

/**
 * Import data in the form of Exchanged_data
 * The data is saved in a map m_exchangeDefinitions
 * 
 * @param exchangeConfig : configuration Exchanged_data as a string 
*/
void ConfigPlugin::importExchangedData(const string & exchangeConfig) {
    
    deleteDataTransient();
    Document document;

    if (document.Parse(const_cast<char*>(exchangeConfig.c_str())).HasParseError()) {
        Logger::getLogger()->fatal("Parsing error in data exchange configuration");
        printf("Parsing error in data exchange configuration\n");
        return;
    }

    if (!document.IsObject())
        return;

    if (!document.HasMember(ConstantsTransient::JSON_EXCHANGED_DATA) || !document[ConstantsTransient::JSON_EXCHANGED_DATA].IsObject()) {
        return;
    }
    const Value& exchangeData = document[ConstantsTransient::JSON_EXCHANGED_DATA];

    if (!exchangeData.HasMember(ConstantsTransient::JSON_DATAPOINTS) || !exchangeData[ConstantsTransient::JSON_DATAPOINTS].IsArray()) {
        return;
    }
    const Value& datapoints = exchangeData[ConstantsTransient::JSON_DATAPOINTS];

    for (const Value& datapoint : datapoints.GetArray()) {
        
        if (!datapoint.IsObject()) continue;
        
        if (!datapoint.HasMember(ConstantsTransient::JSON_PIVOT_TYPE) || !datapoint[ConstantsTransient::JSON_PIVOT_TYPE].IsString()) {
            continue;
        }

        string type = datapoint[ConstantsTransient::JSON_PIVOT_TYPE].GetString();
        if (type != ConstantsTransient::JSON_CDC_SPS && type != ConstantsTransient::JSON_CDC_DPS) {
            continue;
        }

        if (!datapoint.HasMember(ConstantsTransient::JSON_PIVOT_ID) || !datapoint[ConstantsTransient::JSON_PIVOT_ID].IsString()) {
            continue;
        }
        string pivot_id = datapoint[ConstantsTransient::JSON_PIVOT_ID].GetString();

        if (!datapoint.HasMember(ConstantsTransient::JSON_PIVOT_SUBTYPES) || !datapoint[ConstantsTransient::JSON_PIVOT_SUBTYPES].IsArray()) {
            continue;
        }

        bool transient = false;
        auto subtypes = datapoint[ConstantsTransient::JSON_PIVOT_SUBTYPES].GetArray();

        for (rapidjson::Value::ConstValueIterator itr = subtypes.Begin(); itr != subtypes.End(); ++itr) {
            string s = (*itr).GetString();
            if(s == "transient") {
                transient = true;
                break;
            }
        }

        if (transient == false) {
            continue;
        }
        
        m_dataTransient.push_back(pivot_id);
        Logger::getLogger()->debug("Configuration transient on id %s", pivot_id.c_str());

    }
}

/**
 * 
*/
bool ConfigPlugin::isTransient(std::string id) {
    if (std::find(m_dataTransient.begin(), m_dataTransient.end(), id) != m_dataTransient.end()){
        return true;
    }
    return false;
}