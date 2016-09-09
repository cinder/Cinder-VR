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

#include "cinder/vr/oculus/Hmd.h"
#include "cinder/vr/oculus/Context.h"
#include "cinder/vr/oculus/Oculus.h"
//
#include "cinder/app/App.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

namespace cinder { namespace vr { namespace oculus {

const float kFullFov = 110.0f; // Degrees

Hmd::Hmd( ci::vr::oculus::Context *context )
	: ci::vr::Hmd( context ), mContext( context )
{	
	mNearClip = context->getSessionOptions().getNearClip();
	mFarClip = context->getSessionOptions().getFarClip();

/*
	ovrGraphicsLuid luid; // Not used in OpenGL
	ci::vr::oculus::VERIFY( ::ovr_Create( &mSession, &luid ) );
*/

	mSession = mContext->getSession();
	mHmdDesc = ::ovr_GetHmdDesc( mSession );

	for( int i = 0; i < ovrEye_Count; ++i ) {
		mEyeRenderDesc[i] = ::ovr_GetRenderDesc( mSession, (ovrEyeType)i, mHmdDesc.DefaultEyeFov[i] );
	}

	initializeRenderTarget();
	onMonoscopicChange();
	app::getWindow()->getSignalResize().connect( [this](){
		initializeMirrorTexture( app::getWindowSize() );
	} );

	//ovr_SetTrackingOriginType( mSession, ovrTrackingOrigin_EyeLevel );

	if( app::App::get()->isFrameRateEnabled() ) {
		CI_LOG_I( "Disabled framerate for better performance." );
		app::App::get()->disableFrameRate();
	}

	if( gl::isVerticalSyncEnabled() ) {
		CI_LOG_I( "Disabled vertical sync: handled by compositor service." );
		gl::enableVerticalSync( false );
	}

	// Tracking origin
	switch( getSessionOptions().getTrackingOrigin() ) {
		case ci::vr::TRACKING_ORIGIN_SEATED: {
			::ovr_SetTrackingOriginType( mSession, ::ovrTrackingOrigin_EyeLevel );
		}
		break;
		case ci::vr::TRACKING_ORIGIN_STANDING: {
			::ovr_SetTrackingOriginType( mSession, ::ovrTrackingOrigin_FloorLevel );
		}
		break;
	}
}

Hmd::~Hmd()
{
}

ci::vr::oculus::HmdRef Hmd::create( ci::vr::oculus::Context *context )
{
	ci::vr::oculus::HmdRef result = ci::vr::oculus::HmdRef( new ci::vr::oculus::Hmd( context ) );
	return result;
}

void Hmd::initializeRenderTarget()
{
	mRenderTargetSize = ci::ivec2( mScreenPercentage * ci::vec2( fromOvr( mHmdDesc.Resolution ) ) );

    ::ovrTextureSwapChainDesc desc = {};
    desc.Type			= ovrTexture_2D;
    desc.Format			= OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.ArraySize		= 1;
    desc.Width			= mRenderTargetSize.x;
    desc.Height			= mRenderTargetSize.y;
    desc.MipLevels		= getSessionOptions().getMipLevels();
    desc.SampleCount	= getSessionOptions().getSampleCount();
    desc.StaticImage	= ovrFalse;

    ovrResult result = ::ovr_CreateTextureSwapChainGL( mSession, &desc, &mTextureSwapChain );
	if( ! OVR_SUCCESS( result ) ) {
		throw ci::vr::oculus::Exception( "Couldn't create texture swapchain" );
	}

    int swapChainBufferCount = 0;
	result = ::ovr_GetTextureSwapChainLength( mSession, mTextureSwapChain, &swapChainBufferCount );
	if( ! OVR_SUCCESS( result ) ) {
		throw ci::vr::oculus::Exception( "Couldn't get texture swapchain buffer count" );
	}
	CI_LOG_I( "Swapchain buffer count: " << swapChainBufferCount );

	// Shared depth attachment
	ci::gl::Texture::Format depthFmt = ci::gl::Texture::Format();
	depthFmt.setInternalFormat( GL_DEPTH_COMPONENT24 );
	depthFmt.setMinFilter( GL_LINEAR );
	depthFmt.setMagFilter( GL_LINEAR );
	depthFmt.setWrapS( GL_CLAMP_TO_EDGE );
	depthFmt.setWrapT( GL_CLAMP_TO_EDGE );
	// Create texture
	ci::gl::TextureRef depthAttachment = ci::gl::Texture::create( mRenderTargetSize.x, mRenderTargetSize.y, depthFmt );

	// Create render targets corresponding to swapchain buffer count
	for( int i = 0; i < swapChainBufferCount; ++i ) {
		GLuint texId = 0;
		::ovr_GetTextureSwapChainBufferGL( mSession, mTextureSwapChain, i, &texId );
		// Create texture reference
		ci::gl::TextureRef colorAttachment = ci::gl::Texture::create( GL_TEXTURE_2D, texId, mRenderTargetSize.x, mRenderTargetSize.y, true );
		colorAttachment->bind();
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		colorAttachment->unbind();
		// Render target format
		ci::gl::Fbo::Format fboFmt = ci::gl::Fbo::Format();
		fboFmt.attachment( GL_COLOR_ATTACHMENT0, colorAttachment );
		fboFmt.attachment( GL_DEPTH_ATTACHMENT, depthAttachment );
		// Create render target
		ci::gl::FboRef renderTarget = ci::gl::Fbo::create( mRenderTargetSize.x, mRenderTargetSize.y, fboFmt );
		mRenderTargets.push_back( renderTarget );
	}
}

void Hmd::destroyRenderTarget()
{
	mRenderTargets.clear();

	if( nullptr != mTextureSwapChain ) {
		::ovr_DestroyTextureSwapChain( mSession, mTextureSwapChain );
		mTextureSwapChain = nullptr;
	}
}

void Hmd::initializeMirrorTexture( const glm::ivec2& size )
{
	if( mMirrorTexture ) {
		destroyMirrorTexture();
	}

	ovrMirrorTextureDesc desc = {};
	desc.Width  = size.x;
	desc.Height = size.y;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;


	ovrResult result = ::ovr_CreateMirrorTextureGL( mSession, &desc, &mMirrorTexture );
	if( ! OVR_SUCCESS( result ) ) {
		throw ci::vr::oculus::Exception( "Couldn't initialize mirror texture" );
	}

	GLuint mirrorTextureId = 0;
	::ovr_GetMirrorTextureBufferGL( mSession, mMirrorTexture, &mirrorTextureId );
	ci::gl::TextureRef attachment = ci::gl::Texture::create( GL_TEXTURE_2D, mirrorTextureId, size.x, size.y, true );

	ci::gl::Fbo::Format fboFmt = ci::gl::Fbo::Format();
	fboFmt.attachment( GL_COLOR_ATTACHMENT0, attachment );
	fboFmt.disableDepth();
	mMirrorFbo = ci::gl::Fbo::create( size.x, size.y, fboFmt );
}

void Hmd::destroyMirrorTexture()
{
	if( mMirrorFbo ) {
		mMirrorFbo.reset();
	}

	if( mMirrorTexture ) {
		::ovr_DestroyMirrorTexture( mSession, mMirrorTexture );
	}
}

void Hmd::onClipValueChange( float nearClip, float farClip )
{
	mNearClip = nearClip;
	mFarClip = farClip;
}

void Hmd::onMonoscopicChange()
{
	if( isMonoscopic() ) {
		auto centerEyeOffset = 0.5f * ( fromOvr( mEyeRenderDesc[0].HmdToEyeOffset ) + fromOvr( mEyeRenderDesc[1].HmdToEyeOffset ) );
		mEyeViewOffset[0] = toOvr( centerEyeOffset );
		mEyeViewOffset[1] = toOvr( centerEyeOffset );
	}
	else {
		mEyeViewOffset[0] = mEyeRenderDesc[0].HmdToEyeOffset;
		mEyeViewOffset[1] = mEyeRenderDesc[1].HmdToEyeOffset;
	}
}

void Hmd::recenterTrackingOrigin()
{
	::ovr_RecenterTrackingOrigin( mSession );
}

void Hmd::bind()
{
	// Update matrices based on pose data
	{
		::ovrTrackingState trackingState = ::ovr_GetTrackingState( mSession, 0.0, ovrFalse );
		// Calculate device to tracking matrix
		mDeviceToTrackingMatrix = ci::vr::oculus::fromOvr( trackingState.HeadPose.ThePose );
		// Calculate tracking to device matrix
		mTrackingToDeviceMatrix = ci::inverse( mDeviceToTrackingMatrix );

		// Update HMD camera
		{
			const auto& hmdPose = trackingState.HeadPose.ThePose;
			// View matrix
			ci::vec3 position = ci::vr::oculus::fromOvr( hmdPose.Position );
			ci::quat orientation = ci::vr::oculus::fromOvr( hmdPose.Orientation );
			ci::mat4 rotMat = glm::mat4_cast( orientation );
			ci::mat4 posMat = glm::translate( position );
			ci::mat4 viewMatrix = posMat*rotMat;
			viewMatrix = ci::inverse( viewMatrix );
			mHmdCamera.setViewMatrix( viewMatrix );
			// Projection matrix will be set in enableEye
		}
	}
	
	// Calculate origin matrix
	if( ( ! mOriginInitialized ) && mIsVisible ) {
		calculateOriginMatrix();
		// Flag as initialized		
		mOriginInitialized = true;
	}

	// Calculate input ray
	calculateInputRay();

	::ovr_GetEyePoses( mSession, mFrameIndex, ovrTrue, mEyeViewOffset, mEyeRenderPose, &mSensorSampleTime );
	const ::ovrEyeType kEyes[2] = { ::ovrEye_Left, ::ovrEye_Right };
	for( auto eye : kEyes ) {
		// View matrix
		ci::vec3 position = ci::vr::oculus::fromOvr( mEyeRenderPose[eye].Position );
		ci::quat orientation = ci::vr::oculus::fromOvr( mEyeRenderPose[eye].Orientation );
		ci::mat4 rotMat = glm::mat4_cast( orientation );
		ci::mat4 posMat = glm::translate( position );
		ci::mat4 viewMatrix = posMat*rotMat;
		viewMatrix = ci::inverse( viewMatrix );
		mEyeCamera[eye].setViewMatrix( viewMatrix );
		// Projection matrix
		ci::mat4 projectionMatrix = ci::vr::oculus::fromOvr( ::ovrMatrix4f_Projection( mEyeRenderDesc[eye].Fov, mNearClip, mFarClip, ::ovrProjection_None ) );
		mEyeCamera[eye].setProjectionMatrix( projectionMatrix );
	}

	if( mTextureSwapChain && ( ! mRenderTargets.empty() ) && mIsVisible ) {
		// Get current swapchain index
		::ovr_GetTextureSwapChainCurrentIndex( mSession, mTextureSwapChain, &mCurrentSwapChainIndex );
		// Find the corresponding render target and bind it
		auto& renderTarget = mRenderTargets[static_cast<size_t>( mCurrentSwapChainIndex )];
		renderTarget->bindFramebuffer();
		// Clear it
		ci::gl::ScopedViewport scopedViewPort( mRenderTargetSize );
		ci::gl::clear( mClearColor );
	}
}

void Hmd::unbind()
{
	if( mTextureSwapChain && ( ! mRenderTargets.empty() ) && mIsVisible && ( -1 != mCurrentSwapChainIndex ) ) {
		// Unbind current render target
		auto& renderTarget = mRenderTargets[static_cast<size_t>( mCurrentSwapChainIndex )];
		renderTarget->unbindFramebuffer();
		// Commit swapchain
		::ovr_CommitTextureSwapChain( mSession, mTextureSwapChain );
	}

	/*
	submitFrame();

	if( mIsVisible ) {
		updateElapsedFrames();
	}
	*/
}

void Hmd::submitFrame()
{
	// Set up positional data.
	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

	mBaseLayer.Header.Type = ovrLayerType_EyeFov;
	mBaseLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
	mBaseLayer.SensorSampleTime = mSensorSampleTime;
	for( auto eye : getEyes() ) {
		auto area = getEyeViewport( eye );
		viewScaleDesc.HmdToEyeOffset[eye]	= mEyeViewOffset[eye];
		mBaseLayer.Fov[eye]					= mEyeRenderDesc[eye].Fov;
		mBaseLayer.RenderPose[eye]			= mEyeRenderPose[eye];
		mBaseLayer.Viewport[eye]			= { { area.x1, area.y1 }, { area.getWidth(), area.getHeight() } };
	}
	mBaseLayer.ColorTexture[0] = mTextureSwapChain;
	mBaseLayer.ColorTexture[1] = NULL;
	
	ovrLayerHeader* layers = &mBaseLayer.Header;
	auto result = ::ovr_SubmitFrame( mSession, mFrameIndex, &viewScaleDesc, &layers, 1 );
	mIsVisible = ( result == ovrSuccess );

	++mFrameIndex;

	// Update frame index
	if( mIsVisible ) {
		updateElapsedFrames();
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
	else if( ci::vr::EYE_RIGHT == eye ) {
		return Area( ( size.x + 1 ) / 2, 0, size.x, size.y );
	}
	return Area( 0, 0, 0, 0 );
}

void Hmd::enableEye( ci::vr::Eye eye, ci::vr::CoordSys eyeMatrixMode )
{
	ci::Area area = getEyeViewport( eye );
	if( ci::vr::EYE_HMD == eye ) {
		auto viewport = ci::gl::getViewport();
		area = ci::Area( viewport.first.x, viewport.first.y, viewport.first.x + viewport.second.x, viewport.first.y + viewport.second.y );
		float width = static_cast<float>( area.getWidth() );
		float height = static_cast<float>( area.getHeight() );
		float aspect = width / height;
		ci::mat4 mat = glm::perspectiveFov( toRadians( getFullFov() / aspect ), width, height, mNearClip, mFarClip );
		mHmdCamera.setProjectionMatrix( mat );
	}
	ci::gl::viewport( area.getUL(), area.getSize() );

	setMatricesEye( eye, eyeMatrixMode );
}

void Hmd::calculateOriginMatrix()
{
	// Obtain tracking state
	::ovrTrackingState trackingState = ::ovr_GetTrackingState( mSession, 0.0, ovrFalse );
	// Build transform matrix
	ci::vec3 position = ci::vr::oculus::fromOvr( trackingState.HeadPose.ThePose.Position );
	ci::quat orientation = ci::vr::oculus::fromOvr( trackingState.HeadPose.ThePose.Orientation );
	ci::mat4 rotationMatrix = glm::mat4_cast( orientation );
	ci::mat4 positionMatrix = glm::translate( position );
	ci::mat4 deviceToTrackingMatrix = positionMatrix*rotationMatrix; 
	// Rotation matrix
	ci::vec3 p0 = ci::vec3( mDeviceToTrackingMatrix * ci::vec4( 0, 0, 0, 1 ) );
	ci::vec3 p1 = ci::vec3( mDeviceToTrackingMatrix * ci::vec4( 0, 0, -1, 1 ) );
	ci::vec3 dir = p1 - p0;
	ci::vec3 v0 = ci::vec3( 0, 0, -1 );
	ci::vec3 v1 = ci::normalize( ci::vec3( dir.x, 0, dir.z ) );
	ci::quat q = ci::quat( v0, v1 );
	rotationMatrix = glm::mat4_cast( q );
	// Position matrix
	const ci::vec3& offset = getSessionOptions().getOriginOffset();
	ci::vec3 w = v1;
	ci::vec3 v = ci::vec3( 0, 1, 0 );
	ci::vec3 u = ci::cross( w, v );
	positionMatrix = ci::translate( p0 + ( offset.x * u ) + ( offset.y * v ) + ( -offset.z * w ) );

	switch( getSessionOptions().getOriginMode() ) {
		case ci::vr::ORIGIN_MODE_OFFSETTED: {
			rotationMatrix = ci::mat4();
			//positionMatrix = ci::translate( ci::vec3( 0, 0, -dist ) );
			mOriginPosition = offset;
			positionMatrix = ci::translate( mOriginPosition );
		}
		break;

		case ci::vr::ORIGIN_MODE_HMD_OFFSETTED: {
			rotationMatrix = ci::mat4();
			//positionMatrix = ci::translate( ci::vec3( 0, 0, p0.z + -dist ) );
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
	ci::mat4 coordSysMatrix = mInverseLookMatrix * mInverseOriginMatrix * mDeviceToTrackingMatrix;
	ci::vec3 p0 = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, 0, 1 ) );
	ci::vec3 dir = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, -1, 0 ) );
	// Input ray
	mInputRay = ci::Ray( p0, dir );
}

void Hmd::drawMirroredImpl( const ci::Rectf& r )
{
	if( isMirrored() && mMirrorTexture ) {
		ci::gl::ScopedDepth scopedDepth( false );
		ci::gl::ScopedColor scopedColor( 1, 1, 1 );
		ci::gl::ScopedModelMatrix scopedModelMatrix;
		switch( mMirrorMode ) {
			// Default to stereo mirroring
			default:
			case Hmd::MirrorMode::MIRROR_MODE_STEREO: {
				ci::gl::translate( 0.0f, r.getHeight(), 0.0f );
				ci::gl::scale( 1, -1 );
				ci::gl::draw( mMirrorFbo->getColorTexture(), r );
			}
			break;			

			case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_STEREO: {
				auto tex = mRenderTargets[static_cast<size_t>( mCurrentSwapChainIndex )]->getColorTexture();
				auto fittedRect = ci::Rectf( tex->getBounds() ).getCenteredFit( r, true );
				ci::gl::draw( tex, fittedRect );
			}
			break;

			case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_LEFT: {
				auto tex = mRenderTargets[static_cast<size_t>( mCurrentSwapChainIndex )]->getColorTexture();
				float width = static_cast<float>( tex->getWidth() ) / 2.0f;
				float height = static_cast<float>( tex->getHeight() );
				auto texRect = ci::Rectf( 0, 0, width, height );
				auto fittedRect = r.getCenteredFit( texRect, true );
				ci::gl::draw( tex, Area( fittedRect ), r );
			}
			break;

			case Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_RIGHT: {
				auto tex = mRenderTargets[static_cast<size_t>( mCurrentSwapChainIndex )]->getColorTexture();
				float width = static_cast<float>( tex->getWidth() ) / 2.0f;
				float height = static_cast<float>( tex->getHeight() );
				auto texRect = ci::Rectf( 0, 0, width, height );
				texRect += ci::vec2( width, 0.0f );
				auto fittedRect = r.getCenteredFit( texRect, true );
				ci::gl::draw( tex, Area( fittedRect ), r );
			}
			break;
		}
	}
}

void Hmd::drawControllers( ci::vr::Eye eyeType )
{
}

void Hmd::drawDebugInfo()
{
}

glm::vec3 Hmd::getLatencies() const
{
	float latencies[3] = { 0.0f, 0.0f, 0.0f };
	if( ::ovr_GetFloatArray( mSession, "DK2Latency", latencies, 3 ) == 3 ) {
		return 1000.0f * glm::vec3( latencies[0], latencies[1], latencies[2] );
	}
	return glm::vec3( 0 );
}

void Hmd::setScreenPercentage( float sp )
{
	CI_ASSERT( sp > 0.0f );
	mScreenPercentage = sp;
}

void Hmd::cyclePerfHudModes( bool enabled )
{
	mPerfHudMode = ( enabled ) ? (++mPerfHudMode) % int(ovrPerfHud_Count) : 0;
	::ovr_SetInt( mSession, OVR_PERF_HUD_MODE, mPerfHudMode );
}

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )