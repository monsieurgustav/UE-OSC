UE4-OSC
=======

OSC plugin for Unreal Engine 4 Blueprints


Integration
===========

Install Visual Studio 2013 on Windows (Community edition works) or XCode on MaxOSX.

Create a "C++" project, or convert a "Blueprint" project to "C++" **********************

Create a "Plugins" directory in the project root (near the "Content" directory) and put the "OSC" folder of the plugin in it.
(e.g. "/MyProject/Plugins/OSC")

Run the "*.uproject" file: the plugin is compiled automatically.

Alternatively, right-clic the "*.uproject" to generate Visual Studio or XCode project files.


Usage
=====

 - Receiver:
 
Set the listening port in the plugin settings.

Add a OscReceiverComponent to your Blueprint class and bind the OnOscReceived custom event, or

Subclass the OscReceiverActor and bind the OnOscReceived custom event.

Use the PopFloat/Int/Bool/String functions, or standard array functions, to retrieve the OSC message parameters.


 - Sender:
 
Set the "send to" addresses in the plugin settings.

Use the PushFloat/Int/Bool/String functions, or standard array functions, to build the OSC message parameters.

Use the SendOsc function to send the OSC message


 - Inputs:
 
List the messages that should be UE4 inputs (like a game controller). A new input is created for each message.

Bind OSC inputs to UE4 action in the usual "Input" project settings.


Engine version
=====

Get the right branch for your engine version: "default" for the latest version, "before4.8" for 4.4, ..., 4.7, etc.

Optionally, set the OSC_ENGINE_VERSION value (in Source/OSC/Private/Osc.h) to compile for a specific engine version.
