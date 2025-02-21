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
}

SchedulerCHOP::SchedulerCHOP(const OP_NodeInfo* info)
    : myNodeInfo(info), globalMode(false), interval(60.0), lastTriggerTime(-1.0)
{
    activeDays.fill(false);
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
    info->sampleRate = 60; // Fr�quence d'�chantillonnage de 60 Hz
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

    const std::array<std::string, 7> days = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    // R�cup�rer l'heure et la date courantes
    std::time_t currentTime = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);

    int currentDay = localTime.tm_wday; // Jour actuel (0 = Dimanche)
    double currentSeconds = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;

    // Correction du d�calage des jours
    const std::string currentDayName = days[(currentDay + 6) % 7];

    // D�sactiver/activer les param�tres des jours selon le mode
    for (const auto& day : days)
    {
        inputs->enablePar((day + "enable").c_str(), !globalMode);
        inputs->enablePar((day + "starthour").c_str(), !globalMode);
        inputs->enablePar((day + "startminute").c_str(), !globalMode);
        inputs->enablePar((day + "startsecond").c_str(), !globalMode);
        inputs->enablePar((day + "endhour").c_str(), !globalMode);
        inputs->enablePar((day + "endminute").c_str(), !globalMode);
        inputs->enablePar((day + "endsecond").c_str(), !globalMode);
        inputs->enablePar((day + "interval").c_str(), !globalMode);
    }

    bool isActiveDay = false;
    double startSeconds = 0.0, endSeconds = 0.0;
    double dayInterval = interval; // Intervalle � utiliser

    if (globalMode)
    {
        // Mode global
        double globalStartHour = inputs->getParDouble("Globalstarthour");
        double globalStartMinute = inputs->getParDouble("Globalstartminute");
        double globalStartSecond = inputs->getParDouble("Globalstartsecond");

        double globalEndHour = inputs->getParDouble("Globalendhour");
        double globalEndMinute = inputs->getParDouble("Globalendminute");
        double globalEndSecond = inputs->getParDouble("Globalendsecond");

        startSeconds = globalStartHour * 3600 + globalStartMinute * 60 + globalStartSecond;
        endSeconds = globalEndHour * 3600 + globalEndMinute * 60 + globalEndSecond;

        isActiveDay = true; // Toujours actif en mode global
    }
    else
    {
        // Mode sp�cifique au jour
        if (inputs->getParInt((currentDayName + "enable").c_str()))
        {
            double dayStartHour = inputs->getParDouble((currentDayName + "starthour").c_str());
            double dayStartMinute = inputs->getParDouble((currentDayName + "startminute").c_str());
            double dayStartSecond = inputs->getParDouble((currentDayName + "startsecond").c_str());

            double dayEndHour = inputs->getParDouble((currentDayName + "endhour").c_str());
            double dayEndMinute = inputs->getParDouble((currentDayName + "endminute").c_str());
            double dayEndSecond = inputs->getParDouble((currentDayName + "endsecond").c_str());

            startSeconds = dayStartHour * 3600 + dayStartMinute * 60 + dayStartSecond;
            endSeconds = dayEndHour * 3600 + dayEndMinute * 60 + dayEndSecond;

            dayInterval = inputs->getParDouble((currentDayName + "interval").c_str());
            isActiveDay = true;
        }
    }

    // V�rification de la plage horaire active
    bool isInTimeRange = false;

    if (startSeconds <= endSeconds)
    {
        // Plage normale (dans la m�me journ�e)
        isInTimeRange = (currentSeconds >= startSeconds && currentSeconds <= endSeconds);
    }
    else
    {
        // Plage traversant minuit
        isInTimeRange = (currentSeconds >= startSeconds || currentSeconds <= endSeconds);
    }

    // Gestion de l'intervalle
    float triggerValue = 0.0f;

    if (isActiveDay && isInTimeRange)
    {
        if (dayInterval == 0.0)
        {
            // Si l'intervalle est 0, d�clenche et reste � 1
            triggerValue = 1.0f;
        }
        else
        {
            // Bas� sur l'intervalle
            double elapsed = currentSeconds - startSeconds;
            if (elapsed < 0)
            {
                elapsed += 24 * 3600; // Correction si traversant minuit
            }

            if (std::fmod(elapsed, dayInterval) < 1.0 && currentSeconds != lastTriggerTime)
            {
                triggerValue = 1.0f;
                lastTriggerTime = currentSeconds;
            }
        }
    }

    // Remplir la sortie
    for (int i = 0; i < output->numSamples; ++i)
    {
        output->channels[0][i] = triggerValue;
    }
}

void SchedulerCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
    OP_NumericParameter np;

    // Param�tres globaux
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

    // Param�tres par jour
    const std::array<std::string, 7> days = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    for (const auto& day : days)
    {
        np.page = day.c_str();

        np.name = (day + "enable").c_str();
        np.label = "Enable";
        manager->appendToggle(np);

        np.name = (day + "starthour").c_str();
        np.label = "Start Hour";
        manager->appendInt(np);

        np.name = (day + "startminute").c_str();
        np.label = "Start Minute";
        manager->appendInt(np);

        np.name = (day + "startsecond").c_str();
        np.label = "Start Second";
        manager->appendInt(np);

        np.name = (day + "endhour").c_str();
        np.label = "End Hour";
        manager->appendInt(np);

        np.name = (day + "endminute").c_str();
        np.label = "End Minute";
        manager->appendInt(np);

        np.name = (day + "endsecond").c_str();
        np.label = "End Second";
        manager->appendInt(np);

        np.name = (day + "interval").c_str();
        np.label = "Interval";
        manager->appendFloat(np);
    }
}
