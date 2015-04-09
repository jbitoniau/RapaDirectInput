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

#include "RDIDeviceInstance.h"
#include "RDIObject.h"

namespace RDI
{

/*
	Device

	The Device represents a joystick or gamepad connected to the computer.
	
	In theory, DirectInput gives also access to the mouse and keyboard but
	this has been deliberately left out as it's not recommended by Microsoft
	
	A Device is made up of various objects like axes, buttons, POV, etc.. 
	These can be inspected using the getObjects() method.

	It's possible to register listeners to the Device so client code can
	be notified whenever its objects change.

	Various information about the device itself (name, type, etc...) can be 
	obtained via the DeviceInstance object associated with it.
*/
class Device
{
public:
	void						update();

	//HWND						getWindowHandle() const			{ return mWindowHandle; }
	IDirectInput8*				getDirectInput() const			{ return mDirectInput; }
	const DeviceInstance&		getDeviceInstance() const		{ return mDeviceInstance; }
	//DWORD						getCoopSettings() const			{ return mCoopSettings; }
	IDirectInputDevice8*		getInputDevice() const			{ return mInputDevice; }

	const Objects&				getObjects() const { return mObjects; }
	
	class Listener
	{
	public:
		virtual void onObjectChanged( Device* /*device*/, Object* /*object*/ ) {}
	};

	void						addListener( Listener* listener );
	bool						removeListener( Listener* listener );
	void						removeListeners();
	
protected:
	friend class DeviceManager;
	Device( /*HWND windowHandle,*/ IDirectInput8* directInput, const DeviceInstance& identifier/*, DWORD coopSettings*/);
	virtual ~Device();

	bool						initialize();
	bool						enumerateObjects();
	static BOOL CALLBACK		enumObjectsCallback( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef );

	void						addObject( Object* object );
	void						deleteObjects();

	static bool					getDeviceData( IDirectInputDevice8* device, LPDIDEVICEOBJECTDATA dataEntries, LPDWORD numDataEntries );
	
	friend class Object;
	void						notifyObjectChanged( Object* object );

private:
	//HWND						mWindowHandle;
	IDirectInput8*				mDirectInput;
	DeviceInstance				mDeviceInstance;
	//DWORD						mCoopSettings;
	
	static const unsigned int	mDataBufferSize = 124;
	IDirectInputDevice8*		mInputDevice;
	
	Objects						mObjects;

	// Listeners
	typedef						std::vector<Listener*> Listeners; 
	Listeners					mListeners;
};

}

