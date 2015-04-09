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
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <vector>
#include "RDIDevice.h"

namespace RDI
{

class DeviceEnumerationTrigger;
	
/*
	DeviceManager

	The DeviceManager object is responsible for maintaining a list of 
	the Devices that are currently connected to the computer.

	It is possible to register listeners the manager so client code can be 
	called whenever a Device is plugged in or removed.
*/
class DeviceManager
{
public:
	typedef std::vector<std::pair<DeviceInstance, Device*>> DeviceList;
	
	DeviceManager( bool ignoreXInputControllers, bool consoleApplication /*, HWND windowHandle*/ );
	virtual ~DeviceManager();

	virtual void				update();
	void						updateDeviceList();

	class Listener
	{
	public:
		virtual void onDeviceConnected( DeviceManager* /*deviceManager*/, Device* /*device*/ ) {}
		virtual void onDeviceDisconnecting( DeviceManager* /*deviceManager*/, Device* /*device*/ ) {}
	};

	void						addListener( Listener* listener );
	bool						removeListener( Listener* listener );
	void						removeListeners();
	
	const DeviceList&			getDevices() const		{ return mDevices; }
	Device*						getDeviceByName( const std::string& name ) const;

private:
	void						createDirectInput();
	void						deleteDirectInput();
	
	
	void						addDevice( const DeviceInstance& identifier );
	void						removeDevice( const DeviceInstance& identifier );

	static BOOL	CALLBACK		enumDevicesCallback( LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef );
	static void					deviceListToDeviceIdentifiers( const DeviceList& list, DeviceIdentifiers& identifiers );
	static void					calculateAddedDevicesList( const DeviceIdentifiers& previousDevices, const DeviceIdentifiers& currentDevices, DeviceIdentifiers& addedDevices );
	static void					calculateRemovedDevicesList( const DeviceIdentifiers& previousDevices, const DeviceIdentifiers& currentDevices, DeviceIdentifiers& addedRemoved );
	
	bool						mIgnoreXInputControllers;
	DeviceEnumerationTrigger*	mEnumerationTrigger;
	static IDirectInput8*		mDirectInput;
	//HWND						mWindowHandle;
	DeviceList					mDevices;

	// Listeners
	typedef						std::vector<Listener*> Listeners; 
	Listeners					mListeners;
};

}