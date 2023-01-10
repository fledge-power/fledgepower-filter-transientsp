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
	"PIVOT": {
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
	"PIVOT": {
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

static string jsonMessageWithoutT = QUOTE({
	"PIVOT": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "Identifier": "M_2367_3_15_4",
                "stVal": 1
            },
            "TmOrg": {
                "stVal": "genuine"
            }
        }
    }
});

static string jsonMessageWithoutSecondEpoch = QUOTE({
	"PIVOT": {
        "GTIS": {
            "SpsTyp": {
                "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "FractionOfSecond": 1
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

static string jsonMessageWithoutTmOrg = QUOTE({
	"PIVOT": {
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
            "Identifier": "M_2367_3_15_4"
        }
    }
});

static string jsonMessageWithoutStVal = QUOTE({
    "PIVOT": {
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
 
extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
};

class TestGenerateErrorTransient : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, nullptr);
		filter = (FilterTransientSp *) handle;
    }

    void startTest(string json) {
        // Create Reading
        Datapoints *p = parseJson(json);
        Reading *reading = new Reading(nameReading, *p);

        Reading *transient = filter->generateReadingTransient(reading);

        ASSERT_EQ(transient, nullptr);

        delete transient;
        delete reading;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }
};

TEST_F(TestGenerateErrorTransient, MessagePivotTM) 
{
    startTest(jsonMessagePivotTM);
}

TEST_F(TestGenerateErrorTransient, MessageGiTM) 
{
    startTest(jsonMessageGiTM);
}

TEST_F(TestGenerateErrorTransient, MessageMvTyp) 
{
    startTest(jsonMessageMvTyp);
}

TEST_F(TestGenerateErrorTransient, MessageWithoutT) 
{
    startTest(jsonMessageWithoutT);
}

TEST_F(TestGenerateErrorTransient, MessageWithoutSecondEpoch) 
{
    startTest(jsonMessageWithoutSecondEpoch);
}

TEST_F(TestGenerateErrorTransient, MessageWithoutTMOrg) 
{
    startTest(jsonMessageWithoutTmOrg);
}

TEST_F(TestGenerateErrorTransient, MessageWithoutStVal) 
{
    startTest(jsonMessageWithoutStVal);
}
