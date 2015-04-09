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
#include "RDIAxis.h"
#include "RDIDevice.h"

#include <assert.h>
#include <sstream>

namespace RDI
{

Axis::Axis( const ObjectInstance& objectInstance, Device* parentDevice )
  : Object(objectInstance, parentDevice), 
	mValue(0),
	mMinValue(0),
	mMaxValue(0)
{
	assert( objectInstance.isAxis() );		
	assert( getParentDevice() );		

	IDirectInputDevice8*  inputDevice = getParentDevice()->getInputDevice();
	assert(inputDevice);

	// Get the axis value range
	//if ((dev->dwType & DIDFT_ABSAXIS) != 0)
    //{
	DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYID;
    range.diph.dwObj = objectInstance.getDwType();
	HRESULT hr = inputDevice->GetProperty(DIPROP_RANGE, &range.diph);
	assert( SUCCEEDED(hr) );
	if ( SUCCEEDED(hr) )
	{
		// All the axes that I've seen on different devices have always had a range of 0 to 65535,
		// whether they are sticks (that are mechanically centered) or so called sliders.
		// So I don't expect to get usefull information here about the neutral position of a 
		// stick/slider based on the values here (like a self-centering axis for example).
		mMinValue = range.lMin;
		mMaxValue = range.lMax;
		assert( mMinValue!=mMaxValue );
		assert( mMinValue<=mMaxValue );

		// We set the initial axis value of the axis to the midle position in its valid interval/range.
		// This seems to be what DirectInput returns initially until the first physical change happens.
		// This initial value can be very far away from reality for example if you start the application 
		// with an axis pushed in one direction. In other words, DirectInput isn't capable of returning 
		// an initial state that reflects the reality. The first correct state will be returned as soon 
		// as the  user touches an object of the device (axis, button, etc...). 
		// This phenomenom can even be seen in the Windows Control Panel when you try to test a
		// game controller device!
		mValue = mMinValue + (mMaxValue-mMinValue)/2;
	}
}

std::string Axis::toString() const
{
	std::stringstream str;
	str << "Axis Id:" << getObjectInstance().getDwType() << " Value:" << getValue() << " (" << getMinValue() << " to " << getMaxValue() << ")";
	str << " Name:'" << getObjectInstance().getName() << "'";
	return str.str();
}

void Axis::updateFrom( const DIDEVICEOBJECTDATA& entry )
{
	setValue( entry.dwData );		
}

void Axis::setValue( LONG value )
{
	// Check/correct out of range value
	assert( value>=mMinValue );
	assert( value<=mMaxValue );
	if ( value<mMinValue )
		value = mMinValue;
	else if ( value>mMaxValue )
		value = mMaxValue;
	
	if ( value==mValue )
		return;
	
	mValue = value;
	notifyChanged();
}

}