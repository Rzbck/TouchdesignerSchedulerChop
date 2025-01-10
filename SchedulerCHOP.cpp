#include "SchedulerCHOP.h"
#include <ctime>
#include <cmath>

extern "C" {
    DLLEXPORT
        void FillCHOPPluginInfo(CHOP_PluginInfo* info)
    {
        info->apiVersion = CHOPCPlusPlusAPIVersion;
        info->customOPInfo.opType->setString("Scheduler");
        info->customOPInfo.opLabel->setString("Scheduler");
        info->customOPInfo.authorName->setString("Data_C0re_");
        info->customOPInfo.authorEmail->setString("datacoredrive1@gmail.com");
        info->customOPInfo.minInputs = 0;
        info->customOPInfo.maxInputs = 0;
    }

    DLLEXPORT
        CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
    {
        return new SchedulerCHOP(info);
    }

    DLLEXPORT
        void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
    {
        delete (SchedulerCHOP*)instance;
    }
};

SchedulerCHOP::SchedulerCHOP(const OP_NodeInfo* info)
    : myNodeInfo(info), globalMode(false), interval(60.0), lastTriggerTime(0.0)
{
    activeDays.fill(true);
}

SchedulerCHOP::~SchedulerCHOP() {}

void SchedulerCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->cookEveryFrameIfAsked = true;
    ginfo->timeslice = true;
}

bool SchedulerCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
    info->numChannels = 1;
    info->sampleRate = 60; // Fréquence d'échantillonnage de 60 Hz
    return true;
}

void SchedulerCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
    name->setString("Trigger");
}

void SchedulerCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved1)
{
    globalMode = inputs->getParInt("Globalmode");
    interval = inputs->getParDouble("Globalinterval");

    // Désactiver les paramètres des jours individuels si le mode global est activé
    const std::array<std::string, 7> days = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    for (const auto& day : days)
    {
        std::string enableParamName = day + "enable";
        std::string startHourParamName = day + "starthour";
        std::string startMinuteParamName = day + "startminute";
        std::string startSecondParamName = day + "startsecond";
        std::string endHourParamName = day + "endhour";
        std::string endMinuteParamName = day + "endminute";
        std::string endSecondParamName = day + "endsecond";

        inputs->enablePar(enableParamName.c_str(), !globalMode);
        inputs->enablePar(startHourParamName.c_str(), !globalMode);
        inputs->enablePar(startMinuteParamName.c_str(), !globalMode);
        inputs->enablePar(startSecondParamName.c_str(), !globalMode);
        inputs->enablePar(endHourParamName.c_str(), !globalMode);
        inputs->enablePar(endMinuteParamName.c_str(), !globalMode);
        inputs->enablePar(endSecondParamName.c_str(), !globalMode);
    }

    std::time_t currentTime = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);

    int currentDay = localTime.tm_wday;
    double currentSeconds = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;

    bool isActiveDay = false;
    double startSeconds = 0.0, endSeconds = 0.0;

    if (globalMode)
    {
        startSeconds = inputs->getParInt("Globalstarthour") * 3600 +
            inputs->getParInt("Globalstartminute") * 60 +
            inputs->getParInt("Globalstartsecond");

        endSeconds = inputs->getParInt("Globalendhour") * 3600 +
            inputs->getParInt("Globalendminute") * 60 +
            inputs->getParInt("Globalendsecond");

        isActiveDay = true;
    }
    else
    {
        const std::string currentDayName = days[currentDay];
        if (inputs->getParInt((currentDayName + "enable").c_str()))
        {
            startSeconds = inputs->getParInt((currentDayName + "starthour").c_str()) * 3600 +
                inputs->getParInt((currentDayName + "startminute").c_str()) * 60 +
                inputs->getParInt((currentDayName + "startsecond").c_str());

            endSeconds = inputs->getParInt((currentDayName + "endhour").c_str()) * 3600 +
                inputs->getParInt((currentDayName + "endminute").c_str()) * 60 +
                inputs->getParInt((currentDayName + "endsecond").c_str());

            isActiveDay = true;
        }
    }

    bool isInTimeRange = (currentSeconds >= startSeconds && currentSeconds <= endSeconds);
    bool isIntervalElapsed = (currentSeconds - lastTriggerTime >= interval);

    float triggerValue = (isActiveDay && isInTimeRange && isIntervalElapsed) ? 1.0f : 0.0f;

    if (triggerValue == 1.0f)
    {
        lastTriggerTime = currentSeconds;
    }

    for (int i = 0; i < output->numSamples; ++i)
    {
        output->channels[0][i] = triggerValue;
    }
}

void SchedulerCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
    OP_NumericParameter np;

    np.page = "Global";

    np.name = "Globalmode";
    np.label = "Global Mode";
    manager->appendToggle(np);

    np.name = "Globalstarthour";
    np.label = "Start Hour";
    manager->appendInt(np);

    np.name = "Globalstartminute";
    np.label = "Start Minute";
    manager->appendInt(np);

    np.name = "Globalstartsecond";
    np.label = "Start Second";
    manager->appendInt(np);

    np.name = "Globalendhour";
    np.label = "End Hour";
    manager->appendInt(np);

    np.name = "Globalendminute";
    np.label = "End Minute";
    manager->appendInt(np);

    np.name = "Globalendsecond";
    np.label = "End Second";
    manager->appendInt(np);

    np.name = "Globalinterval";
    np.label = "Interval";
    manager->appendFloat(np);

    const std::array<std::string, 7> days = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    for (const auto& day : days)
    {
        np.page = day.c_str();

        std::string paramName;

        paramName = day + "enable";
        np.name = paramName.c_str();
        np.label = "Enable";
        manager->appendToggle(np);

        paramName = day + "starthour";
        np.name = paramName.c_str();
        np.label = "Start Hour";
        manager->appendInt(np);

        paramName = day + "startminute";
        np.name = paramName.c_str();
        np.label = "Start Minute";
        manager->appendInt(np);

        paramName = day + "startsecond";
        np.name = paramName.c_str();
        np.label = "Start Second";
        manager->appendInt(np);

        paramName = day + "endhour";
        np.name = paramName.c_str();
        np.label = "End Hour";
        manager->appendInt(np);

        paramName = day + "endminute";
        np.name = paramName.c_str();
        np.label = "End Minute";
        manager->appendInt(np);

        paramName = day + "endsecond";
        np.name = paramName.c_str();
        np.label = "End Second";
        manager->appendInt(np);
    }
}
