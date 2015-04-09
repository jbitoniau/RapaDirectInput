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
#include <tchar.h>

#include <vector>

namespace RDI
{

/*
	DeviceEnumerationTrigger

	Base class for the object that can be used to trigger a device enumeration (ie an update 
	of the Device List)at the DeviceManager level.
*/
class DeviceEnumerationTrigger
{
public:
	virtual~ DeviceEnumerationTrigger() {}
	virtual bool	enumerationNeeded() = 0;
};

/*
	WindowsHookEnumerationTrigger

	A trigger based on a Windows Proc hook. The hook is installed, then
	whenever a WM_DEVICECHANGE message is received, the trigger indicates 
	that a device enumeration at the DeviceManager level is needed
*/
class WindowsHookEnumerationTrigger : public DeviceEnumerationTrigger
{
public:
	WindowsHookEnumerationTrigger( bool createInvisibleWindow );	
	virtual ~WindowsHookEnumerationTrigger();
	virtual bool	enumerationNeeded();

private:
	bool			createInvisibleWindow();
	static LRESULT CALLBACK invisibleWindowWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static const TCHAR* invisibleWindowClassName;

	static LRESULT CALLBACK wndProcHook( int nCode, WPARAM wParam, LPARAM lParam );
	static std::vector<WindowsHookEnumerationTrigger*> mTriggerInstances;
	static HHOOK	mHookHandle;
	
	bool			mCreateInvisibleWindow;
	HWND			mInvisibleWindow;
	bool			mEnumerationNeeded;
};	
	
/*
	TimeBasedEnumerationTrigger
*/
class TimeBasedEnumerationTrigger : public DeviceEnumerationTrigger
{
public:
	TimeBasedEnumerationTrigger( unsigned int intervalInMs );	
	virtual bool	enumerationNeeded();

private:
	int				updateNextTime();

	unsigned int	mIntervalInMs;
	unsigned int	mStartTime;
	unsigned int	mNextTime;
};	

}