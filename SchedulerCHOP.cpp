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

    const std::array<std::string, 7> days = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    // Récupérer l'heure et la date courantes
    std::time_t currentTime = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);

    int currentDay = localTime.tm_wday; // Jour actuel (0 = Dimanche)
    double currentSeconds = localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec;

    bool isActiveDay = false;
    double startSeconds = 0.0, endSeconds = 0.0;

    if (globalMode)
    {
        // Mode global
        startSeconds = inputs->getParInt("Globalstarthour") * 3600 +
            inputs->getParInt("Globalstartminute") * 60 +
            inputs->getParInt("Globalstartsecond");

        endSeconds = inputs->getParInt("Globalendhour") * 3600 +
            inputs->getParInt("Globalendminute") * 60 +
            inputs->getParInt("Globalendsecond");

        isActiveDay = true; // Toujours actif en mode global
    }
    else
    {
        // Mode spécifique au jour
        const std::string currentDayName = days[(currentDay + 6) % 7];


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

    // Vérification de la plage horaire active
    bool isInTimeRange = (currentSeconds >= startSeconds && currentSeconds <= endSeconds);

    // Gestion de l'intervalle
    float triggerValue = 0.0f;

    if (isActiveDay && isInTimeRange)
    {
        if (interval == 0.0)
        {
            // Si l'intervalle est 0, déclenche et reste à 1
            triggerValue = 1.0f;
        }
        else
        {
            // Basé sur l'intervalle
            double elapsed = currentSeconds - startSeconds;
            if (std::fmod(elapsed, interval) < 1.0 && currentSeconds != lastTriggerTime)
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

    // Paramètres globaux
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

    // Paramètres par jour
    const std::array<std::string, 7> days = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

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
    }
}
