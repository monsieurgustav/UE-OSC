#include "OscPrivatePCH.h"
#include "OscSettings.h"
#include "OscDispatcher.h"

#if OSC_EDITOR_BUILD
#include "Editor.h"
#endif


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

#if OSC_EDITOR_BUILD
        _isPlayingInEditor = false;
        FEditorDelegates::BeginPIE.AddRaw(this, &FOscModule::OnBeginPIE);
        FEditorDelegates::EndPIE.AddRaw(this, &FOscModule::OnEndPIE);
#endif

        _dispatcher = UOscDispatcher::Get();
        
        HandleSettingsSaved();

        // register settings
#if OSC_ENGINE_VERSION < 40600
        auto settingsModule = ISettingsModule::Get();
#else
        auto settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
#endif
        if(settingsModule)
        {
#if OSC_ENGINE_VERSION < 40600
            FSettingsSectionDelegates settingsDelegates;
            settingsDelegates.ModifiedDelegate = FOnSettingsSectionModified::CreateRaw(this, &FOscModule::HandleSettingsSaved);

            auto settings = GetMutableDefault<UOscSettings>();
            settingsModule->RegisterSettings("Project", "Plugins", "OSC",
                LOCTEXT("OscSettingsName", "OSC"),
                LOCTEXT("OscSettingsDescription", "Configure the OSC plug-in."),
                settings,
                settingsDelegates);
#else
            auto settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "OSC",
                LOCTEXT("OscSettingsName", "OSC"),
                LOCTEXT("OscSettingsDescription", "Configure the OSC plug-in."),
                GetMutableDefault<UOscSettings>()
                );

            if (settingsSection.IsValid())
            {
                settingsSection->OnModified().BindRaw(this, &FOscModule::HandleSettingsSaved);
            }
#endif
        }
        else
        {
#if OSC_EDITOR_BUILD
            UE_LOG(LogOSC, Warning, TEXT("Settings changed registration failed"));
#endif
        }
        UE_LOG(LogOSC, Display, TEXT("Startup succeed"));
    }

    virtual void ShutdownModule( ) override
    {
        UE_LOG(LogOSC, Display, TEXT("Shutdown"));

        if(_dispatcher.IsValid())
        {
            auto settings = GetMutableDefault<UOscSettings>();
            settings->ClearKeyInputs(*_dispatcher);

            _dispatcher->Stop();
        }
    }

    bool HandleSettingsSaved()
    {
        if(!_dispatcher.IsValid())
        {
            UE_LOG(LogOSC, Warning, TEXT("Cannot update settings"));
            return false;
        }
        
        UE_LOG(LogOSC, Display, TEXT("Update settings"));

        auto settings = GetMutableDefault<UOscSettings>();

        // receive settings
#if OSC_EDITOR_BUILD
        if(_isPlayingInEditor)
        {
            Listen(settings);
        }
#else
        Listen(settings);
#endif

        // send settings
        settings->UpdateSendAddresses();

        // input settings
        settings->UpdateKeyInputs(*_dispatcher);

        return true;
    }

    void Listen(UOscSettings * settings)
    {
        FIPv4Address receiveAddress(0);
        uint32_t receivePort;
        if(UOscSettings::Parse(settings->ReceiveFrom, &receiveAddress, &receivePort))
        {
            _dispatcher->Listen(receiveAddress, receivePort);
        }
        else
        {
            UE_LOG(LogOSC, Error, TEXT("Fail to parse receive address: %s"), *settings->ReceiveFrom);
        }
    }

private:
#if OSC_EDITOR_BUILD
    void OnBeginPIE(bool isSimulating)
    {
        _isPlayingInEditor = true;

        check(_dispatcher.IsValid())
        auto settings = GetMutableDefault<UOscSettings>();
        Listen(settings);
    }

	void OnEndPIE(bool isSimulating)
    {
        _isPlayingInEditor = false;
        _dispatcher->Stop();
    }

    bool _isPlayingInEditor;
#endif

private:
    
    TWeakObjectPtr<UOscDispatcher> _dispatcher;
};


IMPLEMENT_MODULE(FOscModule, OSC);
DEFINE_LOG_CATEGORY(LogOSC);


#undef LOCTEXT_NAMESPACE
