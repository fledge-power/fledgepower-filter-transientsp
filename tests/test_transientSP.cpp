#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterTransientSp.h>
#include <jsonToDatapoints.h>
#include <constantsTransient.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";
static int timestamp = 1669714183;

static string configure = QUOTE({
    "enable" :{
        "value": "true"
    },
    "exchanged_data": {
        "value" : {
            "exchanged_data": {
                "datapoints" : [          
                    {
                        "label":"TS-1",
                        "pivot_id":"M_2367_3_15_4",
                        "pivot_type":"SpsTyp",
                        "pivot_subtypes": [
                            "transient"
                        ],
                        "protocols":[
                            {
                                "name":"IEC104",
                                "typeid":"M_ME_NC_1",
                                "address":"3271612"
                            }
                        ]
                    },
                    {
                        "label":"TS-2",
                        "pivot_id":"M_2367_3_15_5",
                        "pivot_type":"DpsTyp",
                        "pivot_subtypes": [
                            "transient"
                        ],
                        "protocols":[
                            {
                                "name":"IEC104",
                                "typeid":"M_ME_NC_1",
                                "address":"3271612"
                            }
                        ]
                    }    
                ]
            }
        }
    }
});

static string jsonMessagePivotSpsTyp = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 1669714183
                },
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessagePivotDpsTyp = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "DpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 1669714183
                },
                "stVal": "on"
            },
            "Identifier": "M_2367_3_15_5",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessagePivotDpsTypWithoutQ = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "DpsTyp": {
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 1669714183
                },
                "stVal": "on"
            },
            "Identifier": "M_2367_3_15_5",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});
 
extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle, READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	
	void HandlerTestTransientSp(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}

    void plugin_reconfigure(PLUGIN_HANDLE *handle, const string& newConfig);
};

class TestTransientSp : public testing::Test
{
protected:
    FilterTransientSp * filter = nullptr;  // Object on which we call for tests
    ReadingSet * resultReading;

    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override
    {
        PLUGIN_INFORMATION *info = plugin_info();
		ConfigCategory *config = new ConfigCategory("transientsp", info->config);
		
		ASSERT_NE(config, (ConfigCategory *)NULL);		
		config->setItemsValueFromDefault();
		config->setValue("enable", "true");
		
		void *handle = plugin_init(config, &resultReading, HandlerTestTransientSp);
		filter = (FilterTransientSp *) handle;

        plugin_reconfigure((PLUGIN_HANDLE*)filter, configure);
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }

