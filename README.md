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

- For ASUS put ASUSMWI
- For HP put HPWMI
- For Dell put DELLWMI

If there are several PNP0C14, locate the one with a Method (_WED..., if there are several, try all.
Remove any other hotkey driver before trying it ! I.e. remove, rebuild kext cache and reboot.

## Features

- Features on ASUS:
	keystrokes to system:
	SOUND_UP
	SOUND_DOWN
	MUTE
	BRIGHTNESS_UP
	BRIGHTNESS_DOWN
	VIDMIRROR: (Fn + F8) toggle between extended desktop and clone mode
	
	Hardware control:
	BRIGHTNESS ON/OFF
	BLUETOOTH ON/OFF (Fn + F2)
	TRACKPAD ON/OFF (Fn + F9) : Only lights LED, disabling has to be coded in from PS2 driver, done by RehabMan.

- Features on Dell:
	keystrokes to system:
	SOUND_UP
	SOUND_DOWN
	MUTE
	BRIGHTNESS_UP
	BRIGHTNESS_DOWN
	VIDMIRROR
	
- Features on HP:
	keystrokes to system:
	BRIGHTNESS_UP
	BRIGHTNESS_DOWN