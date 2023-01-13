#pragma once

#include "CoreMinimal.h"

// Target engine version. Used to enable compatibility code.
#define OSC_ENGINE_VERSION 40800

#ifndef OSC_EDITOR_BUILD
#error OSC_EDITOR_BUILD must be set by the OSC.Build.cs
#endif

DECLARE_LOG_CATEGORY_EXTERN(LogUE_OSC, Log, All);
