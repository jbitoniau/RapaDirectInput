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
#include "RDIButton.h"

#include <assert.h>
#include <sstream>

namespace RDI
{
	
Button::Button( const ObjectInstance& objectInstance, Device* parentDevice )
  : Object( objectInstance, parentDevice ), 
	mIsPressed(false)
{
	assert( objectInstance.isButton() );		
}	

std::string Button::toString() const
{
	std::stringstream str;
	str << "Button Id:" << getObjectInstance().getDwType() << " Value:" << mIsPressed << " Name:'" << getObjectInstance().getName() << "'"; 
	return str.str();
}

void Button::updateFrom( const DIDEVICEOBJECTDATA& entry )
{
	bool isPressed = (entry.dwData & 0x80)!=0;
	setPressed( isPressed );
}

void Button::setPressed( bool isPressed )
{
	if ( isPressed==mIsPressed )
		return;
	mIsPressed = isPressed;
	notifyChanged();
}

}