	void startTests(string json, string typeCDC) {
		ASSERT_NE(filter, (void *)NULL);

        // Create Reading
        Datapoints * p = parseJson(json);

		Reading *reading = new Reading(nameReading, *p);
        Readings *readings = new Readings;
        readings->push_back(reading);

        // Create ReadingSet
        ReadingSet * readingSet = new ReadingSet(readings);
		
        plugin_ingest(filter, (READINGSET*)readingSet);
        Readings results = resultReading->getAllReadings();
        ASSERT_EQ(results.size(), 2);

        Reading *outTs = results[0];
        ASSERT_STREQ(outTs->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(outTs->getDatapointCount(), 1);

        Datapoints pointsTs = outTs->getReadingData();
        ASSERT_EQ(pointsTs.size(), 1);

        verifyDatapointOrg(&pointsTs, typeCDC, json);

        Reading *outTsTransient = results[1];
        ASSERT_STREQ(outTsTransient->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(outTsTransient->getDatapointCount(), 1);

        Datapoints pointsTsTransient = outTsTransient->getReadingData();
        ASSERT_EQ(pointsTsTransient.size(), 1);

        verifyDatapointTransient(&pointsTsTransient, typeCDC);

        delete reading;
	}

	void verifyDatapointOrg(Datapoints *dps, string typeCDC, string json) {
		Datapoints *dpPivot = findDictElement(dps, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT);
		ASSERT_NE(dpPivot, nullptr);
		
		Datapoints *dpGi = findDictElement(dpPivot, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT);
		ASSERT_NE(dpGi, nullptr);

        Datapoints *tmOrg = findDictElement(dpGi, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
        ASSERT_NE(tmOrg, nullptr);

        DatapointValue *stValTmOrg = findValueElement(tmOrg, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        ASSERT_NE(stValTmOrg, nullptr);
        ASSERT_STREQ(stValTmOrg->toStringValue().c_str(), "genuine"); 
		
		Datapoints *dpTyp = findDictElement(dpGi, typeCDC);
		ASSERT_NE(dpTyp, nullptr);   

        Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_T);
		ASSERT_NE(dpT, nullptr);

        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SECOND_S_E);
		ASSERT_NE(sinceSecondEpoch, nullptr);
        ASSERT_EQ(sinceSecondEpoch->toInt(), timestamp);

        DatapointValue *stVal = findValueElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        ASSERT_NE(stVal, nullptr);

        if (typeCDC != "DpsTyp") {
            ASSERT_EQ(stVal->toInt(), 1);
        }
        else {
            ASSERT_STREQ(stVal->toStringValue().c_str(), "on");
        }

        if (json != jsonMessagePivotDpsTypWithoutQ) {
            Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_Q);
            ASSERT_NE(dpQ, nullptr);

            DatapointValue *vSource = findValueElement(dpQ, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SOURCE);
            ASSERT_NE(vSource, nullptr);
            ASSERT_STREQ(vSource->toStringValue().c_str(), "process"); 
        }
	}

    void verifyDatapointTransient(Datapoints *dps, string typeCDC) {
		Datapoints *dpPivot = findDictElement(dps, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_ROOT);
		ASSERT_NE(dpPivot, nullptr);
		
		Datapoints *dpGi = findDictElement(dpPivot, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_GT);
		ASSERT_NE(dpGi, nullptr);

        Datapoints *tmOrg = findDictElement(dpGi, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
        ASSERT_NE(tmOrg, nullptr);

        DatapointValue *stValTmOrg = findValueElement(tmOrg, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        ASSERT_NE(stValTmOrg, nullptr);
        ASSERT_STREQ(stValTmOrg->toStringValue().c_str(), ConstantsTransient::VALUE_SUBSTITUTED.c_str()); 
		
		Datapoints *dpTyp = findDictElement(dpGi, typeCDC);
		ASSERT_NE(dpTyp, nullptr);   

        Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_T);
		ASSERT_NE(dpT, nullptr);

        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SECOND_S_E);
		ASSERT_NE(sinceSecondEpoch, nullptr);
        ASSERT_EQ(sinceSecondEpoch->toInt(), timestamp + 1);

        DatapointValue *stVal = findValueElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        ASSERT_NE(stVal, nullptr);

        if (typeCDC != "DpsTyp") {
            ASSERT_EQ(stVal->toInt(), 0);
        }
        else {
            ASSERT_STREQ(stVal->toStringValue().c_str(), "off");
        }

		Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_Q);
        ASSERT_NE(dpQ, nullptr);

        DatapointValue *vSource = findValueElement(dpQ, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SOURCE);
        ASSERT_NE(vSource, nullptr);
        ASSERT_STREQ(vSource->toStringValue().c_str(), ConstantsTransient::VALUE_SUBSTITUTED.c_str()); 
	}
};

TEST_F(TestTransientSp, MessagePivotSpsTyp) 
{
	startTests(jsonMessagePivotSpsTyp, ConstantsTransient::JSON_CDC_SPS);
}

TEST_F(TestTransientSp, MessagePivotDpsTyp) 
{
	startTests(jsonMessagePivotDpsTyp, ConstantsTransient::JSON_CDC_DPS);
}

TEST_F(TestTransientSp, MessagePivotDpsTypWithoutQ) 
{
	startTests(jsonMessagePivotDpsTypWithoutQ, ConstantsTransient::JSON_CDC_DPS);
}
