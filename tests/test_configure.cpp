#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterTransientSp.h>

using namespace std;

static string reconfigureErrorParseJSON = QUOTE({
    "exchanged_data" : {
        "eee"
    });

static string configureErrorExchanged_data = QUOTE({
    "configureErrorExchanged_data": {         
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
            }
        ]
    }
});

static string configureErrorDatapoint = QUOTE({
    "exchanged_data": {         
        "configureErrorDatapoint" : [          
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
            }
        ]
    }
});

static string configureErrorType = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
                "pivot_id":"M_2367_3_15_4",
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
});

static string configureErrorTypeMv = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
                "pivot_id":"M_2367_3_15_4",
                "pivot_type":"MvTyp",
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
});

static string configureErrorPivotID = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
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
            }
        ]
    }
});

static string configureErrorSubtypes = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
                "pivot_id":"M_2367_3_15_4",
                "pivot_type":"SpsTyp",
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
});

static string configureErrorSubtypesWithoutTransient = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
                "pivot_id":"M_2367_3_15_4",
                "pivot_type":"SpsTyp",
                "pivot_subtypes": [
                    "test"
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
});

static string configureOKSps = QUOTE({
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
            }
        ]
    }
});

static string configureOKDps = QUOTE({
    "exchanged_data": {         
        "datapoints" : [          
            {
                "label":"TS-1",
                "pivot_id":"M_2367_3_15_4",
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
});

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
};

class PluginConfigure : public testing::Test
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

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }   
};

TEST_F(PluginConfigure, ConfigureErrorParsingJSON) 
{
	filter->setJsonConfig(reconfigureErrorParseJSON);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorExchanged_data) 
{
	filter->setJsonConfig(configureErrorExchanged_data);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorDatapoint) 
{
	filter->setJsonConfig(configureErrorDatapoint);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorType) 
{
	filter->setJsonConfig(configureErrorType);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorTypeMv) 
{
	filter->setJsonConfig(configureErrorTypeMv);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorPivotID) 
{
	filter->setJsonConfig(configureErrorPivotID);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorSubtypes) 
{
	filter->setJsonConfig(configureErrorSubtypes);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}

TEST_F(PluginConfigure, ConfigureErrorSubtypesWithoutTransient) 
{
	filter->setJsonConfig(configureErrorSubtypesWithoutTransient);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
    ASSERT_FALSE(filter->getConfigPlugin().isTransient("M_2367_3_15_4"));
}

TEST_F(PluginConfigure, ConfigureOKSps) 
{
	filter->setJsonConfig(configureOKSps);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 1);
    ASSERT_TRUE(filter->getConfigPlugin().isTransient("M_2367_3_15_4"));
}

TEST_F(PluginConfigure, ConfigureOKDps) 
{
	filter->setJsonConfig(configureOKDps);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 1);
    ASSERT_TRUE(filter->getConfigPlugin().isTransient("M_2367_3_15_4"));
}

TEST_F(PluginConfigure, DeleteTransient) 
{
	filter->setJsonConfig(configureOKDps);
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 1);
    filter->getConfigPlugin().deleteDataTransient();
    ASSERT_EQ(filter->getConfigPlugin().getDataTransient().size(), 0);
}