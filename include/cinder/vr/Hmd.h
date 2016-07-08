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

#include "cinder/vr/Camera.h"
#include "cinder/Area.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"

#include <vector>

namespace cinder { namespace vr {

class Context;
class SessionOptions;

//! \class Hmd
//!
//!
class Hmd {
public:

	virtual ~Hmd();

	ci::vr::Context*					getContext() const { return mContext; }

	ci::vr::Api							getApi() const;

	const ci::vr::SessionOptions&		getSessionOptions() const;

	virtual void						recenterTrackingOrigin() = 0;

	virtual void						bind() = 0;
	virtual void						unbind() = 0;

	bool								isMirrored() const { return mIsMirrrored; }
	void								enableMirrored( bool enabled ) { mIsMirrrored = enabled; }

	bool								isMonoscopic() const { return mIsMonoscopic; }
	void								enableMonoscopic( bool enabled );

	uint32_t							getElapsedFrames() const { return mElapsedFrames; }

	virtual ci::ivec2					getRenderTargetSize() const { return mRenderTargetSize; }

	const std::vector<ci::vr::Eye>&		getEyes() const { return mEyes; }
	virtual	void						enableEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode = ci::vr::COORD_SYS_WORLD ) = 0;

	const ci::vr::CameraEye&			getEyeCamera( ci::vr::Eye eye ) const;
	ci::mat4							getEyeViewMatrix( ci::vr::Eye eye ) const;
	ci::mat4							getEyeProjectionMatrix( ci::vr::Eye eye ) const;
	ci::mat4							getEyeViewProjectionMatrix( ci::vr::Eye eye ) const;
	virtual ci::Area					getEyeViewport( ci::vr::Eye eyeType ) const = 0;

	virtual void						setMatricesEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode );
	// DEPRECATED: virtual void			setLookAt( const ci::vec3 &position, const ci::vec3 &target );
	virtual void						setClip( float nearClip, float farClip );

	virtual void						calculateOriginMatrix() {}
	const ci::mat4&						getOriginMatrix() const { return mOriginMatrix; }
	const ci::mat4&						getDeviceToTrackingMatrix() const { return mDeviceToTrackingMatrix; }
	const ci::mat4&						getTrackingToDeviceMatrix() const { return mTrackingToDeviceMatrix; }
	virtual void						calculateInputRay() {}
	const ci::Ray&						getInputRay() const { return mInputRay; }

	virtual void						drawMirrored( const ci::Rectf& r ) = 0;

	virtual void						drawControllers( ci::vr::Eye eyeType ) = 0;
	virtual void						drawDebugInfo() {}

protected:
	Hmd( ci::vr::Context* context );

	bool								mIsMirrrored = true;
	bool								mIsMonoscopic = false;

	ci::ivec2							mRenderTargetSize = ci::ivec2( 0 );

	uint32_t							mFirstValidPoseFrame = UINT32_MAX;
	uint32_t							mElapsedFrames = 0;

	ci::vec3							mOriginPosition = ci::vec3( 0, 0, 1 );
	ci::vec3							mOriginViewDirection = ci::vec3( 0, 0, -1 );
	ci::vec3							mOriginWorldUp = ci::vec3( 0, 1, 0 );
	ci::quat							mOriginOrientation;
	ci::mat4							mOriginMatrix;
	bool								mOriginInitialized = false;

	std::vector<ci::vr::Eye>			mEyes;
	ci::vr::CameraEye					mEyeCamera[ci::vr::EYE_COUNT];

	ci::mat4							mDeviceToTrackingMatrix;
	ci::mat4							mTrackingToDeviceMatrix;
	ci::Ray								mInputRay = ci::Ray( ci::vec3( 0 ), ci::vec3( 0 ) );

	ci::ColorA							mClearColor = ci::ColorA( 0, 0, 0, 0 );

	void								updateElapsedFrames();

	virtual void						onClipValueChange( float nearClip, float farClip ) = 0;
	virtual void						onMonoscopicChange() = 0;

private:
	ci::vr::Context*					mContext = nullptr;
};

}} // namespace cinder::vr