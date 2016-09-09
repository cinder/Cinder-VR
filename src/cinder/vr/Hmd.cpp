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
	mHmdCamera = ci::vr::CameraEye( ci::vr::EYE_HMD );
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

bool Hmd::isMirroredUndistorted() const
{
	return ( Hmd::MIRROR_MODE_UNDISTORTED_STEREO == mMirrorMode ) ||
		   ( Hmd::MIRROR_MODE_UNDISTORTED_MONO_LEFT == mMirrorMode ) ||
		   ( Hmd::MIRROR_MODE_UNDISTORTED_MONO_RIGHT == mMirrorMode );
}

void Hmd::enableMonoscopic(bool enabled)
{
	mIsMonoscopic = enabled;
	onMonoscopicChange();
}

const ci::vr::CameraEye& Hmd::getEyeCamera( ci::vr::Eye eye ) const
{
	return ( ci::vr::EYE_HMD == eye ) ? mHmdCamera : mEyeCamera[eye];
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
			ci::gl::multModelMatrix( mOriginMatrix );
			ci::gl::multModelMatrix( mLookMatrix );
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

void Hmd::setClip(float nearClip, float farClip)
{
	onClipValueChange( nearClip, farClip );
}

// Not quite working yet
/*
void Hmd::setLookAt( const ci::vec3 &position, const ci::vec3 &target, const ci::vec3 &worldUp )
{
	mLookPosition = position + vec3( 0, 0, getSessionOptions().getOriginOffset().z );
	mLookViewDirection = ci::normalize( target - position );
	mLookWorldUp = worldUp;
	mLookOrientation = ci::quat( glm::toQuat( alignZAxisWithTarget( -mLookViewDirection, mLookWorldUp ) ) );

	ci::vec3 w = -normalize( mLookViewDirection );
	ci::vec3 u = ci::rotate( mLookOrientation, ci::vec3( 1, 0, 0 ) );
	ci::vec3 v = ci::rotate( mLookOrientation, ci::vec3( 0, 1, 0 ) );	

	ci::mat4 rotationMatrix;
	{
		ci::mat4 &m = rotationMatrix;
		m[0][0] =  u.x; m[1][0] =  u.y; m[2][0] =  u.z; m[3][0] = 0.0f;
		m[0][1] =  v.x; m[1][1] =  v.y; m[2][1] =  v.z; m[3][1] = 0.0f;
		m[0][2] =  w.x; m[1][2] =  w.y; m[2][2] =  w.z; m[3][2] = 0.0f;
		m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; m[3][3] = 1.0f;
	}

	ci::mat4 positionMatrix;
	{
		ci::mat4 &m = positionMatrix;
		m[0][0] = 1.0f; m[1][0] = 0.0f; m[2][0] = 0.0f; m[3][0] = -mLookPosition.x;
		m[0][1] = 0.0f; m[1][1] = 1.0f; m[2][1] = 0.0f; m[3][1] = -mLookPosition.y;
		m[0][2] = 0.0f; m[1][2] = 0.0f; m[2][2] = 1.0f; m[3][2] = -mLookPosition.z;
		m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; m[3][3] = 1.0f;
	}

	mLookMatrix = positionMatrix*rotationMatrix;
	mInverseLookMatrix = glm::affineInverse( mLookMatrix );
}
*/

void Hmd::setLookAt( const ci::vec3 &position )
{
	mLookPosition = position + vec3( 0, 0, getSessionOptions().getOriginOffset().z );
	
	mat4 &m = mLookMatrix;
	m[0][0] = 1.0f; m[1][0] = 0.0f; m[2][0] = 0.0f; m[3][0] = -mLookPosition.x;
	m[0][1] = 0.0f; m[1][1] = 1.0f; m[2][1] = 0.0f; m[3][1] = -mLookPosition.y;
	m[0][2] = 0.0f; m[1][2] = 0.0f; m[2][2] = 1.0f; m[3][2] = -mLookPosition.z;
	m[0][3] = 0.0f; m[1][3] = 0.0f; m[2][3] = 0.0f; m[3][3] = 1.0f;

	mInverseLookMatrix = glm::affineInverse( mLookMatrix );
}

void Hmd::drawMirrored(const ci::Rectf& r, bool handleSubmitFrame )
{
	if( ! isMirrored() ) {
		// Submit the frame if requested even if mirror is turned off
		if( handleSubmitFrame ) {
			submitFrame();
		}
		// Bail since mirroring is disabled
		return;
	}

	// Undistorted mirroring requires drawing before frame submission to the device
	if( isMirroredUndistorted() ) {
		// Draw mirrored
		drawMirroredImpl( r );
		// Submit frame
		if( handleSubmitFrame ) {
			submitFrame();
		}
	}
	else {
		// Submit frame
		if( handleSubmitFrame ) {
			submitFrame();
		}
		// Draw mirrored
		drawMirroredImpl( r );
	}
}

}} // namespace cinder::vr