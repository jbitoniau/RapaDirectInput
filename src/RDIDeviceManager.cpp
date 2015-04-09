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
#include <algorithm>
#include "RDITime.h"
#include "RDICommon.h"
#include "RDIDeviceEnumerationTrigger.h"

/*
	Notes:
	- We probably need to support setting the Cooperation level, which requires a window handle (HWND)
	- Limit include windows.h and dinput to cpp if possible so it doesn't leak in code that uses the lib
*/
namespace RDI
{

IDirectInput8* DeviceManager::mDirectInput = NULL;

DeviceManager::DeviceManager( bool ignoreXInputControllers, bool consoleApplication /*, HWND windowHandle*/ )
	:	mIgnoreXInputControllers(ignoreXInputControllers),
		mEnumerationTrigger(NULL),
		//mWindowHandle(windowHandle),
		mDevices()
{
	createDirectInput();
	mEnumerationTrigger = new WindowsHookEnumerationTrigger( consoleApplication );
	//mEnumerationTrigger = new TimeBasedEnumerationTrigger(3000);
}

DeviceManager::~DeviceManager()
{
	delete mEnumerationTrigger;
	mEnumerationTrigger = NULL;
	deleteDirectInput();
}

void DeviceManager::update()
{
	// Update the list of connected Device (if needed)
	if ( mEnumerationTrigger->enumerationNeeded() )
		updateDeviceList();

	// Update the devices
	for ( std::size_t i=0; i<mDevices.size(); ++i )
	{
		Device* device = mDevices[i].second;
		device->update();
		
/*		Device* dev = static_cast<Device*>(device);
		printf("name:%s\n", dev->getDeviceInstance().getInstanceName() );//, dev->getDeviceInstance().getInstanceId()
		for ( std::size_t i=0; i<dev->getObjects().size(); ++i )
		{
			Object* obj = dev->getObjects()[i];
			printf("%s\n", obj->toString().c_str() );
		}*/	
	}
}

void DeviceManager::updateDeviceList()
{
	// Get the previous list of device identifiers
	DeviceIdentifiers previousDeviceIdentifiers;
	deviceListToDeviceIdentifiers( mDevices, previousDeviceIdentifiers );

	// Get an up to date version
	DeviceIdentifiers currentDeviceIdentifiers;
	std::pair<DeviceIdentifiers*, bool> enumDevicesCallbackUserData = std::make_pair( &currentDeviceIdentifiers, true );

	HRESULT hr = 0;
	// With a DI8DEVCLASS_ALL enumeration, the mouse and keyboard are ALWAYS returned as attached devices even if you unplug them from your computer
	hr = mDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL /*DI8DEVCLASS_ALL*/, enumDevicesCallback, &enumDevicesCallbackUserData, DIEDFL_ATTACHEDONLY ); 
	assert( SUCCEEDED(hr) );
	
	// Work out the differences between the two
	DeviceIdentifiers addedDeviceIdentifiers;
	calculateAddedDevicesList( previousDeviceIdentifiers, currentDeviceIdentifiers, addedDeviceIdentifiers );
	
	DeviceIdentifiers removedDeviceIdentifiers;
	calculateRemovedDevicesList( previousDeviceIdentifiers, currentDeviceIdentifiers, removedDeviceIdentifiers );

	// Create newly appeared devices
	for ( std::size_t i=0; i<addedDeviceIdentifiers.size(); ++i )
	{
		DeviceInstance identifier = addedDeviceIdentifiers[i];
		addDevice( identifier );
	}

	// Delete devices that are no longer connected
	for ( std::size_t i=0; i<removedDeviceIdentifiers.size(); ++i )
	{
		DeviceInstance identifier = removedDeviceIdentifiers[i];
		removeDevice( identifier );
	}
}	

void DeviceManager::addDevice( const DeviceInstance& identifier )
{
	// Create the device
	Device* device = new Device( /*mWindowHandle,*/ mDirectInput, identifier/*, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE*/ );
	
	// Add it to the list
	mDevices.push_back( std::make_pair( identifier, device ) );		

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onDeviceConnected( this, device );
}

