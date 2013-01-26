/*
 *  HPWMIController.cpp
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

#include "HPWMIController.h"
#include "debug.h"


#define HP_WMI_EVENT_GUID	"95F24279-4D7B-4334-9387-ACCDC67EF61C"


#define super IOWMIController

OSDefineMetaClassAndStructors(HPWMIController, IOWMIController)


bool       HPWMIController::init(OSDictionary *dictionary)
{
	return super::init(dictionary);
}

bool       HPWMIController::start(IOService *provider)
{
	return super::start(provider);
}

void       HPWMIController::stop(IOService *provider)
{
	super::stop(provider);
}

void       HPWMIController::free(void)
{
	super::free();
}


IOService * HPWMIController::probe(IOService *provider, SInt32 *score )
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
		
		if (name->isEqualTo("HPWMI"))
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


const wmiKeyMap HPWMIController::keyMap[] = {
	{0x02, NX_KEYTYPE_BRIGHTNESS_UP, "NX_KEYTYPE_BRIGHTNESS_UP"},
	{0x03, NX_KEYTYPE_BRIGHTNESS_DOWN, "NX_KEYTYPE_BRIGHTNESS_DOWN"},
	{0,0xFF,NULL}
};


void HPWMIController::enableEvent()
{
	DbgLog("%s: HPWMIController::enableEvent()\n", this->getName());
	
	if (super::setEvent(HP_WMI_EVENT_GUID, true) != kIOReturnSuccess)
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


void HPWMIController::disableEvent()
{
	if (_keyboardDevice)
	{
		super::setEvent(HP_WMI_EVENT_GUID, false);
		_keyboardDevice->release();
	}
}


void HPWMIController::handleMessage(int code)
{
	
    DbgLog("%s: HPWMIController::handleMessage(%d)\n", this->getName(), code);
    
	switch (code) {
			
		default:
            
            super::handleMessage(code);
            break;
	}	
}

