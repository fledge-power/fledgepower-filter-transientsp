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
#include <utilityPivot.h>

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
}

/**
 * Modification of configuration
 * 
 * @param jsonExchanged : configuration ExchangedData
*/
void FilterTransientSp::setJsonConfig(const string& jsonExchanged) {
    m_configPlugin.importExchangedData(jsonExchanged);
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

            string beforeLog = ConstantsTransient::NamePlugin + " - " + assetName + " - FilterTransientSp::ingest : ";

            Datapoints *dpPivotTS = findDictElement(&dataPoints, ConstantsTransient::KeyMessagePivotJsonRoot);
            if (dpPivotTS == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonRoot.c_str());
                continue;
            }

            Datapoints *dpGtis = findDictElement(dpPivotTS, ConstantsTransient::KeyMessagePivotJsonGt);
            if (dpGtis == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonGt.c_str());
                continue;
            }

            string id = findStringElement(dpGtis, ConstantsTransient::KeyMessagePivotJsonId);
            if (id.compare("") == 0) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonId.c_str());
                continue;
            }

            if (!m_configPlugin.isTransient(id)) {
                Logger::getLogger()->debug("%s Data %s missing from the configuration", beforeLog.c_str(), id.c_str());
                continue;
            }

            bool typeSps = true;
            Datapoints *dpTyp = findDictElement(dpGtis, ConstantsTransient::JsonCdcSps);
            if (dpTyp == nullptr) {
                dpTyp = findDictElement(dpGtis, ConstantsTransient::JsonCdcDps);
                
                if (dpTyp == nullptr) {
                    Logger::getLogger()->debug("%s Missing CDC (%s and %s missing) attribute, it is ignored", beforeLog.c_str(), ConstantsTransient::JsonCdcSps.c_str(), ConstantsTransient::JsonCdcDps.c_str());
                    continue;
                }
                typeSps = false;
            }            

            DatapointValue *valueTS = findValueElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonStVal);
            if (valueTS == nullptr) {
                Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonStVal.c_str());
                continue;
            }

            if (typeSps) {
                if(valueTS->toInt() != 1) {
                    Logger::getLogger()->debug("%s The value is not 1, it is ignored", beforeLog.c_str());
                    continue;
                }
            }
            else {
                if(valueTS->toStringValue() != "on") {
                    Logger::getLogger()->debug("%s The value is not on, it is ignored", beforeLog.c_str());
                    continue;
                }
            }            
            Reading *r = generateReadingTransient((*reading));
            if (r != nullptr){
                Logger::getLogger()->debug("%s Generation of the reading [%s]", beforeLog.c_str(), r->toJSON().c_str());
                vectorReadingTransient.push_back(r);
            }
        }
        
        readingSet->append(vectorReadingTransient);
    }
    (*m_func)(m_data, readingSet);
}

/**
 * Generate of reading for transient
 * 
 * @param reading initial reading
 * @return a modified reading
*/
Reading *FilterTransientSp::generateReadingTransient(Reading *reading) {
    string beforeLog = ConstantsTransient::NamePlugin + " - " + reading->getAssetName() + " - FilterTransientSp::generateReadingTransient : ";
    
    // Deep copy on Datapoint
    Datapoint *dpRoot = reading->getDatapoint(ConstantsTransient::KeyMessagePivotJsonRoot);
    if (dpRoot == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, transient creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonRoot.c_str());
        return nullptr;
    }

    DatapointValue newValueTransient(dpRoot->getData());

    // Generate ouput reading todo
    Datapoints *dpGtis = findDictElement(newValueTransient.getDpVec(), ConstantsTransient::KeyMessagePivotJsonGt);
    if (dpGtis == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, transient creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonGt.c_str());
        return nullptr;
    }

    bool typeSps = true;
    Datapoints *dpTyp = findDictElement(dpGtis, ConstantsTransient::JsonCdcSps);
    if (dpTyp == nullptr) {
        dpTyp = findDictElement(dpGtis, ConstantsTransient::JsonCdcDps);
        
        if (dpTyp == nullptr) {
            Logger::getLogger()->debug("%s Attribute CDC missing, fugitive creation cancelled", beforeLog.c_str());
            return nullptr;
        }
        typeSps = false;
    }
    
    if (typeSps) {
        DatapointValue *valueTS = findValueElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonStVal);
        if (valueTS == nullptr) {
            Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonStVal.c_str());
            return nullptr;
        }

        valueTS->setValue((long)0);
    }
    else {
        createStringElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonStVal, "off");
    }

    Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonT);
    if (dpT == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonT.c_str());
        return nullptr;
    }
    
    DatapointValue *dpSinceEpo = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonSecondSinceEpoch);
     if (dpSinceEpo == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonSecondSinceEpoch.c_str());
        return nullptr;
    }

    DatapointValue *dpFractionSecond = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonFractSec);
    if (dpFractionSecond == nullptr) {
        Datapoint * dpFr = createIntegerElement(dpT, ConstantsTransient::KeyMessagePivotJsonFractSec, 0);
        dpFractionSecond = &dpFr->getData();
    }

    long timestamp = UtilityPivot::toTimestamp(dpSinceEpo->toInt(), dpFractionSecond->toInt());
    timestamp += 1;
    std::pair<long, long> convertTimestamp = UtilityPivot::fromTimestamp(timestamp);

    dpSinceEpo->setValue(convertTimestamp.first);
    dpFractionSecond->setValue(convertTimestamp.second);

    Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonQ);
    if (dpQ == nullptr) {
        Datapoint *datapointQ = createDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonQ);
        dpQ = datapointQ->getData().getDpVec();
    }

    createStringElement(dpQ, ConstantsTransient::KeyMessagePivotJsonSource, ConstantsTransient::ValueSubstituted);

    Datapoints *dpTmOrg = findDictElement(dpGtis, ConstantsTransient::KeyMessagePivotJsonTmOrg);
    if (dpTmOrg == nullptr) {
        Logger::getLogger()->debug("%s Attribute %s missing, fugitive creation cancelled", beforeLog.c_str(), ConstantsTransient::KeyMessagePivotJsonTmOrg.c_str());
        return nullptr;
    }

    createStringElement(dpTmOrg, ConstantsTransient::KeyMessagePivotJsonStVal, ConstantsTransient::ValueSubstituted);

    auto newDatapointTransient = new Datapoint(dpRoot->getName(), newValueTransient);
    auto newReading = new Reading(reading->getAssetName(), newDatapointTransient);
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
