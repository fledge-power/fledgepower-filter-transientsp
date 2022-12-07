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
#include "datapoint.h"
#include "reading.h"

#include <filterTransientSp.h>
#include <constantsTransient.h>
#include <datapointUtility.h>
#include <utility.h>

using namespace std;
using namespace DatapointUtility;

/**
 * Constructor for the LogFilter.
 *
 * We call the constructor of the base class and handle the initial
 * configuration of the filter.
 *
 * @param    filterName      The name of the filter
 * @param    filterConfig    The configuration category for this filter
 * @param    outHandle       The handle of the next filter in the chain
 * @param    output          A function pointer to call to output data to the next filter
 */
FilterTransientSp::FilterTransientSp(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output) :
                                FledgeFilter(filterName, filterConfig, outHandle, output)
{
    m_config = new ConfigPlugin();
}

/**
 * Destructor for this filter class
 */
FilterTransientSp::~FilterTransientSp() {
    delete m_config;
}

/**
 * Modification of configuration
 * 
 * @param jsonExchanged : configuration ExchangedData
*/
void FilterTransientSp::setJsonConfig(string jsonExchanged) {
    m_config->importExchangedData(jsonExchanged);
}

/**
 * The actual filtering code
 *
 * @param readingSet The reading data to filter
 */
void FilterTransientSp::ingest(READINGSET *readingSet) 
{
    lock_guard<mutex> guard(m_configMutex);
    std::vector<Reading*> vectorReadingTransient;
	
    // Filter enable, process the readings 
    if (isEnabled()) {        

        // Just get all the readings in the readingset
        const std::vector<Reading*> & readings = readingSet->getAllReadings();
        for (auto reading = readings.cbegin(); reading != readings.cend(); reading++) {
            
            // Get datapoints on readings
            Datapoints &dataPoints = (*reading)->getReadingData();
            string assetName = (*reading)->getAssetName();

            string beforeLog = assetName + " - FilterTransientSp::ingest : ";

            Datapoints *dpPivotTS = findDictElement(&dataPoints, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT);
            if (dpPivotTS == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT.c_str());
                continue;
            }

            Datapoints *dpGtis = findDictElement(dpPivotTS, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT);
            if (dpGtis == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT.c_str());
                continue;
            }

            string id = findStringElement(dpGtis, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ID);
            if (id.compare("") == 0) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ID.c_str());
                continue;
            }

            if (!m_config->isTransient(id)) {
                Logger::getLogger()->debug("%s Data %s missing from the configuration", beforeLog, id.c_str());
                continue;
            }

            bool typeSps = true;
            Datapoints *dpTyp = findDictElement(dpGtis, ConstantsTransient::JSON_CDC_SPS);
            if (dpTyp == nullptr) {
                dpTyp = findDictElement(dpGtis, ConstantsTransient::JSON_CDC_DPS);
                
                if (dpTyp == nullptr) {
                    Logger::getLogger()->debug("%s Missing CDC (%s and %s missing) attribute, it is ignored", beforeLog, ConstantsTransient::JSON_CDC_SPS.c_str(), ConstantsTransient::JSON_CDC_DPS.c_str());
                    continue;
                }
                typeSps = false;
            }            

            DatapointValue *valueTS = findValueElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
            if (valueTS == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL.c_str());
                continue;
            }

            if (typeSps) {
                if(valueTS->toInt() != 1) {
                    Logger::getLogger()->debug("%s The value is not 1, it is ignored", beforeLog);
                    continue;
                }
            }
            else {
                if(valueTS->toStringValue() != "on") {
                    Logger::getLogger()->debug("%s The value is not on, it is ignored", beforeLog);
                    continue;
                }
            }            
            Reading *r = generateReadingTransient((*reading));
            if (r != nullptr){
                Logger::getLogger()->debug("%s Generation of the reading [%s]", beforeLog, r->toJSON());
                vectorReadingTransient.push_back(r);
            }
        }
        
        readingSet->append(vectorReadingTransient);
    }
    (*m_func)(m_data, readingSet);
}

