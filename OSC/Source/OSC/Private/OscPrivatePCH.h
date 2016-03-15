#pragma once

#include "Osc.h"

#include "Engine.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "ModuleManager.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TaskGraphInterfaces.h"

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Core/Public/Misc/Base64.h"

#include <algorithm>
#include <memory>
#include <tuple>

DECLARE_LOG_CATEGORY_EXTERN(LogOSC, Log, All);
