#ifndef INCLUDE_CONSTANTS_TRANSIENT_H_
#define INCLUDE_CONSTANTS_TRANSIENT_H_

#include <string>

#define FILTER_NAME "transientsp"

namespace ConstantsTransient {

    static const std::string NamePlugin            = FILTER_NAME;

    static const char *JsonExchangedData           = "exchanged_data";
    static const char *JsonDatapoints              = "datapoints";
    static const char *JsonPivotType               = "pivot_type";
    static const char *JsonPivotId                 = "pivot_id";
    static const char *JsonPivotSubtypes           = "pivot_subtypes";

    static const std::string JsonCdcSps     = "SpsTyp";
    static const std::string JsonCdcDps     = "DpsTyp";

    static const std::string KeyMessagePivotJsonRoot       = "PIVOT";
    static const std::string KeyMessagePivotJsonGt         = "GTIS";
    static const std::string KeyMessagePivotJsonId         = "Identifier";
    static const std::string KeyMessagePivotJsonStVal      = "stVal";
    static const std::string KeyMessagePivotJsonT          = "t";
    static const std::string KeyMessagePivotJsonSecondSinceEpoch = "SecondSinceEpoch";
    static const std::string KeyMessagePivotJsonFractSec   = "FractionOfSecond";
    static const std::string KeyMessagePivotJsonQ          = "q";
    static const std::string KeyMessagePivotJsonSource     = "Source";
    static const std::string ValueSubstituted              = "substituted";
    static const std::string KeyMessagePivotJsonTmOrg      = "TmOrg";
};

#endif //INCLUDE_CONSTANTS_TRANSIENT_H_