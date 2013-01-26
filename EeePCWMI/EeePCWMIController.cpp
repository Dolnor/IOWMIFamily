/*
 *  EeePCWMIController.cpp
 *  IOWMIFamily
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <IOKit/hidsystem/ev_keymap.h>

#include "EeePCWMIController.h"
#include "debug.h"


#define EEEPC_WMI_EVENT_GUID	"ABBC0F72-8EA1-11D1-00A0-C90629100000"
#define EEEPC_WMI_MGMT_GUID		"97845ED0-4E6D-11DE-8A39-0800200C9A66"

#define EEEPC_WMI_METHODID_SPEC 0x43455053
#define EEEPC_WMI_METHODID_DEVP 0x50564544
#define EEEPC_WMI_METHODID_DEVS	0x53564544
#define EEEPC_WMI_METHODID_DSTS	0x53544344
#define EEEPC_WMI_METHODID_CFVS	0x53564643

#define EEEPC_WMI_DEVID_BACKLIGHT	0x00050011 
#define EEEPC_WMI_DEVID_BACKLIGHT2	0x00050012
#define EEEPC_WMI_DEVID_BLUETOOTH   0x00010013
#define EEEPC_WMI_DEVID_WIRELESS	0x00010011
#define EEEPC_WMI_DEVID_TRACKPAD	0x00100011

#define super IOWMIController

OSDefineMetaClassAndStructors(EeePCWMIController, IOWMIController)


bool       EeePCWMIController::init(OSDictionary *dictionary)
{
	return super::init(dictionary);
}

bool       EeePCWMIController::start(IOService *provider)
{
	return super::start(provider);
}

void       EeePCWMIController::stop(IOService *provider)
{
	super::stop(provider);
}

void       EeePCWMIController::free(void)
{
	super::free();
}


IOService * EeePCWMIController::probe(IOService *provider, SInt32 *score )
{
	IOService * ret = NULL;
	OSObject * obj;
	OSString * name;
	IOACPIPlatformDevice *dev;
	do
	{
		DbgLog("%s: Probe()\n", this->getName());
		
		if (!super::probe(provider, score))
			continue;
		
		
		dev = OSDynamicCast(IOACPIPlatformDevice, provider);
		if (NULL == dev)
			continue;
		
		dev->evaluateObject("_UID", &obj);
		
		name = OSDynamicCast(OSString, obj);
		if (NULL == name)
			continue;
		
		DbgLog("%s: Probe(%s)\n", this->getName(), name->getCStringNoCopy());
		
		if (name->isEqualTo("ASUSWMI"))
		{
			DbgLog("%s: Probe(OK)\n", this->getName());
			*score +=20;
			ret = this;
		}
		name->release();
		
    }
    while (false);
	
    return (ret);
}


const wmiKeyMap EeePCWMIController::keyMap[] = {
	{0x30, NX_KEYTYPE_SOUND_UP, "NX_KEYTYPE_SOUND_UP"},	
	{0x31, NX_KEYTYPE_SOUND_DOWN, "NX_KEYTYPE_SOUND_DOWN"},
	{0x32, NX_KEYTYPE_MUTE, "NX_KEYTYPE_MUTE"},
	{0x11, NX_KEYTYPE_BRIGHTNESS_UP, "NX_KEYTYPE_BRIGHTNESS_UP"},
	{0x20, NX_KEYTYPE_BRIGHTNESS_DOWN, "NX_KEYTYPE_BRIGHTNESS_DOWN"},
	{0xCC, NX_KEYTYPE_VIDMIRROR, "NX_KEYTYPE_VIDMIRROR"},
	{0,0xFF,NULL}
};



void EeePCWMIController::enableEvent()
{
	DbgLog("%s: EeePCWMIController::enableEvent()\n", this->getName());
	
	if (super::setEvent(EEEPC_WMI_EVENT_GUID, true) != kIOReturnSuccess)
		IOLog("Unable to enable events!!!\n");
    // ignore setEvent by  lvs1974
    
	//else
	//{
		super::_keyboardDevice = new WMIHIKeyboardDevice;
		
		if ( !_keyboardDevice               ||
			!_keyboardDevice->init()       ||
			!_keyboardDevice->attach(this) )  // goto fail;
		{
			_keyboardDevice->release();
			IOLog("%s: Error creating keyboardDevice\n", this->getName());
		}
		else
		{
			_keyboardDevice->setKeyMap(keyMap);
			_keyboardDevice->registerService();
		}
	//}
	
}


void EeePCWMIController::disableEvent()
{
	if (_keyboardDevice)
	{
		super::setEvent(EEEPC_WMI_EVENT_GUID, false);
		_keyboardDevice->release();
	}
}


void EeePCWMIController::handleMessage(int code)
{
	const UInt8 NOTIFY_BRNUP_MIN = 0x11;
	const UInt8 NOTIFY_BRNUP_MAX = 0x1f;
	const UInt8 NOTIFY_BRNDOWN_MIN = 0x20;
	const UInt8 NOTIFY_BRNDOWN_MAX = 0x2e;
	
	switch (code) {
		case 0x57: //AC disconnected
		case 0x58: //AC connected
			//ignore silently
			break;
		case 0x88: //RFkill
			rfkillEvent();
			break;

		case 0xE9: //Back Light ON/OFF
			backLightEvent();
			break;
			
		case 0x6B: //Track Pad ON/OFF
			trackPadEvent();
			break;
			
		case 0x5C: //Fn + space bar
			//test();
			break;
			
		default:
		if (code >= NOTIFY_BRNUP_MIN && code <= NOTIFY_BRNUP_MAX)
			code = NOTIFY_BRNUP_MIN;
		else if (code >= NOTIFY_BRNDOWN_MIN &&
				 code <= NOTIFY_BRNDOWN_MAX)
			code = NOTIFY_BRNDOWN_MIN;
		
		super::handleMessage(code);
		break;
	}	
}

void EeePCWMIController::rfkillEvent()
{
	UInt32 status = -1;
	getDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DSTS, EEEPC_WMI_DEVID_BLUETOOTH, &status);
	status = (status & 0x0001) xor 1;
	setDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DEVS, EEEPC_WMI_DEVID_BLUETOOTH, &status);
}


void EeePCWMIController::backLightEvent()
{
	UInt32 status = -1;
	getDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DSTS, EEEPC_WMI_DEVID_BACKLIGHT, &status);
	status = (status & 0x0001) xor 1;
	setDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DEVS, EEEPC_WMI_DEVID_BACKLIGHT, &status);
}

//trackpad led ON/OFF only !
void EeePCWMIController::trackPadEvent()
{
	UInt32 status = -1;
	getDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DSTS, EEEPC_WMI_DEVID_TRACKPAD, &status);
	status = (status & 0x0001) xor 1;
	setDeviceStatus(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_DEVS, EEEPC_WMI_DEVID_TRACKPAD, &status);
}


void EeePCWMIController::test()
{
	DbgLog("%s: ------ Entering Test event handler ------ \n", this->getName());
/*
	Super hybrid engine performance states:
	0 -> Super Performance Mode
	1 -> High Performance Mode
	2 -> Power Saving Mode
*/	
	static UInt32 status = 0;
	UInt32 temp = status;
	DbgLog("%s: setting cpufv to %d\n", this->getName(), (int)temp);
	//setDevice(EEEPC_WMI_MGMT_GUID, EEEPC_WMI_METHODID_CFVS, &temp);

	status ++;
	if (status > 2)
		status = 0;
	
}
