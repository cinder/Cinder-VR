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

#include "cinder/vr/openvr/Hmd.h"
#include "cinder/vr/openvr/Context.h"
#include "cinder/vr/openvr/DeviceManager.h"
#include "cinder/vr/openvr/OpenVr.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

#include "cinder/app/App.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/draw.h"
#include "cinder/gl/scoped.h"
#include "cinder/Log.h"

#include <iomanip>
#include <utility>

namespace cinder { namespace vr { namespace openvr {

const float kFullFov = 110.0f; // Degrees

std::string toString( const ci::mat4& mat ) 
{
	const float *m = &(mat[0][0]);
	std::stringstream ss;
    ss << std::fixed << std::setprecision(5);
    ss << "[" << std::setw(10) << m[0] << " " << std::setw(10) << m[4] << " " << std::setw(10) << m[8]  <<  " " << std::setw(10) << m[12] << "]\n"
       << "[" << std::setw(10) << m[1] << " " << std::setw(10) << m[5] << " " << std::setw(10) << m[9]  <<  " " << std::setw(10) << m[13] << "]\n"
       << "[" << std::setw(10) << m[2] << " " << std::setw(10) << m[6] << " " << std::setw(10) << m[10] <<  " " << std::setw(10) << m[14] << "]\n"
       << "[" << std::setw(10) << m[3] << " " << std::setw(10) << m[7] << " " << std::setw(10) << m[11] <<  " " << std::setw(10) << m[15] << "]\n";
    //ss << std::resetissflags(std::iss_base::fixed | std::iss_base::floatfield);
    return ss.str();
}

// -------------------------------------------------------------------------------------------------
// Distortion Shader
// -------------------------------------------------------------------------------------------------
const std::string kDistortionShaderVertex = 
	"#version 410 core\n"
	"layout(location = 0) in vec4 ciPosition;\n"
	"layout(location = 1) in vec2 ciTexCoord0;\n"
	"layout(location = 2) in vec2 ciTexCoord1;\n"
	"layout(location = 3) in vec2 ciTexCoord2;\n"
	"uniform mat4 ciModelViewProjection;\n"
	"noperspective out vec2 v2UVred;\n"
	"noperspective out vec2 v2UVgreen;\n"
	"noperspective out vec2 v2UVblue;\n"
	"void main()\n"
	"{\n"
	"	v2UVred   = ciTexCoord0;\n"
	"	v2UVgreen = ciTexCoord1;\n"
	"	v2UVblue  = ciTexCoord2;\n"
	"	gl_Position = ciModelViewProjection * ciPosition;\n"
	"}\n";

const std::string kDistortionShadeFragment = 
	"#version 410 core\n"
	"uniform sampler2D uTex0;\n"
	"noperspective in vec2 v2UVred;\n"
	"noperspective in vec2 v2UVgreen;\n"
	"noperspective in vec2 v2UVblue;\n"
	"out vec4 outputColor;\n"
	"void main() { \n"
	"	float fBoundsCheck = ( (dot( vec2( lessThan( v2UVgreen.xy, vec2(0.05, 0.05)) ), vec2(1.0, 1.0))+dot( vec2( greaterThan( v2UVgreen.xy, vec2( 0.95, 0.95)) ), vec2(1.0, 1.0))) );\n"
	"	if( fBoundsCheck > 1.0 ) { \n"
	"		outputColor = vec4( 0.0, 0.0, 0.0, 1.0 );\n"
	"	}\n"
	"	else {\n"
	"		float r = texture( uTex0, v2UVred ).x;\n"
	"		float g = texture( uTex0, v2UVgreen ).y;\n"
	"		float b = texture( uTex0, v2UVblue ).z;\n"
	"		outputColor = vec4( r, g, b, 1.0 );"
	"	}\n"
	"}\n";

// -------------------------------------------------------------------------------------------------
// Render Model Shader
// -------------------------------------------------------------------------------------------------
const std::string kRenderModelShaderVertex =
	"#version 410\n"
	"uniform mat4 uMatrix;\n"
	"layout(location = 0) in vec4 ciPosition;\n"
	"layout(location = 1) in vec3 ciNormal;\n"
	"layout(location = 2) in vec2 ciTexCoord0;\n"
	"out vec2 v2TexCoord;\n"
	"void main()\n"
	"{\n"
	"	v2TexCoord = ciTexCoord0;\n"
	"	gl_Position = uMatrix * vec4(ciPosition.xyz, 1);\n"
	"}\n";

const std::string kRenderModelShaderFragment =
	"#version 410 core\n"
	"uniform sampler2D uTex0;\n"
	"in vec2 v2TexCoord;\n"
	"out vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"   outputColor = texture( uTex0, v2TexCoord );\n"
	"}\n";

// -------------------------------------------------------------------------------------------------
// Controller Shader
// -------------------------------------------------------------------------------------------------
const std::string kControllerShaderVertex =
	"#version 410\n"
	"uniform mat4 uMatrix;\n"
	"layout(location = 0) in vec4 ciPosition;\n"
	"layout(location = 1) in vec3 ciColor;\n"
	"out vec4 v4Color;\n"
	"void main()\n"
	"{\n"
	"	v4Color.xyz = ciColor;\n"
	"	v4Color.a = 1.0;\n"
	"	gl_Position = uMatrix * ciPosition;\n"
	"}\n";

const std::string kControllerShaderFragment =
	"#version 410\n"
	"in vec4 v4Color;\n"
	"out vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"   outputColor = v4Color;\n"
	"}\n";

// -------------------------------------------------------------------------------------------------
// Hmd
// -------------------------------------------------------------------------------------------------
Hmd::Hmd( ci::vr::openvr::Context* context )
	: ci::vr::Hmd( context ), mContext( context )
{
	mNearClip = context->getSessionOptions().getNearClip();
	mFarClip = context->getSessionOptions().getFarClip();

	mVrSystem = context->getVrSystem();

	mRenderModels.resize( ::vr::k_unMaxTrackedDeviceCount );	

	setupShaders();
	setupMatrices();
	setupStereoRenderTargets();
	setupDistortion();
	setupRenderModels();
	setupCompositor();

	auto shader = ci::gl::getStockShader( ci::gl::ShaderDef().texture( GL_TEXTURE_2D ) );
	mControllerIconBatch[ci::vr::Controller::TYPE_LEFT] = ci::gl::Batch::create( ci::geom::Plane().axes( ci::vec3( 1, 0, 0 ), ci::vec3( 0, 0, -1 ) ), shader );
	mControllerIconBatch[ci::vr::Controller::TYPE_RIGHT] = ci::gl::Batch::create( ci::geom::Plane().axes( ci::vec3( 1, 0, 0 ), ci::vec3( 0, 0, -1 ) ), shader );
}

Hmd::~Hmd()
{
}

ci::vr::openvr::HmdRef Hmd::create( ci::vr::openvr::Context* context )
{
	ci::vr::openvr::HmdRef result = ci::vr::openvr::HmdRef( new ci::vr::openvr::Hmd( context ) );
	return result;
}

void Hmd::setupShaders()
{
	// Distortion shader
	try {
		mDistortionShader = ci::gl::GlslProg::create( kDistortionShaderVertex, kDistortionShadeFragment );
	}
	catch( const std::exception& e ) {
		std::string errMsg = "Distortion shader failed(" + std::string( e.what() ) + ")";
		throw ci::vr::openvr::Exception( errMsg );
	}

	// Render model shader
	try {
		mRenderModelShader = ci::gl::GlslProg::create( kRenderModelShaderVertex, kRenderModelShaderFragment );
	}
	catch( const std::exception& e ) {
		std::string errMsg = "Render model shader failed(" + std::string( e.what() ) + ")";
		throw ci::vr::openvr::Exception( errMsg );
	}

	// Controller shader
	try {
		mControllerShader = ci::gl::GlslProg::create( kControllerShaderVertex, kControllerShaderFragment );
	}
	catch( const std::exception& e ) {
		std::string errMsg = "Controller shader failed(" + std::string( e.what() ) + ")";
		throw ci::vr::openvr::Exception( errMsg );
	}
}

void Hmd::setupMatrices()
{
	mEyePoseMatrix[ci::vr::EYE_LEFT]        = getHmdEyePoseMatrix( mVrSystem, ::vr::Eye_Left );
	mEyePoseMatrix[ci::vr::EYE_RIGHT]       = getHmdEyePoseMatrix( mVrSystem, ::vr::Eye_Right );
	mEyeProjectionMatrix[ci::vr::EYE_LEFT]  = getHmdEyeProjectionMatrix( mVrSystem, ::vr::Eye_Left, mNearClip, mFarClip );
	mEyeProjectionMatrix[ci::vr::EYE_RIGHT] = getHmdEyeProjectionMatrix( mVrSystem, ::vr::Eye_Right, mNearClip, mFarClip );

	// Set eye pose matrices
	mEyeCamera[ci::vr::EYE_LEFT].setViewMatrix( mEyePoseMatrix[ci::vr::EYE_LEFT] );
	mEyeCamera[ci::vr::EYE_RIGHT].setViewMatrix( mEyePoseMatrix[ci::vr::EYE_RIGHT] );

	// Set eye projection matrices
	mEyeCamera[ci::vr::EYE_LEFT].setProjectionMatrix( mEyeProjectionMatrix[ci::vr::EYE_LEFT] );
	mEyeCamera[ci::vr::EYE_RIGHT].setProjectionMatrix( mEyeProjectionMatrix[ci::vr::EYE_RIGHT] );
}

void Hmd::setupStereoRenderTargets()
{
	uint32_t renderWidth = 0;
	uint32_t renderHeight = 0;
	mVrSystem->GetRecommendedRenderTargetSize( &renderWidth, &renderHeight );
	mRenderTargetSize = ivec2( static_cast<int32_t>( renderWidth ), static_cast<int32_t>( renderHeight ) );
	CI_LOG_I( "mRenderTargetSize=" << mRenderTargetSize );

	// Texture format
	ci::gl::Texture2d::Format texFormat = ci::gl::Texture2d::Format();
	texFormat.setInternalFormat( GL_RGBA8 );
	texFormat.setWrapS( GL_CLAMP_TO_EDGE );
	texFormat.setWrapT( GL_CLAMP_TO_EDGE );
	texFormat.setMinFilter( GL_LINEAR );
	texFormat.setMagFilter( GL_LINEAR );
	// Fbo format
	ci::gl::Fbo::Format fboFormat = ci::gl::Fbo::Format();
	fboFormat.setSamples( 4 );
	fboFormat.setColorTextureFormat( texFormat );
	fboFormat.enableDepthBuffer();
	// Render targets
	mRenderTargetLeft = ci::gl::Fbo::create( mRenderTargetSize.x, mRenderTargetSize.y, fboFormat );
	mRenderTargetRight = ci::gl::Fbo::create( mRenderTargetSize.x, mRenderTargetSize.y, fboFormat );
}

void Hmd::setupDistortion()
{
	struct VertexDesc {
		ci::vec2 position;
		ci::vec2 texCoordRed;
		ci::vec2 texCoordGreen;
		ci::vec2 texCoordBlue;
	};

	std::vector<VertexDesc> vertexData;
	std::vector<uint16_t> indices;

	int lensGridSegmentCountH = 43;
	int lensGridSegmentCountV = 43;

	float w = 1.0f/static_cast<float>( lensGridSegmentCountH - 1 );
	float h = 1.0f/static_cast<float>( lensGridSegmentCountV - 1 );

	// Left eye distortion vertex data
	float xOffset = -1.0f;
	for( int y = 0; y < lensGridSegmentCountV; y++ ) {
		for( int x = 0; x < lensGridSegmentCountH; ++x ) {
			float u = x * w; 
			float v = 1.0f - ( y * h );
			::vr::DistortionCoordinates_t dc = mVrSystem->ComputeDistortion( ::vr::Eye_Left, u, v );
			
			VertexDesc vert		= VertexDesc();
			vert.position		= ci::vec2( xOffset + u, -1.0f + ( 2.0f * y * h ) );
			vert.texCoordRed	= ci::vec2( dc.rfRed[0],   1.0f - dc.rfRed[1] );
			vert.texCoordGreen	= ci::vec2( dc.rfGreen[0], 1.0f - dc.rfGreen[1] );
			vert.texCoordBlue	= ci::vec2( dc.rfBlue[0],  1.0f - dc.rfBlue[1] );
			vertexData.push_back( vert );
		}
	}

	// Right eye distortion vertex data
	xOffset = 0;
	for( int y = 0; y < lensGridSegmentCountV; y++ ) {
		for( int x = 0; x < lensGridSegmentCountH; ++x ) {
			float u = x * w; 
			float v = 1 - ( y * h );
			::vr::DistortionCoordinates_t dc = mVrSystem->ComputeDistortion( ::vr::Eye_Right, u, v );

			VertexDesc vert		= VertexDesc();
			vert.position		= ci::vec2( xOffset + u, -1.0f + ( 2.0f * y * h ) );
			vert.texCoordRed	= ci::vec2( dc.rfRed[0],   1.0f - dc.rfRed[1] );
			vert.texCoordGreen	= ci::vec2( dc.rfGreen[0], 1.0f - dc.rfGreen[1] );
			vert.texCoordBlue	= ci::vec2( dc.rfBlue[0],  1.0f - dc.rfBlue[1] );
			vertexData.push_back( vert );
		}
	}

	// Left eye indices
	uint16_t offset = 0;
	for( uint16_t y = 0; y < ( lensGridSegmentCountV - 1 ); ++y )	{
		for( uint16_t x = 0; x < ( lensGridSegmentCountH - 1 ); ++x )		{
			uint16_t a = ( lensGridSegmentCountH * y ) + x + offset;
			uint16_t b = ( lensGridSegmentCountH * y ) + x + 1 + offset;
			uint16_t c = ( ( y+ 1 ) * lensGridSegmentCountH ) + x + 1 + offset;
			uint16_t d = ( ( y +1 ) * lensGridSegmentCountH ) + x  + offset;

			indices.push_back( a );
			indices.push_back( b );
			indices.push_back( c );

			indices.push_back( a );
			indices.push_back( c );
			indices.push_back( d );
		}
	}

	// Right eye indices
	offset = (lensGridSegmentCountH)*(lensGridSegmentCountV);
	for( uint16_t y = 0; y < ( lensGridSegmentCountV - 1 ); ++y ) {
		for( uint16_t x = 0; x < ( lensGridSegmentCountH - 1 ); ++x ) {
			uint16_t a = ( lensGridSegmentCountH * y ) + x + offset;
			uint16_t b = ( lensGridSegmentCountH * y ) + x + 1 + offset;
			uint16_t c = ( ( y+ 1 ) * lensGridSegmentCountH ) + x + 1 + offset;
			uint16_t d = ( ( y +1 ) * lensGridSegmentCountH ) + x  + offset;

			indices.push_back( a );
			indices.push_back( b );
			indices.push_back( c );

			indices.push_back( a );
			indices.push_back( c );
			indices.push_back( d );
		}
	}

	// Distortion index count
	mDistortionIndexCount = static_cast<uint32_t>( indices.size() );

	// Vertex data vbo
	ci::geom::BufferLayout layout = ci::geom::BufferLayout();
	layout.append( ci::geom::POSITION,    2, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, position ) ),      0 );
	layout.append( ci::geom::TEX_COORD_0, 2, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, texCoordRed ) ),   0 );
	layout.append( ci::geom::TEX_COORD_1, 2, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, texCoordGreen ) ), 0 );
	layout.append( ci::geom::TEX_COORD_2, 2, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, texCoordBlue ) ),  0 );	
	ci::gl::VboRef vertexDataVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, vertexData );
	std::vector<std::pair<ci::geom::BufferLayout, ci::gl::VboRef>> vertexArrayBuffers = { std::make_pair( layout, vertexDataVbo ) };
	
	// Indices vbo
	ci::gl::VboRef indicesVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indices );

	// Vbo mesh
	ci::gl::VboMeshRef vboMesh = ci::gl::VboMesh::create( static_cast<uint32_t>( vertexData.size() ), GL_TRIANGLES, vertexArrayBuffers, static_cast<uint32_t>( indices.size() ), GL_UNSIGNED_SHORT, indicesVbo );

	// Create batch
	mDistortionBatch = ci::gl::Batch::create( vboMesh, mDistortionShader );
}

