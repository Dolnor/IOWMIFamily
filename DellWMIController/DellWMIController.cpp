/*
 *  DellWMIController.cpp
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

#include "DellWMIController.h"
#include "debug.h"


#define Dell_WMI_EVENT_GUID	"9DBB5994-A997-11DA-B012-B622A1EF5492"


#define super IOWMIController

OSDefineMetaClassAndStructors(DellWMIController, IOWMIController)


bool       DellWMIController::init(OSDictionary *dictionary)
{
	return super::init(dictionary);
}

bool       DellWMIController::start(IOService *provider)
{
	return super::start(provider);
}

void       DellWMIController::stop(IOService *provider)
{
	super::stop(provider);
}

void       DellWMIController::free(void)
{
	super::free();
}


IOService * DellWMIController::probe(IOService *provider, SInt32 *score )
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
		
		if (name->isEqualTo("DELLWMI"))
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

//assume from now on that extended part from the code is not important, trim away 0xE0 to match other plugins
const wmiKeyMap DellWMIController::keyMap[] = {
	{0x30, NX_KEYTYPE_SOUND_UP, "NX_KEYTYPE_SOUND_UP"},
	{0x2e, NX_KEYTYPE_SOUND_DOWN, "NX_KEYTYPE_SOUND_DOWN"},
	{0x20, NX_KEYTYPE_MUTE, "NX_KEYTYPE_MUTE"},
	{0x06, NX_KEYTYPE_BRIGHTNESS_UP, "NX_KEYTYPE_BRIGHTNESS_UP"},
	{0x05, NX_KEYTYPE_BRIGHTNESS_DOWN, "NX_KEYTYPE_BRIGHTNESS_DOWN"},
	{0x0b, NX_KEYTYPE_VIDMIRROR, "NX_KEYTYPE_VIDMIRROR"},
    {0x09, NX_KEYTYPE_EJECT, "NX_KEYTYPE_EJECT"},
    //{0xe03a, NX_KEYTYPE_CAPS_LOCK, "NX_KEYTYPE_CAPS_LOCK"},
    //{0xe045, NX_MODIFIERKEY_NUMERICPAD, "NX_MODIFIERKEY_NUMERICPAD"},
	{0,0xFF,NULL}
};


void DellWMIController::enableEvent()
{
	DbgLog("%s: DellWMIController::enableEvent()\n", this->getName());
	
	if (super::setEvent(Dell_WMI_EVENT_GUID, true) != kIOReturnSuccess)
		IOLog("Unable to enable events!!!\n");
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


void DellWMIController::disableEvent()
{
	if (_keyboardDevice)
	{
		super::setEvent(Dell_WMI_EVENT_GUID, false);
		_keyboardDevice->release();
	}
}


void DellWMIController::handleMessage(int code)
{	
    DbgLog("%s: DellWMIController::handleMessage(%d)\n", this->getName(), code);
    
	switch (code) {
			
		default:
            
            super::handleMessage(code);
            break;
	}	
}
