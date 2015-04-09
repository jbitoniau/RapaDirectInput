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
#include "RDIDeviceEnumerationTrigger.h"

#include <assert.h>
#include <algorithm>
#include "RDITime.h"
#include "RDICommon.h"

/*
	Notes:
	- Maybe an the "invisible" window code of the WindowsHookEnumerationTrigger could be moved into a InvisibleWindowEnumerationTrigger class. 
	  It would create the invisible window and handle the WM_DEVICECHANGE message in its WndProc normally (without a hook). 
	  This would be cleaner
*/
namespace RDI
{

/*
	WindowsHookEnumerationTrigger
*/
HHOOK WindowsHookEnumerationTrigger::mHookHandle = 0;
std::vector<WindowsHookEnumerationTrigger*> WindowsHookEnumerationTrigger::mTriggerInstances;
const TCHAR* WindowsHookEnumerationTrigger::invisibleWindowClassName = _T("WindowsHookEnumerationTrigger");

WindowsHookEnumerationTrigger::WindowsHookEnumerationTrigger( bool doCreateInvisibleWindow )
	:	mCreateInvisibleWindow( doCreateInvisibleWindow ),
		mInvisibleWindow(NULL),
		mEnumerationNeeded(false)		
{
	if ( mCreateInvisibleWindow )
	{
		bool ret = createInvisibleWindow();
		assert( ret );
	}

	// We initialize mEnumerationNeeded to true so a first enumeration is triggered immediately 
	// (i.e. the first time enumerationNeeded() is called), rather than having to wait 
	// for a first system-wide WM_DEVICECHANGE message
	mEnumerationNeeded = true;

	if ( mTriggerInstances.empty() )
	{
		DWORD threadID = GetCurrentThreadId();
		HINSTANCE hInstance = GetModuleHandle(NULL) ;
		mHookHandle = SetWindowsHookEx( WH_CALLWNDPROC, wndProcHook, hInstance, threadID );
		assert( mHookHandle );
	}
	mTriggerInstances.push_back(this);
}

WindowsHookEnumerationTrigger::~WindowsHookEnumerationTrigger()
{
	std::vector<WindowsHookEnumerationTrigger*>::iterator itr;
	itr = std::find( mTriggerInstances.begin(), mTriggerInstances.end(), this );
	assert( itr!=mTriggerInstances.end() );
	mTriggerInstances.erase(itr);
	
	if ( mTriggerInstances.empty() )
	{
		SetLastError( ERROR_SUCCESS);
		/*BOOL ret =*/ UnhookWindowsHookEx( mHookHandle );
		//DWORD a = GetLastError();
		//assert( ret!=0 );
		mHookHandle = 0;
	}

	if ( mCreateInvisibleWindow )
	{
		/*BOOL ret =*/ DestroyWindow( mInvisibleWindow );
		//assert( ret!=0 );
		mInvisibleWindow = NULL;

		// Try to unregister the class. This will work only if there no Window
		// in existence that uses this class 
		HINSTANCE hInstance = GetModuleHandle(NULL) ;
		/*ret =*/ UnregisterClass( invisibleWindowClassName, hInstance );
		//if ( ret!=0)
		//	printf("OK");
	}
}

bool WindowsHookEnumerationTrigger::enumerationNeeded()
{
	MSG Msg;
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) > 0) 
    {
		TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

	bool ret = mEnumerationNeeded;
	if ( mEnumerationNeeded )
		mEnumerationNeeded = false;
	return ret;
}

LRESULT CALLBACK WindowsHookEnumerationTrigger::invisibleWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// We could check for WM_DEVICECHANGE here, but we use a hook for that
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool WindowsHookEnumerationTrigger::createInvisibleWindow()
{
	// Inspired from http://www.winprog.org/tutorial/simple_window.html
	// and http://msdn.microsoft.com/en-us/library/ms632599(VS.85).aspx#message_only
	WNDCLASS wndclass;
	memset( &wndclass, 0, sizeof(wndclass) );
	HINSTANCE hInstance = GetModuleHandle(NULL) ;
    if ( GetClassInfo( hInstance, invisibleWindowClassName, &wndclass )==0 )
	{
		WNDCLASSEX wc;
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.style         = 0;		
		wc.lpfnWndProc   = invisibleWindowWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = invisibleWindowClassName;
		wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	 	if( !RegisterClassEx(&wc) )
			return false;
	}
    
	// Message-only windows, can't receive broadcast messages
	// http://msdn.microsoft.com/en-us/library/ms632599(VS.85).aspx#message_only
	HWND hwnd = CreateWindowEx( WS_EX_CLIENTEDGE, invisibleWindowClassName, _T(""), WS_OVERLAPPEDWINDOW, 
								CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, 	
								NULL /*HWND_MESSAGE*/,	NULL, hInstance, NULL );
    if( hwnd==NULL )
	{
		BOOL ret = UnregisterClass( invisibleWindowClassName, hInstance );
		assert( ret!=0 );
		return false;
	}

	// The invisible window is not initially shown nor updated
    // ShowWindow(hwnd, SW_SHOW);
	// UpdateWindow(hwnd);

	// Remember the invisible window
	mInvisibleWindow = hwnd;

	return true;
}

LRESULT CALLBACK WindowsHookEnumerationTrigger::wndProcHook( int nCode, WPARAM wParam, LPARAM lParam )
{
	if ( nCode<0 )  // Do not process message 
		return CallNextHookEx( NULL, nCode, wParam, lParam );

	if ( wParam!=0 ) 
	{
		const CWPSTRUCT& params = *reinterpret_cast<CWPSTRUCT*>(lParam);
		if ( params.message==WM_DEVICECHANGE )
		{
			for ( std::size_t i=0; i<WindowsHookEnumerationTrigger::mTriggerInstances.size(); ++i )
				WindowsHookEnumerationTrigger::mTriggerInstances[i]->mEnumerationNeeded = true;
		}
	}

	// Process event
	return CallNextHookEx( NULL, nCode, wParam, lParam );
}

/*
	TimeBasedEnumerationTrigger
*/
TimeBasedEnumerationTrigger::TimeBasedEnumerationTrigger( unsigned int intervalInMs )
	: mIntervalInMs(intervalInMs),
	  mStartTime(0),
	  mNextTime(0)
{
	mStartTime = Time::getTimeAsMilliseconds();
	
	// At startup, we schedule the enumeration as soon as possible,
	// instead of waiting an initial interval like so:
	// updateNextTime();
	mNextTime = mStartTime;
}

bool TimeBasedEnumerationTrigger::enumerationNeeded()
{
	bool ret = false;
	unsigned int currentTime = Time::getTimeAsMilliseconds();
	if ( currentTime>=mNextTime )
	{
		ret = true;
		updateNextTime();
	}
	return ret;
}

// Return the number of intervals done since start. 
// Returns -1 if interval was set to 0 (continuous firing)
int TimeBasedEnumerationTrigger::updateNextTime()
{
	int numIntervalsDone = -1;
	unsigned int currentTime = Time::getTimeAsMilliseconds();
	if ( mIntervalInMs==0 )
	{
		mNextTime = currentTime;
	}
	else
	{
		unsigned int timeSinceStart = currentTime - mStartTime;
		numIntervalsDone = timeSinceStart / mIntervalInMs;
		assert( numIntervalsDone>=0 );
		mNextTime = mStartTime + (static_cast<unsigned int>(numIntervalsDone) + 1) * mIntervalInMs;
	}
	return numIntervalsDone;
}

}