void Hmd::setupRenderModels()
{
	// Allocate entries for all tracked devices
	mRenderModels.resize( ::vr::k_unMaxTrackedDeviceCount );

	for( ::vr::TrackedDeviceIndex_t trackedDeviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd + 1; trackedDeviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++trackedDeviceIndex ) {
		activateRenderModel( trackedDeviceIndex );
	}
}

void Hmd::setupCompositor()
{
	if( ! ::vr::VRCompositor() ) {
		throw ci::vr::openvr::Exception( "Couldn't initialize compositor" );
	}

	switch( getSessionOptions().getTrackingOrigin() ) {
		case ci::vr::TRACKING_ORIGIN_SEATED: {
			::vr::VRCompositor()->SetTrackingSpace( ::vr::TrackingUniverseSeated );
		}
		break;

		case ci::vr::TRACKING_ORIGIN_STANDING: {
			::vr::VRCompositor()->SetTrackingSpace( ::vr::TrackingUniverseStanding );
		}
		break;
	}
}

void Hmd::updatePoseData()
{
	mContext->updatePoseData();
	const auto& pose = mContext->getPose( ::vr::k_unTrackedDeviceIndex_Hmd );

	if( pose.bPoseIsValid ) {
		const auto& hmdMat = mContext->getTrackingToDeviceMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
		mEyeCamera[ci::vr::EYE_LEFT].setHmdMatrix( hmdMat );
		mEyeCamera[ci::vr::EYE_RIGHT].setHmdMatrix( hmdMat );
		mHmdCamera.setHmdMatrix( hmdMat );

		mDeviceToTrackingMatrix = mContext->getDeviceToTrackingMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
		mTrackingToDeviceMatrix = mContext->getTrackingToDeviceMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
	}

	if( ( ! mOriginInitialized ) && pose.bPoseIsValid ) {
		calculateOriginMatrix();
		// Flag as initialized		
		mOriginInitialized = true;
	}

	if( mOriginInitialized ) {
		calculateInputRay();
	}
}

