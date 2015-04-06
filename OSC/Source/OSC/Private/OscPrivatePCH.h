#pragma once

#include "Osc.h"

#if OSC_ENGINE_VERSION < 40600
#include "Core.h"
#include "Settings.h"
#else
#include "Engine.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

#include "ModuleManager.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TaskGraphInterfaces.h"

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Core/Public/Misc/Base64.h"

#include <algorithm>
#include <memory>

DECLARE_LOG_CATEGORY_EXTERN(LogOSC, Log, All);
