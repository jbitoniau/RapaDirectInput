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
#include "RDICommon.h"

#include <wbemidl.h>		// For isXInputDevice()
#include <oleauto.h>		// For isXInputDevice() (SysAllocString)
#include <stdio.h>			// For isXInputDevice() (swscanf)

//#include "RDITime.h"

namespace RDI
{

std::string	Common::TCHARToUTF8( const TCHAR* tcharString )
{
#ifdef _UNICODE
	return UTF16toUTF8String( tcharString );
#else
	return UTF16toUTF8String( MBCStoUTF16String( tcharString ) );
#endif
}

std::wstring Common::MBCStoUTF16String( const std::string& mbcsString )
{
	if( mbcsString.empty() )    
		return std::wstring();
	// ACP means ANSI Code Page. The conversion depends on the computer current code page settings.
	// See http://msdn.microsoft.com/en-us/library/dd319072%28VS.85%29.aspx
	size_t retStringSize = ::MultiByteToWideChar(CP_ACP, 0, mbcsString.c_str(), static_cast<int>(mbcsString.length()), 0, 0);
    std::wstring retString( retStringSize, L'\0' );
	::MultiByteToWideChar(CP_UTF8, 0, mbcsString.c_str(), static_cast<int>(mbcsString.length()), &retString[0], static_cast<int>(retString.length()));
    return retString;
}

// Use this method when the program is compiled with UNICODE defined (aka UTF16)
std::string Common::UTF16toUTF8String( const std::wstring& utf16String )
{
	if ( utf16String.empty() )
		return std::string();
	int retStringSizeInByte = ::WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), 0, 0, 0, 0);
	std::string retString( retStringSizeInByte, '\0' );
	::WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), &retString[0], retStringSizeInByte, 0, 0);
	return retString;	
}


std::wstring Common::UTF8toUTF16String( const std::string& utf8String )
{
	if( utf8String.empty() )    
		return std::wstring();
	size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), static_cast<int>(utf8String.length()), 0, 0);
    std::wstring ret( reqLength, L'\0' );
	::MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), static_cast<int>(utf8String.length()), &ret[0], static_cast<int>(ret.length()));
    return ret;
}

// The following is taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx
// It is EXTREMELY slow! In the code, it seems that most time is spent between t3 and t4 (>300ms). 
// It might be possible to improve that by not recreating COM objects all the time, i.e. creating a class
// doing the work and maintaining a state.
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
bool Common::isXInputController( const GUID* pGuidProductFromDirectInput )
{
    IWbemLocator*           pIWbemLocator  = NULL;
    IEnumWbemClassObject*   pEnumDevices   = NULL;
    IWbemClassObject*       pDevices[20]   = {0};
    IWbemServices*          pIWbemServices = NULL;
    BSTR                    bstrNamespace  = NULL;
    BSTR                    bstrDeviceID   = NULL;
    BSTR                    bstrClassName  = NULL;
    DWORD                   uReturned      = 0;
    bool                    bIsXinputDevice= false;
    UINT                    iDevice        = 0;
    VARIANT                 var;
    HRESULT                 hr;

//unsigned int t0 = Time::getTimeAsMilliseconds();

    // CoInit if needed
    hr = CoInitialize(NULL);
	bool bCleanupCOM = SUCCEEDED(hr);

//unsigned int t1 = Time::getTimeAsMilliseconds();

    // Create WMI
    hr = CoCreateInstance( __uuidof(WbemLocator),
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(IWbemLocator),
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;
//unsigned int t2 = Time::getTimeAsMilliseconds();
    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;        
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;        
    bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &pIWbemServices );
    if( FAILED(hr) || pIWbemServices == NULL )
        goto LCleanup;
//unsigned int t3 = Time::getTimeAsMilliseconds();
    // Switch security level to IMPERSONATE. 
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );                    

    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;
//unsigned int t4 = Time::getTimeAsMilliseconds();
    // Loop over all devices
    for( ;; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED(hr) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice=0; iDevice<uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it's an XInput device
				    // This information can not be found from DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning ( disable : 4996 )
#endif
                    if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;
#ifdef _MSC_VER
	#pragma warning(pop)
#endif
                    // Compare the VID/PID to the DInput device
                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );
                    if( dwVidPid == pGuidProductFromDirectInput->Data1 )
                    {
                        bIsXinputDevice = true;
                        goto LCleanup;
                    }
                }
            }   
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }
LCleanup:
//unsigned int t5 = Time::getTimeAsMilliseconds();
    if(bstrNamespace)
        SysFreeString(bstrNamespace);
    if(bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if(bstrClassName)
        SysFreeString(bstrClassName);
    for( iDevice=0; iDevice<20; iDevice++ )
        SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    if( bCleanupCOM )
		CoUninitialize();
//unsigned int t6 = Time::getTimeAsMilliseconds();

/*t1 = t1-t0;
t2 = t2-t0;
t3 = t3-t0;
t4 = t4-t0;
t5 = t5-t0;
t6 = t6-t0;
t0 = 0;
*/
    return bIsXinputDevice;
}

std::string	Common::GUIDToString(const GUID* guid)
{
	// Inspired from http://stackoverflow.com/questions/1672677/print-a-guid-variable
	const unsigned int numChars = 37;
	char guidString[numChars];
    sprintf_s(	guidString, numChars,
				"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				guid->Data1, guid->Data2, guid->Data3,
				guid->Data4[0], guid->Data4[1], guid->Data4[2],
				guid->Data4[3], guid->Data4[4], guid->Data4[5],
				guid->Data4[6], guid->Data4[7]);
    return guidString;
}