void Hmd::updateControllerGeometry()
{
	if( mVrSystem->IsInputFocusCapturedByAnotherProcess() ) {
		return;
	}

	struct VertexDesc {
		ci::vec4 position;
		ci::vec3 color;
		VertexDesc() {}
		VertexDesc( const ci::vec4& p, const ci::vec3& c )
			: position( p ), color( c ) {}
	};

	mControllerVertexCount = 0;
	mControllerCount = 0;

	std::vector<VertexDesc> vertexData;
	for( ::vr::TrackedDeviceIndex_t deviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex ) {
		if( ! mVrSystem->IsTrackedDeviceConnected( deviceIndex ) ) {		
			continue;
		}

		if( mVrSystem->GetTrackedDeviceClass( deviceIndex ) != ::vr::TrackedDeviceClass_Controller ) {
			continue;
		}

		mControllerCount += 1;

		const auto& pose = mContext->getPose( deviceIndex );
		if( ! pose.bPoseIsValid ) {
			continue;
		}

		const ci::mat4& mat = mContext->getDeviceToTrackingMatrix( deviceIndex );

		ci::vec4 center = mat * ci::vec4( 0, 0, 0, 1 );
		for( uint32_t i = 0; i < 3; ++i ) {
			ci::vec3 color( 0, 0, 0 );
			ci::vec4 point( 0, 0, 0, 1 );
			point[i] +=  ( 2 == i ? -1 : 1 ) * 0.05f;  // offset in X, Y, Z
			color[i] = 1.0f;		// R, G, B
			point = mat * point;

			vertexData.push_back( VertexDesc( center, color ) );
			vertexData.push_back( VertexDesc( point, color ) );
		
			mControllerVertexCount += 2;
		}
	}

	
	if( ! vertexData.empty() ) {
		if( mControllerVbo ) {
			mControllerVbo->bufferData( vertexData.size() * sizeof( VertexDesc ), static_cast<const GLvoid*>( vertexData.data() ), GL_STREAM_DRAW );
		}
		else {
			// Vertex data vbo
			ci::geom::BufferLayout layout = ci::geom::BufferLayout();
			layout.append( ci::geom::POSITION, 4, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, position ) ), 0 );
			layout.append( ci::geom::COLOR,    3, sizeof( VertexDesc ), static_cast<size_t>( offsetof( VertexDesc, color ) ),    0 );
			mControllerVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, vertexData );
			std::vector<std::pair<ci::geom::BufferLayout, ci::gl::VboRef>> vertexArrayBuffers = { std::make_pair( layout, mControllerVbo ) };
	
			// Vbo mesh
			ci::gl::VboMeshRef vboMesh = ci::gl::VboMesh::create( static_cast<uint32_t>( vertexData.size() ), GL_LINES, vertexArrayBuffers );

			// Create batch
			mControllerBatch = ci::gl::Batch::create( vboMesh, mControllerShader );
		}
	}
}

