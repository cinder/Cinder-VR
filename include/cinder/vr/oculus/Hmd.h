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

#include "cinder/vr/Hmd.h"
#include "cinder/vr/oculus/Oculus.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>


namespace cinder { namespace vr { namespace oculus  {

class Context;

class Hmd;
using HmdRef = std::shared_ptr<Hmd>;

//! \class Hmd
//!
//!
class Hmd : public vr::Hmd {
public:

	virtual ~Hmd();

	static ci::vr::oculus::HmdRef		create( ci::vr::oculus::Context *context );

	// ---------------------------------------------------------------------------------------------
	// Public methods inherited from ci::vr::Hmd
	// ---------------------------------------------------------------------------------------------

	virtual void						recenterTrackingOrigin() override;

	virtual void						bind() override;
	virtual void						unbind() override;
	virtual void						submitFrame() override;

	virtual float						getFullFov() const;

	virtual ci::Area					getEyeViewport( ci::vr::Eye eye ) const override;
	
	virtual	void						enableEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode = ci::vr::COORD_SYS_WORLD ) override;

	virtual void						calculateOriginMatrix() override;
	virtual void						calculateInputRay() override;

	virtual void						drawControllers( ci::vr::Eye eyeType ) override;
	virtual void						drawDebugInfo() override;

	// ---------------------------------------------------------------------------------------------
	// Public methods
	// ---------------------------------------------------------------------------------------------

	glm::vec3							getLatencies() const;
	float								getScreenPercentage() const { return mScreenPercentage; }
	void								setScreenPercentage( float value );

	void								cyclePerfHudModes( bool enabled = true );

	glm::ivec2							getNativeResolution() const { return fromOvr( mHmdDesc.Resolution ); }
	glm::ivec2							getRenderTargetSize() const { return mRenderTargetSize; }

protected:
	// ---------------------------------------------------------------------------------------------
	// Protected methods inherited from ci::vr::Hmd
	// ---------------------------------------------------------------------------------------------

	virtual void						onClipValueChange( float nearClip, float farClip ) override;
	virtual void						onMonoscopicChange() override;

	virtual void						drawMirroredImpl( const ci::Rectf& r ) override;

private:
	Hmd( ci::vr::oculus::Context *context );

	ci::vr::oculus::Context				*mContext = nullptr;

	void								initializeRenderTarget();
	void								destroyRenderTarget();

	void								initializeMirrorTexture( const glm::ivec2& size );
	void								destroyMirrorTexture();

	float								mScreenPercentage = 1.3f;
	bool								mIsVisible = true;

	// Oculus SDK
	long long							mFrameIndex = 0;
	::ovrSession						mSession = nullptr;
	::ovrHmdDesc						mHmdDesc;

	float								mNearClip = 0.1f;
	float								mFarClip = 100.0f;

	::ovrEyeRenderDesc					mEyeRenderDesc[ovrEye_Count];
	::ovrPosef							mEyeRenderPose[ovrEye_Count];
	::ovrVector3f						mEyeViewOffset[ovrEye_Count];
	::ovrLayerEyeFov					mBaseLayer;

	::ovrTextureSwapChain				mTextureSwapChain = nullptr;
	std::vector<ci::gl::FboRef>			mRenderTargets;
	int									mCurrentSwapChainIndex = -1;

	::ovrMirrorTexture					mMirrorTexture = nullptr;
	ci::gl::FboRef						mMirrorFbo;

	double								mSensorSampleTime = 0;
	int									mPerfHudMode = 0;
};

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )