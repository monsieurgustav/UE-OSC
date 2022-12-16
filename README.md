UE-OSC
=======

OSC plugin for Unreal Engine 4 and Unreal Engine 5 to send and receive OSC messages with blueprints!

It uses oscpack, a C++ library for packing/unpacking OSC messages, by Ross Bencina. http://www.rossbencina.com/code/oscpack


# Migrating from UE4-OSC to UE-OSC

This plugin has been working for Unreal since 2014 (UE4.4!) and it continues today with UE5!

I decided to change its name from UE4-OSC to a more generic UE-OSC.

To migrate an existing project using UE4-OSC to UE-OSC, add the following in the Config/DefaultEngine.ini file.
```
[CoreRedirects]
+PackageRedirects=(OldName="/Script/UE4_OSC", NewName="/Script/UE_OSC")
```

# Tutorial

Dannington made a super tutorial!

Watch it to properly install the plugin and get some brilliant ideas on how to use it!

https://www.youtube.com/watch?v=GGGs-n-CKtY


Also, go to the UE forum dedicated thread:

https://forums.unrealengine.com/showthread.php?49627-Plugin-OSC-for-UE4


# Integration

Install Visual Studio 2019 on Windows (Community edition works) or XCode on MaxOSX.

Create a "C++" project, or convert a "Blueprint" project to "C++".

Create a "Plugins" directory in the project root (near the "Content" directory) and put the "OSC" folder of the plugin in it.
(e.g. "/MyProject/Plugins/OSC")

Run the "*.uproject" file: the plugin is compiled automatically.

Alternatively, right-clic the "*.uproject" to generate Visual Studio or XCode project files.


# Usage

### Receiver

Set the listening port as "Receive From" in the plugin settings. (Edit/Project Settings/Plugins/OSC)

Add a OscReceiverComponent to your Blueprint class and bind the OnOscReceived custom event, or
subclass the OscReceiverActor and bind the OnOscReceived custom event.

The OnOscReceived event gives: the OSC address, the OSC parameters and the IP of the sender.

The OSC parameters is an array of OSC elements. To read the content, either:
 - chain PopFloat/Int/Bool/String/Blob functions to get the first value, or
 - use the standard GET function and AsFloat/Int/Bool/String/Blob functions to cast the element to its value. This is slightly more efficient.

### Sender

Set the "Send Targets" addresses in the plugin settings. (Edit/Project Settings/Plugins/OSC)

You can also add new targets dynamically, using the "Add Send Osc Target" function.
It is especially useful for "ping"-like behavior.

Send a single message with "Send Osc" or a bundle of messages with "Send Osc Bundle".

The "Send Osc" function takes: the OSC address, the OSC parameters and the "target index" of the destination.

Build the OSC parameters for a message:
 - chain PushFloat/Int/Bool/String/Blob functions, or
 - fill an array of "Osc Data Elem" with the standard array functions and the FromFloat/Int/Bool/String/Blob functions.

The "target index" refers to an index in the "Send Targets" array in the settings,
or to the result of the "Add Send Osc Target" function.

### Inputs (experimental)

List the messages that should be UE inputs (like a game controller). A new input is created for each message.

Bind OSC inputs to UE action in the usual "Input" project settings.


# General information

### Troubleshooting

The plugin uses the standard UE logging system.

Check your logs (Window/Developers Tools/Output Log) before asking for help!

### String limitation

Historically, the plugin handles OSC string using "FName". Unfortunately, the length of
a "FName" is limited to 1024 characters. So this plugin does not handle long strings correctly.

If you need long strings, prefer using blobs.

### Blob

The OSC protocol supports buffers as "blob".

This plugin implements them as "Array<uint8>". It plays nicely with the "Extended Standard Library" plugin by "Low Entry".


# Engine version

Get the right branch for your engine version: "master" for the latest version, "before4.8" for 4.4, ..., 4.7, etc.

If the plugin does not work/build for the current version, feel free to create an issue or email me!
