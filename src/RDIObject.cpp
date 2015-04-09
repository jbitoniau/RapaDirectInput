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
#include "RDIObject.h"

#include <assert.h>
#include "RDIButton.h"
#include "RDIAxis.h"
#include "RDIPOV.h"
#include "RDIDevice.h"

namespace RDI
{

Object::Object( const ObjectInstance& objectInstance, Device* parentDevice )
	: mObjectInstance(objectInstance),
	  mParentDevice(parentDevice)
{
	assert(mParentDevice);
}

Object::~Object()
{
}

bool Object::setUserData( UINT_PTR data )
{
	IDirectInputDevice8* inputDevice = getParentDevice()->getInputDevice();
	assert(inputDevice);

	DIPROPPOINTER diptr;
	diptr.diph.dwSize       = sizeof(DIPROPPOINTER);
	diptr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diptr.diph.dwHow        = DIPH_BYID;
	diptr.diph.dwObj        = getObjectInstance().getDwType();
	diptr.uData             = data;

	HRESULT hr = 0;
	hr = inputDevice->SetProperty( DIPROP_APPDATA, &diptr.diph );
	return SUCCEEDED(hr);
}

Object*	Object::createObject( const ObjectInstance& objectInstance, Device* parentDevice )
{
	if ( objectInstance.isAxis() )
		return new Axis( objectInstance, parentDevice );
	else if ( objectInstance.isButton() )
		return new Button( objectInstance, parentDevice );
	else if ( objectInstance.isPOV() )
		return new POV( objectInstance, parentDevice );
	return NULL;
}

void Object::notifyChanged()
{
	getParentDevice()->notifyObjectChanged(this);
}
}