void Hmd::onClipValueChange( float nearClip, float farClip )
{
	mNearClip = nearClip;
	mFarClip = farClip;
	
	// Update projection matrices
	mEyeProjectionMatrix[ci::vr::EYE_LEFT]  = getHmdEyeProjectionMatrix( mVrSystem, ::vr::Eye_Left, mNearClip, mFarClip );
	mEyeProjectionMatrix[ci::vr::EYE_RIGHT] = getHmdEyeProjectionMatrix( mVrSystem, ::vr::Eye_Right, mNearClip, mFarClip );
	
	// Set eye projection matrices
	mEyeCamera[ci::vr::EYE_LEFT].setProjectionMatrix( mEyeProjectionMatrix[ci::vr::EYE_LEFT] );
	mEyeCamera[ci::vr::EYE_RIGHT].setProjectionMatrix( mEyeProjectionMatrix[ci::vr::EYE_RIGHT] );
}

void Hmd::onMonoscopicChange()
{
}

void Hmd::recenterTrackingOrigin()
{
}

void Hmd::bind()
{
	updateControllerGeometry();
}

void Hmd::unbind()
{
	mRenderTargetLeft->unbindFramebuffer();
	mRenderTargetRight->unbindFramebuffer();

/*
	submitFrame();
	updatePoseData(); 

	const auto& pose = mContext->getPose( ::vr::k_unTrackedDeviceIndex_Hmd );
	if( pose.bPoseIsValid ) {
		updateElapsedFrames();
	}
*/
}

