#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterTransientSp.h>
#include <jsonToDatapoints.h>
#include <constantsTransient.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string jsonMessagePivotTM = QUOTE({
    "PIVOTTM": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
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

static string jsonMessageGiTM = QUOTE({
	"PIVOTTS": {
        "GTIM": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
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

static string jsonMessageMvTyp = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "MvTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
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

static string jsonMessageWithoutID = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
                },
                "stVal": 1
            },
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessageUnknownConfig = QUOTE({
	"PIVOTTS": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
                },
                "stVal": 1
            },
            "Identifier": "unknown",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessageWithoutStVal = QUOTE({
    "PIVOTTS": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
                }
            },
            "Identifier": "M_2367_3_15_4",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessageSpsDiffOf0 = QUOTE({
    "PIVOTTS": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
                },
                "stVal": 0
            },
            "Identifier": "M_2367_3_15_4",
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessageDpsDiffOfOn = QUOTE({
    "PIVOTTS": {
        "GTIS": {
            "DpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1,
                    "SecondSinceEpoch": 0
                },
                "stVal": "off"
            },
            "Identifier": "M_2367_3_15_4",
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
	
	void HandlerNoModifyData(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}
};

class NoModifyData : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, HandlerNoModifyData);
		filter = (FilterTransientSp *) handle;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }

	void startTests(string json, std::string namePivotData, std::string nameGi, std::string nameTyp) {
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
        ASSERT_EQ(results.size(), 1);

        Reading *out = results[0];
        ASSERT_STREQ(out->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(out->getDatapointCount(), 1);

        Datapoints points = out->getReadingData();
        ASSERT_EQ(points.size(), 1);

        verifyDatapoint(&points, namePivotData, nameGi, nameTyp, json);

        delete reading;
	}

	void verifyDatapoint(Datapoints *dps, std::string namePivotData, std::string nameGi, std::string nameTyp, string json) {
		Datapoints *dpPivot = findDictElement(dps, namePivotData);
		ASSERT_NE(dpPivot, nullptr);
		
		Datapoints *dpGi = findDictElement(dpPivot, nameGi);
		ASSERT_NE(dpGi, nullptr);

        Datapoints *tmOrg = findDictElement(dpGi, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_TM_ORG);
        ASSERT_NE(tmOrg, nullptr);

        DatapointValue *stValTmOrg = findValueElement(tmOrg, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
        ASSERT_NE(stValTmOrg, nullptr);
        ASSERT_STREQ(stValTmOrg->toStringValue().c_str(), "genuine"); 
		
		Datapoints *dpTyp = findDictElement(dpGi, nameTyp);
		ASSERT_NE(dpTyp, nullptr);   

        Datapoints *dpT = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_T);
		ASSERT_NE(dpT, nullptr);

        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SECOND_S_E);
		ASSERT_NE(sinceSecondEpoch, nullptr);
        ASSERT_EQ(sinceSecondEpoch->toInt(), 0);

        if (json != jsonMessageWithoutStVal) {
            DatapointValue *stVal = findValueElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_STVAL);
            ASSERT_NE(stVal, nullptr);

            if (nameTyp != "DpsTyp") {
                if (json != jsonMessageSpsDiffOf0) {
                    ASSERT_EQ(stVal->toInt(), 1);
                }
                else {
                    ASSERT_EQ(stVal->toInt(), 0);
                }
            }
            else {
                if (json != jsonMessageDpsDiffOfOn) {
                    ASSERT_EQ(stVal->toStringValue(), "on");
                }
                else {
                    ASSERT_EQ(stVal->toStringValue(), "off");
                }
            }
        }

		Datapoints *dpQ = findDictElement(dpTyp, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_Q);
        ASSERT_NE(dpQ, nullptr);

        DatapointValue *vSource = findValueElement(dpQ, ConstantsTransient::KEY_MESSAGE_PIVOT_JSON_SOURCE);
        ASSERT_NE(vSource, nullptr);
        ASSERT_STREQ(vSource->toStringValue().c_str(), "process"); 
    }
};

TEST_F(NoModifyData, MessagePIVOTTM) 
{
	startTests(jsonMessagePivotTM, "PIVOTTM", "GTIS", "SpsTyp");
}

TEST_F(NoModifyData, MessageGTIM) 
{
	startTests(jsonMessageGiTM, "PIVOTTS", "GTIM", "SpsTyp");
}

TEST_F(NoModifyData, MessageMvTyp) 
{
	startTests(jsonMessageMvTyp, "PIVOTTS", "GTIS", "MvTyp");
}

TEST_F(NoModifyData, MessageWithoutID) 
{
	startTests(jsonMessageWithoutID, "PIVOTTS", "GTIS", "SpsTyp");
}

TEST_F(NoModifyData, MessageUnknownConfig) 
{
	startTests(jsonMessageUnknownConfig, "PIVOTTS", "GTIS", "SpsTyp");
}

TEST_F(NoModifyData, MessageWithoutStVal) 
{
	startTests(jsonMessageWithoutStVal, "PIVOTTS", "GTIS", "SpsTyp");
}

TEST_F(NoModifyData, MessageSpsDiffOf0) 
{
	startTests(jsonMessageSpsDiffOf0, "PIVOTTS", "GTIS", "SpsTyp");
}

TEST_F(NoModifyData, MessageDpsDiffOfOn) 
{
	startTests(jsonMessageDpsDiffOfOn, "PIVOTTS", "GTIS", "DpsTyp");
}