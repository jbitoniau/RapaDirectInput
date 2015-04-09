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

namespace RDI
{

/*
	The DeviceInstance is a wrapper around the DirectInput DIDEVICEINSTANCE structure
	See http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.dideviceinstance(v=vs.85).aspx
	It is basically a descriptor/identifier of a DirectInput device 
*/
struct DeviceInstance
{
public:
	DeviceInstance();
	DeviceInstance( LPCDIDEVICEINSTANCE deviceObjectInstance );
	DeviceInstance( const DeviceInstance& other );

	bool operator==( const DeviceInstance& other ) const;
	DeviceInstance& operator=( const DeviceInstance& other );

	const GUID&			getGuidInstance() const		{ return mDeviceInstance.guidInstance; }

	const GUID&			getGuidProduct() const		{ return mDeviceInstance.guidProduct; }

	// This is the raw information about the device type. It combines the type and subtype
	// Method below allow easier acces to one or the other
	DWORD				getDwDevType() const		{ return mDeviceInstance.dwDevType; }
	DWORD				getDeviceType() const		{ return GET_DIDEVICE_TYPE( getDwDevType() ); }
	DWORD				getDeviceSubType() const	{ return GET_DIDEVICE_SUBTYPE( getDwDevType() ); }

	std::string			getInstanceName() const;
	std::string			getProductName() const;

	const GUID&			getFFDriver() const			{ return mDeviceInstance.guidFFDriver; }
	
	WORD				getUsagePage() const		{ return mDeviceInstance.wUsagePage; }
	WORD				getUsage() const			{ return mDeviceInstance.wUsage; }

private:
	const TCHAR*		getRawInstanceName() const	{ return mDeviceInstance.tszInstanceName; }
	const TCHAR*		getRawProductName() const	{ return mDeviceInstance.tszProductName; }
	
	DIDEVICEINSTANCE	mDeviceInstance;
	mutable std::string	mInstanceName;		// Cached UTF-8 version of instance name
	mutable std::string	mProductName;		// Same of product name
};

typedef std::vector<DeviceInstance> DeviceIdentifiers;

}

