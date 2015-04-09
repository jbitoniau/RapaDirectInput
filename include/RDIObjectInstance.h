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

class Device;

/*
	The ObjectInstance class is a wrapper around the DirectInput DIDEVICEOBJECTINSTANCE structure
	See http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.dideviceobjectinstance(v=vs.85).aspx
	It is basically a descriptor/identifier of an DirectInput object within a device
*/
class ObjectInstance
{
public:
	ObjectInstance();
	ObjectInstance( LPCDIDEVICEOBJECTINSTANCE deviceObjectInstance );
	ObjectInstance( const ObjectInstance& other );
	ObjectInstance& operator=( const ObjectInstance& other );
	
	// Returns a GUID_xxxx value, like GUID_XAxis, GUID_Button, etc.. This is an optional piece of information.
	const GUID&		getGuidType() const				{ return mDeviceObjectInstance.guidType; }	
	//const char*		getGuidTypeString() const;

	DWORD			getDwOfs() const				{ return mDeviceObjectInstance.dwOfs; }   

	// This is the raw piece of information about the type of the instance
	// The methods below help extract the relevant bits
	DWORD			getDwType() const				{ return mDeviceObjectInstance.dwType; }

	// Returns the "index" of this type of DirectInput object in its parent device.
	// For example, it can return 2 for the third button of the device. It can also return 2 for the third axis of the device
	// Apparently, there might be gaps in the indices (this could return 5 with no 4 for example)
	DWORD			getObjectIndex() const			{ return DIDFT_GETINSTANCE( getDwType() ); }

	// Returns the type of object. The value returned is a bitfield, for some types of object 
	// several bits can be on. For example: an axis can be reported as a combination of 
	// DIDFT_RELAXIS, DIDFT_ABSAXIS and	DIDFT_AXIS (not sure what a relative AND absolute axis means)
	// Some of the DIDFT_XXX values are masks that help check what one's interested in. These
	// have been exposed as separate methods here below.
	DWORD			getObjectType() const			{ return DIDFT_GETTYPE( getDwType() ); }  
	bool			isButton() const				{ return (getObjectType() & DIDFT_BUTTON)!=0; }
	bool			isAxis() const					{ return (getObjectType() & DIDFT_AXIS)!=0; }
	bool			isPOV() const					{ return (getObjectType() & DIDFT_POV)!=0; }
	
	DWORD			getDwFlags() const				{ return mDeviceObjectInstance.dwFlags; }
	
	std::string		getName() const;

	DWORD			getDwFFMaxForce() const			{ return mDeviceObjectInstance.dwFFMaxForce; }
    DWORD			getFFForceResolution() const	{ return mDeviceObjectInstance.dwFFForceResolution; }
	WORD			getCollectionNumber() const		{ return mDeviceObjectInstance.wCollectionNumber;}
    WORD			getDesignatorIndex() const		{ return mDeviceObjectInstance.wDesignatorIndex; }
	WORD			getUsagePage() const			{ return mDeviceObjectInstance.wUsagePage; }
    WORD			getUsage() const				{ return mDeviceObjectInstance.wUsage; }
	DWORD			getDimension() const			{ return mDeviceObjectInstance.dwDimension; }
    WORD			getExponent() const				{ return mDeviceObjectInstance.wExponent; }
    WORD			getReportId() const				{ return mDeviceObjectInstance.wReportId; }

private:
	const TCHAR*	getRawName() const				{ return mDeviceObjectInstance.tszName; }
	
	bool operator==( const ObjectInstance& other ) const;
	DIDEVICEOBJECTINSTANCE mDeviceObjectInstance;
	mutable std::string	mName;						// Cached UTF-8 version of name string
};

typedef std::vector<ObjectInstance> ObjectInstances;

}