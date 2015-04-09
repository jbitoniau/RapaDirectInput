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
#include "RDIDeviceInstance.h"

#include <tchar.h>
#include "RDICommon.h"

namespace RDI
{

DeviceInstance::DeviceInstance()
	:	mInstanceName(),
		mProductName()
{
	memset( &mDeviceInstance, 0, sizeof(DIDEVICEINSTANCE) );
}

DeviceInstance::DeviceInstance( LPCDIDEVICEINSTANCE deviceInstanceData )
	:	mInstanceName(),
		mProductName()
{
	memcpy( &mDeviceInstance, deviceInstanceData, sizeof(DIDEVICEINSTANCE) );
}

DeviceInstance::DeviceInstance( const DeviceInstance& other )
	:	mInstanceName( other.mInstanceName ),
		mProductName( other.mProductName )
{
	memcpy( &mDeviceInstance, &(other.mDeviceInstance), sizeof(DIDEVICEINSTANCE) );
}

bool DeviceInstance::operator==( const DeviceInstance& other ) const
{
	if ( getGuidInstance()!=other.getGuidInstance() )
		return false;
	if ( _tcsncmp( getRawInstanceName(), other.getRawInstanceName(), MAX_PATH )!=0 )
		return false;
	if ( _tcsncmp( getRawProductName(), other.getRawProductName(), MAX_PATH )!=0 )
		return false;
	return true;
}

DeviceInstance& DeviceInstance::operator=( const DeviceInstance& other ) 
{
	memcpy( &mDeviceInstance, &(other.mDeviceInstance), sizeof(DIDEVICEINSTANCE) );
	mInstanceName = other.mInstanceName;
	mProductName = other.mProductName;
	return *this;
}

std::string DeviceInstance::getInstanceName() const
{
	if ( mInstanceName.empty() )
		mInstanceName = Common::TCHARToUTF8( getRawInstanceName() );

/*	// For testing that complex non-ascii device names are properly handled
	{	
#ifdef UNICODE
		mInstanceName = Common::TCHARToUTF8( L"Pound £ Alpha α Oméga ω" );
#else
		unsigned char s[] = { 0x50, 0x6F, 0x75, 0x6E, 0x64, 0x20, 0xC2, 0xA3, 0x20, 0x41, 0x6C, 0x70, 0x68, 0x61, 0x20, 0xCE, 0xB1, 0x20, 0x4F, 0x6D, 0xC3, 0xA9, 0x67, 0x61, 0x20, 0xCF, 0x89, 0x00 };
		//mInstanceName = Common::TCHARToUTF8( L"Pound £ Alpha α Oméga ω");
		mInstanceName = Common::TCHARToUTF8( (const TCHAR*)(s) );
#endif
	}
*/
	return mInstanceName;
}

std::string	DeviceInstance::getProductName() const
{
	if ( mProductName.empty() )
		mProductName = Common::TCHARToUTF8( getRawProductName() );
	return mProductName;
}

}