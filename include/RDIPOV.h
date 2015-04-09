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
	POV

	The POV (Point-of-View, aka "Hat switch") object represents a small 
	bidimensional axis indicating a direction. 
	
	At rest, it is mechanically centered. In that case, the method isCentered() 
	returns true. It's not valid to call getAngle(). It will complain 
	in debug and return 0.
	
	If the user moves the stick, isCentered() returns false and getAngle() 
	returns the angle of the stick multiplied by a factor of 100.

	The angle goes clockwise from 0 indicating north, 45000 for north-east, 
	90000 for east, all the way to 31500 for north-west. 

	Note that as far as I've seen, most POV are digital and only provide 
	discrete (45 degrees-spaced) values.
*/
class POV : public Object
{
public:
	POV( const ObjectInstance& objectInstance, Device* parentDevice );

	// Indicate whether the POV stick is centered. In this rest position,
	// the stick doesn't indicate any angle (by default getAngle returns 0)
	bool				isCentered() const				{ return mIsCentered; }
	
	// The angle of the POV stick returned in hundredth-of-degree units (9000 means 90 degrees).
	// It is expressed clockwise with 0 being "north" (i.e forward)
	DWORD				getAngle() const;
	
	virtual std::string	toString() const;
	virtual void		updateFrom( const DIDEVICEOBJECTDATA& entry );

protected:
	void				setValue( bool isCentered, DWORD value );
	
private:
	bool				mIsCentered;
	DWORD				mAngle;
};

}
