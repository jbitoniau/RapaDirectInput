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
#include "RDIPOV.h"

#include <assert.h>
#include <sstream>

namespace RDI
{

POV::POV( const ObjectInstance& objectInstance, Device* parentDevice )
  : Object( objectInstance, parentDevice ), 
    mAngle(0), 
	mIsCentered(true)
{
	assert( objectInstance.isPOV() );		
}

std::string POV::toString() const
{
	std::stringstream str;
	str << "POV Id:" << getObjectInstance().getDwType();
	if ( isCentered() )
		str << " Value:Centered";
	else 
		str << " Value:Not centered " << " Angle:" << getAngle();
	str << " Name:'" << getObjectInstance().getName() << "'";
	return str.str();
}

DWORD POV::getAngle() const					
{ 
	assert( !isCentered() );
	if ( isCentered() ) 
		return 0;
	return mAngle; 
}

void POV::updateFrom( const DIDEVICEOBJECTDATA& entry )
{
	if ( LOWORD(entry.dwData)==0xFFFF )
		setValue( true, 0 );
	else 
		setValue( false, entry.dwData );
}

void POV::setValue( bool isCentered, DWORD angle )
{
	assert( (!isCentered) || ( isCentered && angle==0) );
	assert( angle>=0 && angle<=35999 );
	if ( isCentered==mIsCentered && angle==mAngle )
		return;
	mIsCentered = isCentered;
	mAngle = angle;
	notifyChanged();
}


}
