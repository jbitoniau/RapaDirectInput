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
#include "RDIObjectInstance.h"

#include <assert.h>
#include "RDICommon.h"

namespace RDI
{

ObjectInstance::ObjectInstance()
	: mName()
{
	memset( &mDeviceObjectInstance, 0, sizeof(DIDEVICEOBJECTINSTANCE) );
}

ObjectInstance::ObjectInstance( LPCDIDEVICEOBJECTINSTANCE deviceObjectInstance )
	: mName()
{
	memcpy( &mDeviceObjectInstance, deviceObjectInstance, sizeof(DIDEVICEOBJECTINSTANCE) );
}

ObjectInstance::ObjectInstance( const ObjectInstance& other )
	: mName( other.mName )
{
	memcpy( &mDeviceObjectInstance, &(other.mDeviceObjectInstance), sizeof(DIDEVICEOBJECTINSTANCE) );
}

ObjectInstance& ObjectInstance::operator=( const ObjectInstance& other )
{
	memcpy( &mDeviceObjectInstance, &(other.mDeviceObjectInstance), sizeof(DIDEVICEOBJECTINSTANCE) );
	mName = other.mName;
	return *this;
}

std::string	ObjectInstance::getName() const
{
	if ( mName.empty() )
		mName = Common::TCHARToUTF8( getRawName() );
	return mName;
}

/*
const char* ObjectInstance::getGuidTypeString() const
{
	const GUID&	guidType = getGuidType();
	if ( guidType==GUID_XAxis ) 
		return "GUID_XAxis";
	else if ( guidType==GUID_YAxis ) 
		return "GUID_YAxis";
	else if ( guidType==GUID_ZAxis ) 
		return "GUID_ZAxis";
	else if ( guidType==GUID_RxAxis ) 
		return "GUID_RxAxis";
	else if ( guidType==GUID_RyAxis ) 
		return "GUID_RyAxis";
	else if ( guidType==GUID_RzAxis ) 
		return "GUID_RzAxis";
	else if ( guidType==GUID_Slider ) 
		return "GUID_Slider";
	else if ( guidType==GUID_Button ) 
		return "GUID_Button";
	else if ( guidType==GUID_Key ) 
		return "GUID_Key";
	else if ( guidType==GUID_POV ) 
		return "GUID_POV";
	else if ( guidType==GUID_Unknown ) 
		return "GUID_Unknown";
	return "GUID_UNDEFINED";
}*/

}