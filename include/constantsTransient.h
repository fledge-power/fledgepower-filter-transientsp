#ifndef INCLUDE_CONSTANTS_TRANSIENT_H_
#define INCLUDE_CONSTANTS_TRANSIENT_H_

#include <string>

struct ConstantsTransient {
    static const char *JSON_EXCHANGED_DATA;
    static const char *JSON_DATAPOINTS;
    static const char *JSON_PIVOT_TYPE;
    static const char *JSON_PIVOT_ID;
    static const char *JSON_TFID;
    static const char *JSON_DEADBAND;
    static const char *JSON_PARAMS;
    static const char *JSON_PIVOT_SUBTYPES ;

    static const std::string JSON_ATTR_MVTYP;
    static const std::string JSON_CDC_SPS;
    static const std::string JSON_CDC_DPS;

    static const std::string KEY_MESSAGE_PIVOT_JSON_ROOT;
    static const std::string KEY_MESSAGE_PIVOT_JSON_GT;
    static const std::string KEY_MESSAGE_PIVOT_JSON_ID;
    static const std::string KEY_MESSAGE_PIVOT_JSON_STVAL;
    static const std::string KEY_MESSAGE_PIVOT_JSON_SECOND_S_E;
    static const std::string KEY_MESSAGE_PIVOT_JSON_T;
    static const std::string KEY_MESSAGE_PIVOT_JSON_Q;
    static const std::string VALUE_SUBSTITUTED;
    static const std::string KEY_MESSAGE_PIVOT_JSON_TM_ORG;
    static const std::string KEY_MESSAGE_PIVOT_JSON_SOURCE;
};

#endif //INCLUDE_CONSTANTS_TRANSIENT_H_