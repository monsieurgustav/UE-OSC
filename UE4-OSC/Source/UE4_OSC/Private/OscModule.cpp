#include "OscSettings.h"
#include "Receive/OscDispatcher.h"

#if OSC_EDITOR_BUILD
#include "Editor.h"
#endif


#define LOCTEXT_NAMESPACE "FOscModule"


class FOscModule : public IModuleInterface
{
public:
    virtual void StartupModule( ) override
    {
        if(!FModuleManager::Get().LoadModule(TEXT("Networking")))
        {
            UE_LOG(LogUE4_OSC, Error, TEXT("Required module Networking failed to load"));
            return;
        }

#if OSC_EDITOR_BUILD
        _mustListen = !GIsEditor;  // must not listen now if IsEditor (listen when PIE), else (Standalone Game) listen now
        FEditorDelegates::BeginPIE.AddRaw(this, &FOscModule::OnBeginPIE);
        FEditorDelegates::EndPIE.AddRaw(this, &FOscModule::OnEndPIE);
#endif

        _dispatcher = UOscDispatcher::Get();
        
        HandleSettingsSaved();

        // register settings
        auto settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
        if(settingsModule)
        {
            auto settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "OSC",
                LOCTEXT("OscSettingsName", "OSC"),
                LOCTEXT("OscSettingsDescription", "Configure the OSC plug-in."),
                GetMutableDefault<UOscSettings>()
                );

            if (settingsSection.IsValid())
            {
                settingsSection->OnModified().BindRaw(this, &FOscModule::HandleSettingsSaved);
            }
        }
        else
        {
#if OSC_EDITOR_BUILD
            UE_LOG(LogUE4_OSC, Warning, TEXT("Settings changed registration failed"));
#endif
        }
        UE_LOG(LogUE4_OSC, Display, TEXT("Startup succeed"));
    }

    virtual void ShutdownModule( ) override
    {
        UE_LOG(LogUE4_OSC, Display, TEXT("Shutdown"));

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
            UE_LOG(LogUE4_OSC, Warning, TEXT("Cannot update settings"));
            return false;
        }
        
        UE_LOG(LogUE4_OSC, Display, TEXT("Update settings"));

        auto settings = GetMutableDefault<UOscSettings>();

        // receive settings
#if OSC_EDITOR_BUILD
        if(_mustListen)
        {
            Listen(settings);
        }
#else
        Listen(settings);
#endif

        // send settings
        settings->InitSendTargets();

        // input settings
        settings->UpdateKeyInputs(*_dispatcher);

        return true;
    }

    void Listen(UOscSettings * settings)
    {
        FIPv4Address receiveAddress(0);
        uint32_t receivePort;
        FIPv4Address receiveMulticastAddress(0);
        if(UOscSettings::Parse(settings->ReceiveFrom, &receiveAddress, &receivePort, &receiveMulticastAddress, UOscSettings::ParseOption::OptionalAddress))
        {
            _dispatcher->Listen(receiveAddress, receivePort, receiveMulticastAddress, settings->MulticastLoopback);
        }
        else
        {
            UE_LOG(LogUE4_OSC, Error, TEXT("Fail to parse receive address: %s"), *settings->ReceiveFrom);
        }
    }

private:
#if OSC_EDITOR_BUILD
    void OnBeginPIE(bool isSimulating)
    {
        _mustListen = true;

        check(_dispatcher.IsValid())
        auto settings = GetMutableDefault<UOscSettings>();
        Listen(settings);
    }

	void OnEndPIE(bool isSimulating)
    {
        _mustListen = false;
        _dispatcher->Stop();
    }

    bool _mustListen;
#endif

private:
    
    TWeakObjectPtr<UOscDispatcher> _dispatcher;
};


IMPLEMENT_MODULE(FOscModule, UE4_OSC);
DEFINE_LOG_CATEGORY(LogUE4_OSC);


#undef LOCTEXT_NAMESPACE
