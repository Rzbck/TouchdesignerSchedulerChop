#pragma once
#include "CHOP_CPlusPlusBase.h"
#include <array>
#include <string>

using namespace TD;

class SchedulerCHOP : public CHOP_CPlusPlusBase
{
public:
    SchedulerCHOP(const OP_NodeInfo* info);
    virtual ~SchedulerCHOP();

    void getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1) override;
    bool getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1) override;
    void getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1) override;
    void execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1) override;
    void setupParameters(OP_ParameterManager* manager, void* reserved1) override;

private:
    const OP_NodeInfo* myNodeInfo;

    // Paramètres globaux
    bool globalMode;
    int globalStartHour, globalStartMinute, globalStartSecond;
    int globalEndHour, globalEndMinute, globalEndSecond;
    double interval;
    double lastTriggerTime;

    // Jours actifs
    std::array<bool, 7> activeDays;
};