/**
 * 
*/
Reading *FilterTransientSp::generateReadingTransient(Reading *reading) {
    string beforeLog = reading->getAssetName() + " - FilterTransientSp::generateReadingTransient : ";
    
    // Deep copy on Datapoint
    Datapoint *dpRoot = reading->getDatapoint(ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT);
    if (dpRoot == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, transient creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT);
        return nullptr;
    }

    DatapointValue newValueTransient(dpRoot->getData());

    // Generate ouput reading todo
    Datapoints *dpGtis = findDictElement(newValueTransient.getDpVec(), ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT);
    if (dpGtis == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, transient creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT);
        return nullptr;
    }

    bool typeSps = true;
    Datapoints *dpTyp = findDictElement(dpGtis, ConstantsTransient::JSON_CDC_SPS);
    if (dpTyp == nullptr) {
        dpTyp = findDictElement(dpGtis, ConstantsTransient::JSON_CDC_DPS);
        
        if (dpTyp == nullptr) {
            Logger::getLogger()->debug("%s Attribute CDC missing, fugitive creation cancelled", beforeLog);
            return nullptr;
        }
        typeSps = false;
    }
    
    if (typeSps) {
        DatapointValue *valueTS = findValueElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        if (valueTS == nullptr) {
            Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
            return nullptr;
        }

        valueTS->setValue((long)0);
    }
    else {
        createStringElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL, "off");
    }

    Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_T);
    if (dpT == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_T);
        return nullptr;
    }
    
    DatapointValue *dpSinceEpo = findValueElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SECOND_S_E);
     if (dpSinceEpo == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SECOND_S_E);
        return nullptr;
    }

    DatapointValue *dpFractionSecond = findValueElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_FRACT_SEC);
    long msPart = 0;
    if (dpFractionSecond == nullptr) {
        Datapoint * dpFr = createIntegerElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_FRACT_SEC, 0);
        dpFractionSecond = &dpFr->getData();
    }

    long timestamp = Utility::toTimestamp(dpSinceEpo->toInt(), dpFractionSecond->toInt());
    timestamp += 1;
    std::pair<long, long> convertTimestamp = Utility::fromTimestamp(timestamp);

    dpSinceEpo->setValue(convertTimestamp.first);
    dpFractionSecond->setValue(convertTimestamp.second);

    Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_Q);
    if (dpQ == nullptr) {
        Datapoint *datapointQ = createDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_Q);
        dpQ = datapointQ->getData().getDpVec();
    }

    createStringElement(dpQ, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SOURCE, ConstantsTransient::VALUE_SUBSTITUTED);

    Datapoints *dpTmOrg = findDictElement(dpGtis, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
    if (dpTmOrg == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
        return nullptr;
    }

    createStringElement(dpTmOrg, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL, ConstantsTransient::VALUE_SUBSTITUTED);

    Datapoint *newDatapointTransient = new Datapoint(dpRoot->getName(), newValueTransient);
    Reading *newReading = new Reading(reading->getAssetName(), newDatapointTransient);
    return newReading;
}

/**
 * Reconfiguration entry point to the filter.
 *
 * This method runs holding the configMutex to prevent
 * ingest using the regex class that may be destroyed by this
 * call.
 *
 * Pass the configuration to the base FilterPlugin class and
 * then call the private method to handle the filter specific
 * configuration.
 *
 * @param newConfig  The JSON of the new configuration
 */
void FilterTransientSp::reconfigure(const std::string& newConfig) {
    lock_guard<mutex> guard(m_configMutex);
    setConfig(newConfig);

    ConfigCategory config("newConfig", newConfig);
    if (config.itemExists("exchanged_data")) {
        this->setJsonConfig(config.getValue("exchanged_data"));
    }
}