void DeviceManager::removeDevice( const DeviceInstance& identifier )
{
	DeviceList::iterator itr;
	Device* device = NULL;
	for ( itr=mDevices.begin(); itr!=mDevices.end(); ++itr )
	{
		if ( identifier==(*itr).first )
		{
			device = (*itr).second;
	
			// Notify
			for ( Listeners::iterator itr2=mListeners.begin(); itr2!=mListeners.end(); ++itr2 )
				(*itr2)->onDeviceDisconnecting( this, device );

			// Remove the device from the list
			mDevices.erase(itr);
			break;
		}
	}

	// Delete the device
	delete device;
}

void DeviceManager::createDirectInput()
{
	if ( !mDirectInput )
	{
		HINSTANCE hInst = GetModuleHandle(0);
		HRESULT hr;
		hr = DirectInput8Create( hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mDirectInput, NULL );
		assert( SUCCEEDED(hr) );	
	}
	else 
	{
		mDirectInput->AddRef();
	}
}

void DeviceManager::deleteDirectInput()
{
	if( mDirectInput )
	{
		if ( mDirectInput->Release()==0 )
			mDirectInput = NULL;
	}
}

BOOL CALLBACK DeviceManager::enumDevicesCallback( LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef )
{
	assert( pvRef );
	std::pair<DeviceIdentifiers*, bool>* userData = static_cast<std::pair<DeviceIdentifiers*, bool>*>( pvRef );
	
	DeviceIdentifiers* deviceIdentifers = userData->first;
	assert( deviceIdentifers );
	
	bool ignoreXInputController = userData->second;

	if ( ignoreXInputController )
	{
		if ( Common::isXInputController( &lpddi->guidProduct ) )
			return DIENUM_CONTINUE;
	}

	DeviceInstance identifier( lpddi );
	DWORD deviceType = identifier.getDeviceType();
	if( deviceType == DI8DEVTYPE_JOYSTICK ||
		deviceType == DI8DEVTYPE_GAMEPAD ||
		deviceType == DI8DEVTYPE_1STPERSON ||
		deviceType == DI8DEVTYPE_DRIVING ||
		deviceType == DI8DEVTYPE_FLIGHT )			 
	//	deviceType == DI8DEVTYPE_MOUSE ||
	//	deviceType == DI8DEVTYPE_KEYBOARD )
	{
		deviceIdentifers->push_back( identifier );
	}
	return DIENUM_CONTINUE;
}

void DeviceManager::deviceListToDeviceIdentifiers( const DeviceList& list, DeviceIdentifiers& identifiers )
{
	identifiers.resize( list.size() );
	for ( std::size_t i=0; i<identifiers.size(); ++i )
		identifiers[i]=list[i].first;
}

void DeviceManager::calculateAddedDevicesList( const DeviceIdentifiers& previousDevices, const DeviceIdentifiers& currentDevices, DeviceIdentifiers& addedDevices )
{
	for ( std::size_t i=0; i<currentDevices.size(); ++i )
	{
		const DeviceInstance& currentDevice = currentDevices[i];
		DeviceIdentifiers::const_iterator itr = std::find( previousDevices.begin(), previousDevices.end(), currentDevice );
		if ( itr==previousDevices.end() )
			addedDevices.push_back( currentDevice );
	}
}

void DeviceManager::calculateRemovedDevicesList( const DeviceIdentifiers& previousDevices, const DeviceIdentifiers& currentDevices, DeviceIdentifiers& removedDevices )
{
	for ( std::size_t i=0; i<previousDevices.size(); ++i )
	{
		const DeviceInstance& previousDevice = previousDevices[i];
		DeviceIdentifiers::const_iterator itr = std::find( currentDevices.begin(), currentDevices.end(), previousDevice );
		if ( itr==currentDevices.end() )
			removedDevices.push_back( previousDevice );
	}
}

void DeviceManager::addListener( Listener* listener )
{
	assert(listener);
	mListeners.push_back(listener);
}

bool DeviceManager::removeListener( Listener* listener )
{
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

void DeviceManager::removeListeners()
{
	Listeners listeners = mListeners; // The copy is on purpose here
	for ( Listeners::iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
		removeListener( *itr );
}

Device* DeviceManager::getDeviceByName( const std::string& name ) const
{
	for ( std::size_t i=0; i<mDevices.size(); ++i )
	{
		Device* device = mDevices[i].second;
		const std::string& deviceName = device->getDeviceInstance().getInstanceName();
		if ( deviceName==name )
			return device;
	}
	return NULL;
}

}

