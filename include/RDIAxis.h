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

#include "RDIObject.h"

namespace RDI
{

/*
	Axis

	The Axis object represents a 1-D axis in the DirectInput sense.
	The "stick" of a joystick or gamepad is usually composed of 
	two Axis objects, one for horizontal movements and another for 
	vertical ones.

	Sliders or analog triggers are also reported by DirectInput as Axis
	objects. 

	It's unfortunately not possible to distinguish between a mechanically 
	centered axis and a trigger/slider. 

	An Axis will return its value between a min/max value range (note that 
	as far as I could tell, all the axes I've seen report the value in
	the 0..65535 range).	
*/
class Axis : public Object
{
public:
	Axis( const ObjectInstance& objectInstance, Device* parentDevice );

	LONG					getValue() const		{ return mValue; }
	LONG					getMinValue() const		{ return mMinValue; }
	LONG					getMaxValue() const		{ return mMaxValue; }
	
	virtual std::string		toString() const;
	virtual void			updateFrom( const DIDEVICEOBJECTDATA& entry );
	
protected:
	void					setValue( LONG value );

private:
	LONG	mValue;
	LONG	mMinValue;
	LONG	mMaxValue;
};

}