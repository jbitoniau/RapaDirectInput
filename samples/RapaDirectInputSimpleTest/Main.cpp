/*
   The MIT License (MIT) (http://opensource.org/licenses/MIT)
   
   Copyright (c) 2015 Jacques Menuet
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "RDIDeviceManager.h"

#include <assert.h>
#include <map>
#include <sstream>
#include <string.h>

class DebugDeviceListener : public RDI::Device::Listener
{
public:
	virtual void onObjectChanged( RDI::Device* /*device*/, RDI::Object* object )
	{
		printf( "Object changed - %s\n", object->toString().c_str() );
	}
};

class DebugDeviceManagerListener : public RDI::DeviceManager::Listener
{
public:
	virtual void onDeviceConnected( RDI::DeviceManager* /*deviceManager*/, RDI::Device* device )
	{
		printf( "Device connected - %s\n", device->getDeviceInstance().getInstanceName().c_str() );
		
		// Create a listener for the Device, register it and remember it
		RDI::Device::Listener* listener = new DebugDeviceListener();
		mListeners.insert( std::make_pair( device, listener ) );
		device->addListener( listener );
	}

	virtual void onDeviceDisconnecting( RDI::DeviceManager* /*deviceManager*/, RDI::Device* device )
	{
		printf( "Device disconnecting - %s\n", device->getDeviceInstance().getInstanceName().c_str() );
		
		// Retrieve the listener we created for the Device, unregister it, delete it and forget it
		std::map<RDI::Device*, RDI::Device::Listener*>::iterator itr = mListeners.find( device );
		assert( itr!=mListeners.end() );
		RDI::Device::Listener* listener = (*itr).second;
		device->removeListener( listener );
		mListeners.erase( itr );
		delete listener;
	}

private:
	std::map<RDI::Device*, RDI::Device::Listener*> mListeners;	
};

std::string deviceToString( const RDI::Device* device )
{
	std::stringstream stream;
	stream << "Device:\n";
	stream << " InstanceName:'" << device->getDeviceInstance().getInstanceName() << "'\n";
	stream << " ProductName:'" << device->getDeviceInstance().getProductName() << "'\n";
	stream << " DeviceType:" << device->getDeviceInstance().getDeviceType() << "\n";
	stream << " DeviceSubType:" << device->getDeviceInstance().getDeviceSubType() << "\n";
	stream << " Objects:\n";
	const RDI::Objects& objects = device->getObjects();
	for ( std::size_t i=0; i<objects.size(); ++i )
	{
		const RDI::Object* object = objects[i];
		stream << "  " << object->toString() << "\n";
	}
	return stream.str();
}

std::string devicesToString( const RDI::DeviceManager& manager )
{
	std::stringstream stream;
	const RDI::DeviceManager::DeviceList& devices =  manager.getDevices();
	for ( std::size_t i=0; i<devices.size(); ++i )
	{
		RDI::Device* device = devices[i].second;
		stream << deviceToString( device );
	}
	return stream.str();
}

int main()
{
	RDI::DeviceManager manager(true, true);
	manager.addListener( new DebugDeviceManagerListener() );
	int i = 0;
	while ( i!=30 )
	{
		manager.update();
		Sleep(500);
		//printf("%s", devicesToString( manager ).c_str() );
		i++;
	}
	return 0;
}
