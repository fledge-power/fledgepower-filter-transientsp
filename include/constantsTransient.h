#ifndef INCLUDE_CONSTANTS_TRANSIENT_H_
#define INCLUDE_CONSTANTS_TRANSIENT_H_

#include <string>

#define FILTER_NAME "transientsp"

struct ConstantsTransient {

    static const std::string NamePlugin;

    static const char *JsonExchangedData;
    static const char *JsonDatapoints;
    static const char *JsonPivotType;
    static const char *JsonPivotId;
    static const char *JsonTfid;
    static const char *JsonDeadband;
    static const char *JsonParams;
    static const char *JsonPivotSubtypes ;

    static const std::string JsonAttrMvtyp;
    static const std::string JsonCdcSps;
    static const std::string JsonCdcDps;

    static const std::string KeyMessagePivotJsonRoot;
    static const std::string KeyMessagePivotJsonGt;
    static const std::string KeyMessagePivotJsonId;
    static const std::string KeyMessagePivotJsonStVal;
    static const std::string KeyMessagePivotJsonFractSec;
    static const std::string KeyMessagePivotJsonSecondSinceEpoch;
    static const std::string KeyMessagePivotJsonT;
    static const std::string KeyMessagePivotJsonQ;
    static const std::string ValueSubstituted;
    static const std::string KeyMessagePivotJsonTmOrg;
    static const std::string KeyMessagePivotJsonSource;
};

#endif //INCLUDE_CONSTANTS_TRANSIENT_H_