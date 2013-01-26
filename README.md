## Original Description:

IOWMIFamily driver coded by hotKoffy over at InsanelyMac.
http://www.insanelymac.com/forum/topic/248116-netbook-hot-key-driver-asus-dell-hp/

Last update: 18 sept 2011

I've wrote a driver for my Asus EEEPC 1201N since the existing one didn't worked after sleep.
It is base on the WMI device you can find in the DSDT (PNP0C14 + ASUSWMI),  so I think it can works 
with other asus product shipped with win7. Feel  free to try and report.

## Allowing the driver to work

To make sure this driver can attach you have to do some modification on WMI Device in your DSDT.
Generally you would have something similar to:

		Device (AMW0)
		{
			Name (_HID, EisaId ("PNP0C14"))
			Name (_UID, Zero)
			Method (_STA, 0, NotSerialized)
			{
            	If (LEqual (MSOS (), MSW7)) //check if OS Windows 7
            	{
                	Return (0x0F) // means device is enabled   
                }
                	Return (Zero) //means device is disabled   
            }
            ...

We don't need to be checking for OS to be Windows 7 as in our case OS is Darwin.
So you will need to remove this check if you have it in your DSDT, make _STA always Return (0x0F).
Secondly, we need to be changing the _UID

- For ASUS put "ASUSMWI" instead of Zero (or 0x00 - depends on the decompiler you've used)
- For HP put "HPWMI"
- For Dell put "DELLWMI"

If there are several PNP0C14, locate the one with a Method (_WED..., if there are several, try all.
Remove any other hotkey driver before trying it ! I.e. remove, rebuild kext cache and reboot.

## Features

- Features on ASUS:
	keystrokes to system:
	SOUND_UP (0x30), 
	SOUND_DOWN (0x31),
	MUTE (0x32),
	BRIGHTNESS_UP (0x11),
	BRIGHTNESS_DOWN (0x20),
	VIDMIRROR (0xCC) toggle between extended desktop and clone mode
	
	Hardware control:
	BRIGHTNESS ON/OFF (0xE9),
	BLUETOOTH ON/OFF (0x88),
	TRACKPAD ON/OFF (0x6B) only lights LED, disabling has to be coded in from PS2 driver, done by RehabMan.

- Features on Dell:
	keystrokes to system:
	SOUND_UP (0xe030),
	SOUND_DOWN (0xe02e),
	MUTE (0xe020),
	BRIGHTNESS_UP (0xe006),
	BRIGHTNESS_DOWN (0xe005),
	VIDMIRROR (0xe00b)
	
- Features on HP:
	keystrokes to system:
	BRIGHTNESS_UP (0x02),
	BRIGHTNESS_DOWN (0x03)

## Quick recap about newer Dell machines

EC queries usially call for WMI notifications, not all the keys on newer Dell machines utilize WMI, some are still tied to plain old PS/2 scancodes. For my particular laptop only brightness keys are tied to WMI. Here's an overview:

			//decrease brighntess
			Method (_Q80, 0, NotSerialized)
			{
            	....
            	
              	If (PWRS) //if AC charger is connected
                {
                	Store (Local0, BRGA) //store brightness level into register for AC brightness
                }
                Else //if running on batteries
                {
                	Store (Local0, BRGD) //store brightness level into register for DC brightness
                }
            	Store (0x03, ^^^^AMW0.INF0)
            	Store (Zero, ^^^^AMW0.INF1)
            	Store (0xE005, ^^^^AMW0.INF2) //0xE005 is a notification to decrease brightness
            	Store (Local0, ^^^^AMW0.INF3) //to a desired brightness level
            	If (LEqual (DMFG, Zero))
            	{
                	Notify (AMW0, 0xD0)
                }
          	} 
          	
          	//increase brighntess                     
			Method (_Q81, 0, NotSerialized)
			{                                        
            	....
            	
            	If (PWRS)
                {
                	Store (Local0, BRGA)
                }
                Else
                {
                	Store (Local0, BRGD)
                }
            	Store (0x03, ^^^^AMW0.INF0)
            	Store (Zero, ^^^^AMW0.INF1)
            	Store (0xE006, ^^^^AMW0.INF2) //0xE006 is a notification to increase brightness
            	Store (Local0, ^^^^AMW0.INF3) //to a desired brightness level
            	If (LEqual (DMFG, Zero))
            	{
                	Notify (AMW0, 0xD0)
                }
            }
            
Now let's see how WMI reacts to these notifications from EC queries:
                                           
            Method (_WED, 1, NotSerialized)
            {
                If (LEqual (Arg0, 0xD0)) //every EC Query notifies AMW0 with this argument
                {
                    Return (INFO) //so array INFO is being returned after each query
                }
                CLBY (INFO)
                Store (Arg0, INFO)
                Return (INFO)
            }
            
Array called INFO is being returned from _WED either way. As this driver analyzes the output from _WED's 0-index, in both cases it would get a value of 0x03, because it's written to AMW0.INF0 in both Q80 and Q81 queries. This 0x03 doesn't correspond to any predefined values in plugins, so nothing would happen.
            
            Name (INFO, Buffer (0x80) {})
            CreateWordField (INFO, Zero, INF0)
            CreateWordField (INFO, 0x02, INF1)
            CreateWordField (INFO, 0x04, INF2)             
            CreateWordField (INFO, 0x06, INF3)
            CreateWordField (INFO, 0x08, INF4)
            CreateWordField (INFO, 0x0A, INF5)
            CreateWordField (INFO, 0x0C, INF6)
            CreateWordField (INFO, 0x0E, INF7)
            
INF2 is the 4th and 5th byte of this INFO array returned by _WED, this one holds the code we are interested to see from the driver to be able to perform certaina ction. So our "scancode" that kext expects to see results has to be coded in like:
number = OSDynamicCast(OSNumber, array->getObject(4))           
