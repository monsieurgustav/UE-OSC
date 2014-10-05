#include "OscPrivatePCH.h"
#include "OscDispatcher.h"


#define LOCTEXT_NAMESPACE "FOscModule"


class FOscModule : public IModuleInterface
{
public:
    virtual void StartupModule( ) override
    {
        if(!FModuleManager::Get().LoadModule(TEXT("Networking")).IsValid())
        {
            UE_LOG(LogOSC, Error, TEXT("Required module Networking failed to load"));
            return;
        }

        auto settings = GetMutableDefault<UOscSettings>();

        _dispatcher = UOscDispatcher::Get();
        _dispatcher->Listen(settings->ReceivePort);
        settings->UpdateSendAddresses();

        // register settings
        ISettingsModule* settingsModule = ISettingsModule::Get();
        if(settingsModule)
        {
            FSettingsSectionDelegates settingsDelegates;
            settingsDelegates.ModifiedDelegate = FOnSettingsSectionModified::CreateRaw(this, &FOscModule::HandleSettingsSaved);

            settingsModule->RegisterSettings("Project", "Plugins", "OSC",
                LOCTEXT("OscSettingsName", "OSC"),
                LOCTEXT("OscSettingsDescription", "Configure the OSC plug-in."),
                settings,
                settingsDelegates
            );
        }
        else
        {
            UE_LOG(LogOSC, Warning, TEXT("Settings changed registration failed"));
        }
        UE_LOG(LogOSC, Display, TEXT("Startup succeed"));
    }

    virtual void ShutdownModule( ) override
    {
        UE_LOG(LogOSC, Display, TEXT("Shutdown"));
        if(_dispatcher.IsValid())
        {
            _dispatcher->Stop();
        }
    }

    bool HandleSettingsSaved()
    {
        UE_LOG(LogOSC, Display, TEXT("Update settings"));
        UOscSettings* settings = GetMutableDefault<UOscSettings>();
        if(_dispatcher.IsValid())
        {
            _dispatcher->Listen(settings->ReceivePort);
        }
        settings->UpdateSendAddresses();
        return true;
    }

private:
    
    TWeakObjectPtr<UOscDispatcher> _dispatcher;
};


IMPLEMENT_MODULE(FOscModule, OSC);
DEFINE_LOG_CATEGORY(LogOSC);


#undef LOCTEXT_NAMESPACE
