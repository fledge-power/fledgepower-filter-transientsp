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
 * Import data in the form of Exchanged_data
 * The data is saved in a map m_exchangeDefinitions
 * 
 * @param exchangeConfig : configuration Exchanged_data as a string 
*/
void ConfigPlugin::importExchangedData(const string & exchangeConfig) {
    
    m_dataTransient.clear();
    Document document;

    if (document.Parse(exchangeConfig.c_str()).HasParseError()) {
        Logger::getLogger()->fatal("Parsing error in data exchange configuration");
        printf("Parsing error in data exchange configuration\n");
        return;
    }

    if (!document.IsObject())
        return;

    if (!document.HasMember(ConstantsTransient::JsonExchangedData) || !document[ConstantsTransient::JsonExchangedData].IsObject()) {
        return;
    }
    const Value& exchangeData = document[ConstantsTransient::JsonExchangedData];

    if (!exchangeData.HasMember(ConstantsTransient::JsonDatapoints) || !exchangeData[ConstantsTransient::JsonDatapoints].IsArray()) {
        return;
    }
    const Value& datapoints = exchangeData[ConstantsTransient::JsonDatapoints];

    for (const Value& datapoint : datapoints.GetArray()) {
        
        if (!datapoint.IsObject()) continue;
        
        if (!datapoint.HasMember(ConstantsTransient::JsonPivotType) || !datapoint[ConstantsTransient::JsonPivotType].IsString()) {
            continue;
        }

        string type = datapoint[ConstantsTransient::JsonPivotType].GetString();
        if (type != ConstantsTransient::JsonCdcSps && type != ConstantsTransient::JsonCdcDps) {
            continue;
        }

        if (!datapoint.HasMember(ConstantsTransient::JsonPivotId) || !datapoint[ConstantsTransient::JsonPivotId].IsString()) {
            continue;
        }
        string pivot_id = datapoint[ConstantsTransient::JsonPivotId].GetString();

        if (!datapoint.HasMember(ConstantsTransient::JsonPivotSubtypes) || !datapoint[ConstantsTransient::JsonPivotSubtypes].IsArray()) {
            continue;
        }

        bool transient = false;
        auto subtypes = datapoint[ConstantsTransient::JsonPivotSubtypes].GetArray();

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
bool ConfigPlugin::isTransient(const std::string& id) {
    if (std::find(m_dataTransient.begin(), m_dataTransient.end(), id) != m_dataTransient.end()){
        return true;
    }
    return false;
}