// See http://msdn.microsoft.com/en-us/library/ee416869(VS.85).aspx
const char* Common::HRESULTToString( HRESULT hr )
{
	switch ( hr ) 
	{
		case	DI_BUFFEROVERFLOW				:	return "S_FALSE,DI_NOTATTACHED,DI_BUFFEROVERFLOW,DI_PROPNOEFFECT,DI_NOEFFECT"; break;
		case	DI_DOWNLOADSKIPPED				:	return "DI_DOWNLOADSKIPPED"; break;
		case	DI_EFFECTRESTARTED				:	return "DI_EFFECTRESTARTED"; break;
		//case	DI_NOEFFECT						:	return "DI_NOEFFECT"; break;
		//case	DI_NOTATTACHED					:	return "DI_NOTATTACHED"; break;
		case	DI_OK							:	return "DI_OK"; break;
		case	DI_POLLEDDEVICE					:	return "DI_POLLEDDEVICE"; break;
		//case	DI_PROPNOEFFECT					:	return "DI_PROPNOEFFECT"; break;
		case	DI_SETTINGSNOTSAVED				:	return "DI_SETTINGSNOTSAVED"; break;
		case	DI_TRUNCATED					:	return "DI_TRUNCATED"; break;
		case	DI_TRUNCATEDANDRESTARTED		:	return "DI_TRUNCATEDANDRESTARTED,(DI_EFFECTRESTARTED | DI_TRUNCATED)"; break;
		case	DI_WRITEPROTECT					:	return "DI_WRITEPROTECT (SUCCESS code indicating that settings cannot be modified)"; break;
		case	DIERR_ACQUIRED					:	return "DIERR_ACQUIRED"; break;
		case	DIERR_ALREADYINITIALIZED		:	return "DIERR_ALREADYINITIALIZED"; break;
		case	DIERR_BADDRIVERVER				:	return "DIERR_BADDRIVERVER"; break;
		case	DIERR_BETADIRECTINPUTVERSION	:	return "DIERR_BETADIRECTINPUTVERSION"; break;
		case	DIERR_DEVICEFULL				:	return "DIERR_DEVICEFULL"; break;
		case	DIERR_DEVICENOTREG				:	return "DIERR_DEVICENOTREG,REGDB_E_CLASSNOTREG"; break;
		case	DIERR_EFFECTPLAYING				:	return "DIERR_EFFECTPLAYING"; break;
		case	DIERR_GENERIC					:	return "DIERR_GENERIC,E_FAIL"; break;
		case	DIERR_HANDLEEXISTS				:	return "DIERR_HANDLEEXISTS,E_ACCESSDENIED, DIERR_OTHERAPPHASPRIO"; break;
		case	DIERR_HASEFFECTS				:	return "DIERR_HASEFFECTS"; break;
		case	DIERR_INCOMPLETEEFFECT			:	return "DIERR_INCOMPLETEEFFECT"; break;
		case	DIERR_INPUTLOST					:	return "DIERR_INPUTLOST"; break;
		case	DIERR_INVALIDPARAM				:	return "DIERR_INVALIDPARAM,E_INVALIDARG "; break;
		case	DIERR_MAPFILEFAIL				:	return "DIERR_MAPFILEFAIL"; break;
		case	DIERR_MOREDATA					:	return "DIERR_MOREDATA"; break;
		case	DIERR_NOAGGREGATION				:	return "DIERR_NOAGGREGATION"; break;
		case	DIERR_NOINTERFACE				:	return "DIERR_NOINTERFACE,E_NOINTERFACE "; break;
		case	DIERR_NOTACQUIRED				:	return "DIERR_NOTACQUIRED"; break;
		case	DIERR_NOTBUFFERED				:	return "DIERR_NOTBUFFERED"; break;
		case	DIERR_NOTDOWNLOADED				:	return "DIERR_NOTDOWNLOADED"; break;
		case	DIERR_NOTEXCLUSIVEACQUIRED		:	return "DIERR_NOTEXCLUSIVEACQUIRED"; break;
		case	DIERR_NOTFOUND					:	return "DIERR_NOTFOUND"; break;
		case	DIERR_NOTINITIALIZED			:	return "DIERR_NOTINITIALIZED"; break;
		//case	DIERR_OBJECTNOTFOUND			:	return "DIERR_OBJECTNOTFOUND"; break;
		case	DIERR_OLDDIRECTINPUTVERSION		:	return "DIERR_OLDDIRECTINPUTVERSION"; break;
		//case	DIERR_OTHERAPPHASPRIO			:	return "DIERR_OTHERAPPHASPRIO"; break;
		case	DIERR_OUTOFMEMORY				:	return "DIERR_OUTOFMEMORY,E_OUTOFMEMORY "; break;
		//case	DIERR_READONLY					:	return "DIERR_READONLY"; break;
		case	DIERR_REPORTFULL				:	return "DIERR_REPORTFULL"; break;
		case	DIERR_UNPLUGGED					:	return "DIERR_UNPLUGGED"; break;
		case	DIERR_UNSUPPORTED				:	return "DIERR_UNSUPPORTED,E_NOTIMPL"; break;
		case	E_HANDLE						:	return "E_HANDLE"; break;
		case	E_PENDING						:	return "E_PENDING"; break;
		case	E_POINTER						:	return "E_POINTER"; break;
	}
	return "E_UNDEFINED";
}

}