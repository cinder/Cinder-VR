/*
 Copyright 2016 Google Inc.
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.


 Copyright (c) 2016, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/CinderAssert.h"
#include "cinder/Cinder.h"
#include "cinder/Exception.h"
#include "cinder/Quaternion.h"
#include "cinder/Vector.h"

#define CINDER_VR_ENABLE_OCULUS
#define CINDER_VR_ENABLE_OPENVR

namespace cinder { namespace vr {

enum Api {
	API_OCULUS	= 0x00000001,
	API_OPENVR	= 0x00000002,
	API_AUTO	= 0x3FFFFFFF,

	// Starting point for custom device API id's
	API_CUSTOM	= 0x40000000,

	API_MAX		= 0x7FFFFFFF,
	API_ANY		= API_MAX,
	API_UNKNOWN	= 0xFFFFFFFF
};

using ApiFlags = uint32_t;

enum TrackingOrigin {
	TRACKING_ORIGIN_DEVICE_DEFAULT	= 1,
	TRACKING_ORIGIN_SEATED			= 2,
	TRACKING_ORIGIN_STANDING		= 3,
};

enum CoordSys {
	// No coordinate system defined.
	COORD_SYS_NONE,

	// Coordinate system defined by a local coordinate system relative to the device 
	// for both HMDs and controllers.
	COORD_SYS_DEVICE,

	// Coordinate system that is defined by the tracking systems. Generally the 'raw' numbers
	// from the tracking hardware itself. 
	COORD_SYS_TRACKING,

	// Coordinate system defined by a matrix applied over the tracking coordinate system to
	// move the origin to a desired location.
	COORD_SYS_WORLD,
};

enum OriginMode {
	// Origin is the tracking coordinate system origin.
	ORIGIN_MODE_DEFAULT = 0,

	// Origin is offset by a vector V from the tracking coordinate system origin.
	//
	// For example, if V=<1, 2, -5> the origin will be placed at (1, 2, -5) relative to
	// the tracking coordinate system's origin.
	//
	// For standing mode, the origin is offset relative what the HMD considers to be the floor.
	//
	ORIGIN_MODE_OFFSETTED = 1,

	// Origin is offset by a vector V from the HMD's initial position in tracking 
	// coordinate system origin.
	//
	// For example, if V=<1, 2, -5> and the HMD's current position is (1, 1, 2), the  origin will 
	// be placed at (2, 3, -3) relative to the tracking coordinate system's origin.
	//
	// For standing mode, the origin is offset relative what the HMD considers to be the floor.
	//
	ORIGIN_MODE_HMD_OFFSETTED = 2,

	// Origin is offset by a vector V from the HMD using the HMD's current orientation and 
	// position. The origin's Z-axis will be aligned to the HMD's facing direction.
	//
	// For example, if V=<1, 2, -5> and the HMD's current position is (1, 1, 2) and looking 
	// towards positive X, the origin will be placed at (-4, 3, 3). 
	//
	// The origin is always calculated with the the world up vector as <0, 1, 0>. 
	//
	// For standing mode, the origin is offset relative what the HMD considers to be the floor.
	//
	ORIGIN_MODE_HMD_ORIENTED = 3,
};

enum Eye {
	EYE_LEFT	= 0,
	EYE_RIGHT	= 1,
	EYE_COUNT	= 2,
	EYE_HMD     = 0x7FFFFFFF,
	EYE_UNKNOWN	= 0xFFFFFFFF
};

//! \class Exception
//!
//!
class Exception : public ci::Exception {
public:
	Exception() {}
	Exception( const std::string& msg ) : ci::Exception( msg ) {}
	virtual ~Exception() {}
};

}} // namespace cinder::vr