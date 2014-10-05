#pragma once

#include "Osc.h"

#include "Core.h"
#include "ModuleManager.h"
#include "Settings.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TaskGraphInterfaces.h"

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Networking/Public/Networking.h"

#include "OscSettings.h"

#include <algorithm>
#include <memory>

DECLARE_LOG_CATEGORY_EXTERN(LogOSC, Log, All);
