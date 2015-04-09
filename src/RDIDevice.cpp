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
#include "RDIDevice.h"

#include <assert.h>
#include <algorithm>

/*
	Notes:
	- A Device that gets disconnected can be maintained alive. If it gets physically connected again, 
	  the Device object will resume working (there's a reacquisition process in place in the update() method
*/

namespace RDI
{

Device::Device( /*HWND windowHandle,*/ IDirectInput8* directInput, const DeviceInstance& identifier/*, DWORD coopSettings*/ )
	: //mWindowHandle(windowHandle),
	  mDirectInput(directInput),
	  mDeviceInstance(identifier)
	  //mCoopSettings(coopSettings)
{
	bool ret = initialize();
	assert(ret);
	ret = enumerateObjects();
	assert(ret);
}

Device::~Device()
{
	deleteObjects();

	assert( mInputDevice );
	mInputDevice->Unacquire();
	mInputDevice->Release();
	mInputDevice = NULL;
}

void Device::update()
{
	DIDEVICEOBJECTDATA dataEntries[mDataBufferSize];
	DWORD numDataEntries = mDataBufferSize;

	// Try to get the data
	bool ret = getDeviceData( mInputDevice, dataEntries, &numDataEntries );
	if ( !ret )
	{
		// Getting data from the device can fail if for example the device
		// has been physically removed and we haven't yet update the device list 
		// In the meantime, that means that we're returning the last valid state
		return;
	}

	for( unsigned int i=0; i<numDataEntries; ++i )					
	{															
		// Find the Object involved in the event and update it
		const DIDEVICEOBJECTDATA& entry = dataEntries[i];
		
		// The 0xFFFFFFFF value indicates no user data, therefore its a 
		// DirectInput object we're not considering
		if ( entry.uAppData!=0xFFFFFFFF )			
		{
			Object* object = reinterpret_cast<Object*>( entry.uAppData );
			object->updateFrom( entry );
		}
	}
}

bool Device::initialize()
{
	IDirectInput8* directInput = getDirectInput();
	const DeviceInstance& deviceIndentifier = getDeviceInstance();
	
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = mDataBufferSize;

	HRESULT hr = 0;
	GUID deviceGuid = deviceIndentifier.getGuidInstance();
	hr = directInput->CreateDevice( deviceGuid, &mInputDevice, NULL );
	assert( SUCCEEDED(hr) );

	hr = mInputDevice->SetDataFormat( &c_dfDIJoystick2 );
	assert( SUCCEEDED(hr) );		// Note: if we want to return false, we need to delete the device

/*	HWND hWnd = HWND( getWindowHandle() );
	hr = mInputDevice->SetCooperativeLevel(hWnd, getCoopSettings());
	assert( SUCCEEDED(hr) );		// Note: same here
*/
	hr = mInputDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	assert( SUCCEEDED(hr) );		// Note: same here

	return true;
}

bool Device::enumerateObjects()
{
	assert( mObjects.empty() );

	// Enumerate the ObjectInstances making up this device
	ObjectInstances objectInstances;
	HRESULT hr = 0;
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectinputdevice8.idirectinputdevice8.enumobjects(v=vs.85).aspx
	hr = mInputDevice->EnumObjects(enumObjectsCallback, &objectInstances, DIDFT_ALL);
	assert( SUCCEEDED(hr) );
	
	// Create Object using this ObjectInstances and add them to this Device
	for ( std::size_t i=0; i<objectInstances.size(); ++i )
	{
		const ObjectInstance& objectInstance = objectInstances[i];
		Object* object = Object::createObject( objectInstance, this );
		if ( object )
		{
			// Set the user data of the DirectInput object to the Object that represents it
			// so we can retrieve it rapidly in the update method
			if ( object->setUserData( reinterpret_cast<UINT_PTR>(object) ) )
			{
				// Add the Object to our list
				mObjects.push_back( object );
			}
			else
			{
				delete object;
			}
		}
		else
		{
			// Type of Object not supported
			//assert( false );
		}
	}
	return true;
}

BOOL CALLBACK Device::enumObjectsCallback( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{
	ObjectInstances* objectInstances = static_cast<ObjectInstances*>( pvRef );
	assert(objectInstances);
	ObjectInstance identifier( lpddoi );
	objectInstances->push_back( identifier );
	return DIENUM_CONTINUE;
}

void Device::addObject( Object* object )
{
	assert(object);
	mObjects.push_back(object);
}

void Device::deleteObjects()
{
	for ( std::size_t i=0; i<mObjects.size(); ++i )
		delete mObjects[i];
	mObjects.clear();
}

bool Device::getDeviceData( IDirectInputDevice8* device, LPDIDEVICEOBJECTDATA dataEntries, LPDWORD numDataEntries )
{
	// This method can detect unplugged devices with the HRESULT code DIERR_UNPLUGGED.
	// In foreground cooperative mode, this is only detectable if the window has the focus.

	// This method is heavily inspired from OIS code
	bool result = false;
	HRESULT hr;
	hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), dataEntries, numDataEntries, 0 );

	//std::string str = "After GetDeviceData " + HRESULTToString( hr ) + "\n";
	//OutputDebugString( str.c_str() );
	if ( hr==DIERR_NOTACQUIRED || hr==DIERR_INPUTLOST )
	{
		hr = device->Acquire();
		//str = "After Acquire " + HRESULTToString( hr ) + "\n";
		//OutputDebugString( str.c_str() );
		if ( hr==DI_OK || hr==S_FALSE )
		{
			// Device got acquired (S_FALSE simply means it was already acquired) 
			hr = device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), dataEntries, numDataEntries, 0 );

			//str = "After second GetDeviceData " + HRESULTToString( hr ) + "\n";
			//OutputDebugString( str.c_str() );
			result = ( hr==DI_OK );
		}
		else
		{
			// Failed to acquire, better luck next update
			result = false;
		}
	}
	else if ( hr==DI_BUFFEROVERFLOW )
	{
		// The call to GetDeviceData() returned a full buffer, so we return it.
		// We certainly have lost some events but there's nothing we can do
		result = true;
	}
	else if ( hr==DI_OK )
	{
		// Everything went fine
		result = true;
	}
	else
	{
		// Something went wrong.. What to do?
		result = true;
	}
	
	//if ( result )
	//{
	//	std::stringstream stream;
	//	stream << "Num entries: " << *_numDataEntries << "\n";
	//	OutputDebugString( stream.str().c_str() );
	//}
	return result;
}

// Called by contained Objects to notify that they've changed
void Device::notifyObjectChanged( Object* object )
{
	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onObjectChanged( this, object );
}

void Device::addListener( Listener* listener )
{
	assert(listener);
	mListeners.push_back(listener);
}

bool Device::removeListener( Listener* listener )
{
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

void Device::removeListeners()
{
	Listeners listeners = mListeners; // The copy is on purpose here
	for ( Listeners::iterator itr=listeners.begin(); itr!=listeners.end(); ++itr )
		removeListener( *itr );
}

}
