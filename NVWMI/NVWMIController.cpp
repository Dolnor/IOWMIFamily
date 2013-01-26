/*
 *  NVWMIController.h
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


#include "NVWMIController.h"

#include "debug.h"



#define super IOWMIController

OSDefineMetaClassAndStructors(NVWMIController, IOWMIController)


bool       NVWMIController::init(OSDictionary *dictionary)
{
	return super::init(dictionary);
}

bool       NVWMIController::start(IOService *provider)
{
	return super::start(provider);
}

void       NVWMIController::stop(IOService *provider)
{
	super::stop(provider);
}

void       NVWMIController::free(void)
{
	super::free();
}


IOService * NVWMIController::probe(IOService *provider, SInt32 *score )
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
		
		if (name->isEqualTo("NVIF"))
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





void NVWMIController::enableEvent()
{
	DbgLog("%s: NVWMIController::enableEvent()\n", this->getName());
	
}


void NVWMIController::disableEvent()
{

}


void NVWMIController::handleMessage(int code)
{
	IOLog("%s: envent message code %x\n", this->getName(), uint(code));
}
