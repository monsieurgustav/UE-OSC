UE4-OSC
=======

OSC plugin for Unreal Engine 4 Blueprints


Integration
===========

Put the "OSC" folder in a "Plugins" folder in the project root.
e.g. "/MyProject/Plugins/OSC"


Usage
=====

 - Receiver:
Set the listening port in the plugin settings.
Add a OscReceiverComponent to your Blueprint class.
Bind the OnOscReceived custom event.
Use the PopFloat/Int/Bool/String functions to retrieve the OSC message parameters.

 - Sender:
Set the "send to" addresses in the plugin settings.
Use the PushFloat/Int/Bool/String functions to build the OSC message parameters.
Use the SendOsc function to send the OSC message