void Hmd::submitFrame()
{
	// Left eye
	{
		GLuint resolvedTexId = mRenderTargetLeft->getColorTexture()->getId();
		::vr::Texture_t eyeTex = { reinterpret_cast<void*>( resolvedTexId ), ::vr::API_OpenGL, ::vr::ColorSpace_Gamma };
		::vr::VRCompositor()->Submit( ::vr::Eye_Left, &eyeTex );
	}

	// Right eye
	{
		GLuint resolvedTexId = mRenderTargetRight->getColorTexture()->getId();
		::vr::Texture_t eyeTex = { reinterpret_cast<void*>( resolvedTexId ), ::vr::API_OpenGL, ::vr::ColorSpace_Gamma };
		::vr::VRCompositor()->Submit( ::vr::Eye_Right, &eyeTex );
	}

	{
		// Note from OpenVR sample:
		//
		// HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		//
		glFinish();
	}


	// Update pose data
	{
		updatePoseData(); 

		const auto& pose = mContext->getPose( ::vr::k_unTrackedDeviceIndex_Hmd );
		if( pose.bPoseIsValid ) {
			updateElapsedFrames();
		}
	}
}

float Hmd::getFullFov() const
{
	return kFullFov;
}

ci::Area Hmd::getEyeViewport( ci::vr::Eye eye ) const
{
	auto size = mRenderTargetSize;
	if( ci::vr::EYE_LEFT == eye ) {
		return Area( 0, 0, size.x / 2, size.y );
	}
	return Area( ( size.x + 1 ) / 2, 0, size.x, size.y );
}

void Hmd::enableEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode )
{
	switch( eye ) {
		case ci::vr::EYE_LEFT: {
			mRenderTargetLeft->bindFramebuffer();
			ci::gl::viewport( mRenderTargetLeft->getSize() );
			ci::gl::clear( mClearColor );
		}
		break;

		case ci::vr::EYE_RIGHT: {
			mRenderTargetRight->bindFramebuffer();
			ci::gl::viewport( mRenderTargetRight->getSize() );
			ci::gl::clear( mClearColor );
		}
		break;

		case ci::vr::EYE_HMD: {
			auto viewport = ci::gl::getViewport();
			auto area = ci::Area( viewport.first.x, viewport.first.y, viewport.first.x + viewport.second.x, viewport.first.y + viewport.second.y );
			float width = area.getWidth();
			float height = area.getHeight();
			float aspect = width / height;
			ci::mat4 mat = glm::perspectiveFov( toRadians( getFullFov() / aspect ), width, height, mNearClip, mFarClip );
			mHmdCamera.setProjectionMatrix( mat );
			ci::gl::clear( mClearColor );
		}
		break;
	}

	setMatricesEye( eye, eyeMatrixMode );
}

void Hmd::calculateOriginMatrix()
{
	// Rotation matrix
	const ci::mat4& hmdDeviceToTrackingMat = mContext->getDeviceToTrackingMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
	ci::vec3 p0 = ci::vec3( hmdDeviceToTrackingMat * ci::vec4( 0, 0, 0, 1 ) );
	ci::vec3 p1 = ci::vec3( hmdDeviceToTrackingMat * ci::vec4( 0, 0, -1, 1 ) );
	ci::vec3 dir = p1 - p0;
	ci::vec3 v0 = ci::vec3( 0, 0, -1 );
	ci::vec3 v1 = ci::normalize( ci::vec3( dir.x, 0, dir.z ) );
	ci::quat q = ci::quat( v0, v1 );
	ci::mat4 rotationMatrix = glm::mat4_cast( q );
	// Position matrix
	//float dist = getSessionOptions().getOriginOffset();
	//ci::vec3 offset = p0 + v1*dist;
	//ci::mat4 positionMatrix = ci::translate( offset );
	const ci::vec3& offset = getSessionOptions().getOriginOffset();
	ci::vec3 w = v1;
	ci::vec3 v = ci::vec3( 0, 1, 0 );
	ci::vec3 u = ci::cross( w, v );
	ci::mat4 positionMatrix = ci::translate( p0 + ( offset.x * u ) + ( offset.y * v ) + ( -offset.z * w ) );

	switch( getSessionOptions().getOriginMode() ) {
		case ci::vr::ORIGIN_MODE_OFFSETTED: {
			// Rotation matrix
			rotationMatrix = ci::mat4();
			// Position matrix
			mOriginPosition = offset;
			positionMatrix = ci::translate( mOriginPosition );
		}
		break;

		case ci::vr::ORIGIN_MODE_HMD_OFFSETTED: {
			// Rotation matrix
			rotationMatrix = ci::mat4();
			// Position matrix
			mOriginPosition = ci::vec3( p0.z ) + offset;
			positionMatrix = ci::translate( mOriginPosition );
		}
		break;

		case ci::vr::ORIGIN_MODE_HMD_ORIENTED: {
			// Uses the current rotationMatrix and positionMatrix values.
		}
		break;

		default: {
			rotationMatrix = ci::mat4();
			positionMatrix = ci::mat4();
		}
		break;
	}

	// Compose origin matrix
	mOriginMatrix = positionMatrix*rotationMatrix;
	mInverseOriginMatrix = glm::affineInverse( mOriginMatrix );
}

void Hmd::calculateInputRay()
{
	// Ray components
	ci::mat4 deviceToTrackingMatrix = mContext->getDeviceToTrackingMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
	ci::mat4 coordSysMatrix = mInverseLookMatrix * mInverseOriginMatrix * deviceToTrackingMatrix;
	ci::vec3 p0 = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, 0, 1 ) );
	ci::vec3 dir = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, -1, 0 ) );
	// Input ray
	mInputRay = ci::Ray( p0, dir );
}

void Hmd::drawMirroredImpl( const ci::Rectf& r )
{
	const uint32_t kTexUnit = 0;

	ci::gl::ScopedDepthTest scopedDepthTest( false );
	ci::gl::ScopedModelMatrix scopedModelMatrix;
	ci::gl::ScopedColor scopedColor( 1, 1, 1 );

	switch( mMirrorMode ) {
		// Default to stereo mirroring
		default:
		case Hmd::MirrorMode::MIRROR_MODE_STEREO: {
			ci::gl::ScopedGlslProg scopedShader( mDistortionShader );

			float w = r.getWidth() / 2.0f;
			float h = r.getHeight() / 2.0f;

			// Offset and the scale to fit the rect
			ci::mat4 m = ci::mat4();
			m[0][0] =  w;
			m[1][1] = -h;
			m[3][0] =  w + r.x1;
			m[3][1] =  h + r.y1;
			ci::gl::multModelMatrix( m );

			// Render left eye
			{
				auto resolvedTex = mRenderTargetLeft->getColorTexture();
				resolvedTex->bind( kTexUnit );
				mDistortionShader->uniform( "uTex0", kTexUnit );
				mDistortionBatch->draw( 0, mDistortionIndexCount / 2 );
				resolvedTex->unbind( kTexUnit );
			}

			// Render right eye
			{
				auto resolvedTex = mRenderTargetRight->getColorTexture();
				resolvedTex->bind( kTexUnit );
				mDistortionShader->uniform( "uTex0", kTexUnit );
				mDistortionBatch->draw( mDistortionIndexCount / 2, mDistortionIndexCount / 2 );
				resolvedTex->unbind( kTexUnit );
			}
		}
		break;

		case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_STEREO: {
			float width = r.getWidth();
			float height = r.getHeight();
			ci::Rectf fittedRect = ci::Rectf( 0, 0, width / 2.0f, height );

			// Render left eye
			{
				auto resolvedTex = mRenderTargetLeft->getColorTexture();
				ci::gl::draw( resolvedTex, fittedRect );
			}

			// Render right eye
			{
				fittedRect += vec2( width / 2.0f, 0 );
				auto resolvedTex = mRenderTargetRight->getColorTexture();
				ci::gl::draw( resolvedTex, fittedRect );
			}
		}
		break;

		case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_LEFT: {
			auto tex = mRenderTargetLeft->getColorTexture();
			float width = static_cast<float>( tex->getWidth() );
			float height = static_cast<float>( tex->getHeight() );
			auto texRect = ci::Rectf( 0, 0, width, height );
			auto fittedRect = r.getCenteredFit( texRect, true );
			ci::gl::draw( tex, Area( fittedRect ), r );	
		}
		break;

		case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_RIGHT: {
			auto tex = mRenderTargetRight->getColorTexture();
			float width = static_cast<float>( tex->getWidth() );
			float height = static_cast<float>( tex->getHeight() );
			auto texRect = ci::Rectf( 0, 0, width, height );
			auto fittedRect = r.getCenteredFit( texRect, true );
			ci::gl::draw( tex, Area( fittedRect ), r );
		}
		break;
	}
}

void Hmd::drawControllers( ci::vr::Eye eye )
{
	if( mVrSystem->IsInputFocusCapturedByAnotherProcess() ) {
		return;
	}

	if( mControllerVertexCount > 0 ) {
		ci::gl::ScopedGlslProg scopedShader( mControllerShader );
		ci::mat4 vpMat = getEyeViewProjectionMatrix( eye );
		mControllerShader->uniform( "uMatrix", vpMat );
		mControllerBatch->draw( 0, static_cast<GLsizei>( mControllerVertexCount ) );
	}

	{
		ci::gl::ScopedGlslProg scopedShader( mRenderModelShader );

		for( uint32_t deviceIndex = 0; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex ) {
			if( ! mRenderModels[deviceIndex] ) {
				continue;
			}

			const ::vr::TrackedDevicePose_t& pose = mContext->getPose( deviceIndex );
			if( ! pose.bPoseIsValid ) {
				continue;
			}

			const ci::mat4& hmdTrackingToDeviceMat = mContext->getTrackingToDeviceMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
			const ci::mat4& ctrlDeviceToTrackingMat = mContext->getDeviceToTrackingMatrix( deviceIndex );
			//ci::mat4 matMVP = mEyeProjectionMatrix[eye] * mEyePoseMatrix[eye] * hmdTrackingToDeviceMat * ctrlDeviceToTrackingMat;
			ci::mat4 vpMat = getEyeViewProjectionMatrix( eye );
			ci::mat4 mvpMat = vpMat * ctrlDeviceToTrackingMat;
			mRenderModelShader->uniform( "uMatrix", mvpMat );
			mRenderModelShader->uniform( "uTex0", 0 );

			auto renderModel = mRenderModels[deviceIndex];
			renderModel->draw();

			ci::vr::Controller::Type ctrlType = ci::vr::Controller::TYPE_UNKNOWN;
			::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
			switch( role ) {
				case ::vr::TrackedControllerRole_LeftHand  : ctrlType = ci::vr::Controller::TYPE_LEFT; break; 
				case ::vr::TrackedControllerRole_RightHand : ctrlType = ci::vr::Controller::TYPE_RIGHT; break;
			}

			if( ci::vr::Controller::TYPE_UNKNOWN != ctrlType ) {
				ci::gl::ScopedBlendAlpha scopedBlend;
				ci::gl::ScopedMatrices ScopedMatrices;
				ci::gl::setProjectionMatrix( mEyeProjectionMatrix[eye] );
				ci::gl::setViewMatrix( mEyePoseMatrix[eye] * hmdTrackingToDeviceMat );
				ci::gl::setModelMatrix( ctrlDeviceToTrackingMat );
				ci::gl::translate( 0, -0.002f, 0.145f );
				ci::gl::rotate( 0.11f, 1.0f, 0.0f, 0.0f );
				ci::gl::scale( ci::vec3( 0.01f ) );
				const auto& tex = mContext->getControllerIconTexture( ctrlType );
				tex->bind( 0 );
				mControllerIconBatch[ctrlType]->getGlslProg()->uniform( "uTex0", 0 );
				mControllerIconBatch[ctrlType]->draw();
				tex->unbind( 0 );
			}
		}
	}
}

void Hmd::drawDebugInfo()
{
}

void Hmd::activateRenderModel( ::vr::TrackedDeviceIndex_t trackedDeviceIndex )
{
	// Bail if there's an existing model for trackedDeviceIndex.
	if( mRenderModels[trackedDeviceIndex] ) {
		return;
	}

	ci::vr::openvr::DeviceManager* deviceManager = mContext->getDeviceManager();
	std::string renderModelName = ci::vr::openvr::getTrackedDeviceString( mVrSystem, trackedDeviceIndex, ::vr::Prop_RenderModelName_String );
	if( ! renderModelName.empty() ) {
		RenderModelDataRef renderModelData = deviceManager->getRenderModelData( renderModelName );
		if( renderModelData ) {
			RenderModelRef renderModel = RenderModel::create( renderModelData, mRenderModelShader );
			mRenderModels[trackedDeviceIndex] = renderModel;
		}

		CI_LOG_I( "...added: " << renderModelName );
	}
}

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )