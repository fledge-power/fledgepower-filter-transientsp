#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterTransientSp.h>
#include <constantsTransient.h>
#include <utilityPivot.h>

using namespace std;
using namespace DatapointUtility;

static string nameReading = "data_test";
static long timestamp = 1669714183568;

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
	"PIVOT": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 9529458,
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
	"PIVOT": {
        "GTIS": {
            "DpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 9529458,
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
	"PIVOT": {
        "GTIS": {
            "DpsTyp": {
                "t": {
                    "FractionOfSecond": 9529458,
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

static string jsonMessagePivotSpsTypWithoutFractionOfSeconds = QUOTE({
	"PIVOT": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "SecondSinceEpoch": 1669714183
                },
                "stVal": 1
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
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
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
    FilterTransientSp *filter = nullptr;  // Object on which we call for tests
    ReadingSet *resultReading;

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

	void startTests(string json, string typeCDC, long ts=timestamp) {
		ASSERT_NE(filter, (void *)NULL);

        // Create Reading
        DatapointValue d("");
        Datapoint *tmp = new Datapoint("", d);
        Datapoints *p = tmp->parseJson(json);
        delete tmp;

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

        verifyDatapointOrg(&pointsTs, typeCDC, json, ts);

        Reading *outTsTransient = results[1];
        ASSERT_STREQ(outTsTransient->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(outTsTransient->getDatapointCount(), 1);

        Datapoints pointsTsTransient = outTsTransient->getReadingData();
        ASSERT_EQ(pointsTsTransient.size(), 1);

        verifyDatapointTransient(&pointsTsTransient, typeCDC, ts);

        delete reading;
        delete outTsTransient;
	}

	void verifyDatapointOrg(Datapoints *dps, string typeCDC, string json, long ts) {
		Datapoints *dpPivot = findDictElement(dps, ConstantsTransient::KeyMessagePivotJsonRoot);
		ASSERT_NE(dpPivot, nullptr);
		
		Datapoints *dpGi = findDictElement(dpPivot, ConstantsTransient::KeyMessagePivotJsonGt);
		ASSERT_NE(dpGi, nullptr);

        Datapoints *tmOrg = findDictElement(dpGi, ConstantsTransient::KeyMessagePivotJsonTmOrg);
        ASSERT_NE(tmOrg, nullptr);

        DatapointValue *stValTmOrg = findValueElement(tmOrg, ConstantsTransient::KeyMessagePivotJsonStVal);
        ASSERT_NE(stValTmOrg, nullptr);
        ASSERT_STREQ(stValTmOrg->toStringValue().c_str(), "genuine"); 
		
		Datapoints *dpTyp = findDictElement(dpGi, typeCDC);
		ASSERT_NE(dpTyp, nullptr);   

        Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonT);
		ASSERT_NE(dpT, nullptr);

        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonSecondSinceEpoch);
		ASSERT_NE(sinceSecondEpoch, nullptr);

        long fract = 0;
        if (json != jsonMessagePivotSpsTypWithoutFractionOfSeconds) {
            DatapointValue *fractionOfSecond = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonFractSec);
		    ASSERT_NE(fractionOfSecond, nullptr);
            fract = fractionOfSecond->toInt();
        }

        long time_calcul = UtilityPivot::toTimestamp(sinceSecondEpoch->toInt(), fract) ;
        ASSERT_EQ(ts, time_calcul);

        DatapointValue *stVal = findValueElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonStVal);
        ASSERT_NE(stVal, nullptr);

        if (typeCDC != "DpsTyp") {
            ASSERT_EQ(stVal->toInt(), 1);
        }
        else {
            ASSERT_STREQ(stVal->toStringValue().c_str(), "on");
        }

        if (json != jsonMessagePivotDpsTypWithoutQ) {
            Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonQ);
            ASSERT_NE(dpQ, nullptr);

            DatapointValue *vSource = findValueElement(dpQ, ConstantsTransient::KeyMessagePivotJsonSource);
            ASSERT_NE(vSource, nullptr);
            ASSERT_STREQ(vSource->toStringValue().c_str(), "process"); 
        }
	}

    void verifyDatapointTransient(Datapoints *dps, string typeCDC, long ts) {
		Datapoints *dpPivot = findDictElement(dps, ConstantsTransient::KeyMessagePivotJsonRoot);
		ASSERT_NE(dpPivot, nullptr);
		
		Datapoints *dpGi = findDictElement(dpPivot, ConstantsTransient::KeyMessagePivotJsonGt);
		ASSERT_NE(dpGi, nullptr);

        Datapoints *tmOrg = findDictElement(dpGi, ConstantsTransient::KeyMessagePivotJsonTmOrg);
        ASSERT_NE(tmOrg, nullptr);

        DatapointValue *stValTmOrg = findValueElement(tmOrg, ConstantsTransient::KeyMessagePivotJsonStVal);
        ASSERT_NE(stValTmOrg, nullptr);
        ASSERT_STREQ(stValTmOrg->toStringValue().c_str(), ConstantsTransient::ValueSubstituted.c_str()); 
		
		Datapoints *dpTyp = findDictElement(dpGi, typeCDC);
		ASSERT_NE(dpTyp, nullptr);   

        Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonT);
		ASSERT_NE(dpT, nullptr);

        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonSecondSinceEpoch);
		ASSERT_NE(sinceSecondEpoch, nullptr);
            
        DatapointValue *fractionOfSecond = findValueElement(dpT, ConstantsTransient::KeyMessagePivotJsonFractSec);
		ASSERT_NE(fractionOfSecond, nullptr);

        long time_calcul = UtilityPivot::toTimestamp(sinceSecondEpoch->toInt(), fractionOfSecond->toInt()) ;
        ASSERT_EQ(ts + 1, time_calcul);

        DatapointValue *stVal = findValueElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonStVal);
        ASSERT_NE(stVal, nullptr);

        if (typeCDC != "DpsTyp") {
            ASSERT_EQ(stVal->toInt(), 0);
        }
        else {
            ASSERT_STREQ(stVal->toStringValue().c_str(), "off");
        }

		Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KeyMessagePivotJsonQ);
        ASSERT_NE(dpQ, nullptr);

        DatapointValue *vSource = findValueElement(dpQ, ConstantsTransient::KeyMessagePivotJsonSource);
        ASSERT_NE(vSource, nullptr);
        ASSERT_STREQ(vSource->toStringValue().c_str(), ConstantsTransient::ValueSubstituted.c_str()); 
	}
};

TEST_F(TestTransientSp, MessagePivotSpsTyp) 
{
	startTests(jsonMessagePivotSpsTyp, ConstantsTransient::JsonCdcSps);
}

TEST_F(TestTransientSp, MessagePivotDpsTyp) 
{
	startTests(jsonMessagePivotDpsTyp, ConstantsTransient::JsonCdcDps);
}

TEST_F(TestTransientSp, MessagePivotDpsTypWithoutQ) 
{
	startTests(jsonMessagePivotDpsTypWithoutQ, ConstantsTransient::JsonCdcDps);
}

TEST_F(TestTransientSp, MessagePivotSpsTypWithoutFractionOfSeconds) 
{
	startTests(jsonMessagePivotSpsTypWithoutFractionOfSeconds, ConstantsTransient::JsonCdcSps, 1669714183000);
}
