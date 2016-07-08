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

#include "cinder/vr/Platform.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Matrix.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

#include <openvr.h>

namespace cinder { namespace vr { namespace openvr  {

class RenderModel;
class RenderModelData;
using RenderModelRef = std::shared_ptr<RenderModel>;
using RenderModelDataRef = std::shared_ptr<RenderModelData>;

//! \class Exception
//!
//!
class Exception : public ci::vr::Exception {
public:
	Exception() {}
	Exception( const std::string& msg ) : ci::vr::Exception( msg ) {}
	virtual ~Exception() {}
};

//! \class RenderModelData
//!
//!
class RenderModelData {
public:
	virtual ~RenderModelData() {}
	static RenderModelDataRef			create( const std::string& name, ::vr::RenderModel_t* model, ::vr::RenderModel_TextureMap_t* texture );
	const std::string&					getName() const { return mName; }
	const ci::gl::VboMeshRef			getVboMesh() const { return mVboMesh; }
	const ci::gl::Texture2dRef&			getTexture() const { return mTexture; }
private:
	RenderModelData( const std::string& name, ::vr::RenderModel_t* model, ::vr::RenderModel_TextureMap_t* texture );
	std::string							mName;
	ci::gl::VboMeshRef					mVboMesh;
	ci::gl::Texture2dRef				mTexture;
};

//! \class RenderModel
//!
//!
class RenderModel {
public:
	virtual~RenderModel() {}
	static RenderModelRef				create( const ci::vr::openvr::RenderModelDataRef& data, const ci::gl::GlslProgRef& shader );
	const ci::gl::BatchRef&				getBatch() const { return mBatch; }
	const ci::gl::GlslProgRef			getShader() const { return mShader; }
	void								draw();
private:
	RenderModel( const ci::vr::openvr::RenderModelDataRef& data, const ci::gl::GlslProgRef& shader );
	RenderModelDataRef					mData;
	ci::gl::GlslProgRef					mShader;
	ci::gl::BatchRef					mBatch;
};

inline ci::mat4 fromOpenVr( const ::vr::HmdMatrix34_t& m )
{
	return ci::mat4(
		m.m[0][0], m.m[1][0], m.m[2][0], 0.0f,
		m.m[0][1], m.m[1][1], m.m[2][1], 0.0f,
		m.m[0][2], m.m[1][2], m.m[2][2], 0.0f,
		m.m[0][3], m.m[1][3], m.m[2][3], 1.0f
	);
}

inline ci::mat4 fromOpenVr( const ::vr::HmdMatrix44_t& m )
{
	return ci::mat4(
		m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
		m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1], 
		m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2], 
		m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]
	);
}

inline ci::vec3 getTranslate( const ::vr::HmdMatrix34_t& m )
{
	return ci::vec3( m.m[0][3], m.m[1][3], m.m[2][3] );
}

std::string	getTrackedDeviceString( ::vr::IVRSystem* vrSystem, ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ::vr::TrackedDeviceProperty prop, ::vr::TrackedPropertyError *peError = nullptr );
ci::mat4	getHmdEyeProjectionMatrix( ::vr::IVRSystem* vrSystem, ::vr::Hmd_Eye eye, float nearClip, float farClip );
ci::mat4	getHmdEyePoseMatrix( ::vr::IVRSystem* vrSystem, ::vr::Hmd_Eye eye );

void threadSleep( unsigned long nMilliseconds );

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )