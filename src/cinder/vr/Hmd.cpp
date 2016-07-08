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

#include "cinder/vr/Hmd.h"
#include "cinder/vr/Context.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

namespace cinder { namespace vr {

Hmd::Hmd( ci::vr::Context* context )
	: mContext( context )
{
	mEyes.push_back( ci::vr::EYE_LEFT );
	mEyes.push_back( ci::vr::EYE_RIGHT );
	mEyeCamera[ci::vr::EYE_LEFT] = ci::vr::CameraEye( ci::vr::EYE_LEFT );
	mEyeCamera[ci::vr::EYE_RIGHT] = ci::vr::CameraEye( ci::vr::EYE_RIGHT );
}

Hmd::~Hmd()
{
}

ci::vr::Api Hmd::getApi() const
{
	return mContext->getApi();
}	

const ci::vr::SessionOptions& Hmd::getSessionOptions() const
{
	return mContext->getSessionOptions();
}

void Hmd::enableMonoscopic( bool enabled )
{
	mIsMonoscopic = enabled;
	onMonoscopicChange();
}

/* DEPRECATED:
void Hmd::setLookAt( const ci::vec3 &position, const ci::vec3 &target )
{
	mOriginPosition = position;
	mOriginViewDirection = ci::normalize( target - position );
	mOriginOrientation = ci::quat( glm::toQuat( alignZAxisWithTarget( -mOriginViewDirection, mOriginWorldUp ) ) );

	ci::vec3 mW = -normalize( mOriginViewDirection );
	ci::vec3 mU = ci::rotate( mOriginOrientation, ci::vec3( 1, 0, 0 ) );
	ci::vec3 mV = ci::rotate( mOriginOrientation, ci::vec3( 0, 1, 0 ) );	
	ci::vec3 d  = ci::vec3( - dot( mOriginPosition, mU ), - dot( mOriginPosition, mV ), - dot( mOriginPosition, mW ) );

	mat4 &m = mOriginMatrix;
	m[0][0] = mU.x; m[1][0] = mU.y; m[2][0] = mU.z; m[3][0] =  d.x;
	m[0][1] = mV.x; m[1][1] = mV.y; m[2][1] = mV.z; m[3][1] =  d.y;
	m[0][2] = mW.x; m[1][2] = mW.y; m[2][2] = mW.z; m[3][2] =  d.z;
	m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; m[3][3] = 1.0f;
}
*/

void Hmd::setClip( float nearClip, float farClip )
{
	onClipValueChange( nearClip, farClip );
}

const ci::vr::CameraEye& Hmd::getEyeCamera(ci::vr::Eye eye) const
{
	return mEyeCamera[eye];
}

void Hmd::updateElapsedFrames()
{
	if( UINT32_MAX != mFirstValidPoseFrame ) {
		mElapsedFrames = ci::app::getElapsedFrames() - mFirstValidPoseFrame;
	}
	else {
		mFirstValidPoseFrame = ci::app::getElapsedFrames();	
		mElapsedFrames = 0;
	}
}

ci::mat4 Hmd::getEyeViewMatrix( ci::vr::Eye eye ) const
{
	const ci::vr::CameraEye& cam = mEyeCamera[eye];
	ci::mat4 result = cam.getViewMatrix();
	return result;
}

ci::mat4 Hmd::getEyeProjectionMatrix( ci::vr::Eye eye ) const
{
	const ci::vr::CameraEye& cam = mEyeCamera[eye];
	ci::mat4 result = cam.getProjectionMatrix();
	return result;
}

ci::mat4 Hmd::getEyeViewProjectionMatrix( ci::vr::Eye eye ) const
{
	ci::mat4 viewMat = getEyeViewMatrix( eye );
	ci::mat4 projMat = getEyeProjectionMatrix( eye );
	ci::mat4 result = projMat * viewMat;
	return result;
}

void Hmd::setMatricesEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode )
{
	const auto& cam = getEyeCamera( eye );
	ci::gl::setMatrices( cam );

	switch( eyeMatrixMode ) {
		case ci::vr::COORD_SYS_DEVICE: {
			ci::gl::multModelMatrix( mDeviceToTrackingMatrix );
		}
		break;

		case ci::vr::COORD_SYS_WORLD: {
			ci::gl::multViewMatrix( mOriginMatrix );
		}
		break;

		case ci::vr::COORD_SYS_TRACKING: {
			// No need to do anything
		}
		break;

		default: {
		}
		break;
	}
}

}} // namespace cinder::vr