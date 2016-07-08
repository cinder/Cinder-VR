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

#include "cinder/vr/oculus/Context.h"
#include "cinder/vr/oculus/Controller.h"
#include "cinder/vr/oculus/DeviceManager.h"
#include "cinder/vr/oculus/Hmd.h"
#include "cinder/vr/oculus/Oculus.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

namespace cinder { namespace vr { namespace oculus {

Context::Context( const ci::vr::SessionOptions& sessionOptions, ci::vr::oculus::DeviceManager* deviceManager )
	: ci::vr::Context( sessionOptions, deviceManager )
{
}

Context::~Context()
{
	endSession();
}

ContextRef Context::create( const ci::vr::SessionOptions& sessionOptions, ci::vr::oculus::DeviceManager* deviceManager )
{
	ContextRef result = ContextRef( new Context( sessionOptions, deviceManager ) );
	return result;
}

void Context::scanForControllers()
{
	uint32_t ctrlTypeMask = ::ovr_GetConnectedControllerTypes( mSession );
	for( uint32_t i = 0; i < 5; ++i ) {
		uint32_t ctrlType = 1 << i;
		if( ctrlType == ( ctrlType & ctrlTypeMask ) ) {
			switch( ctrlType ) {
				case ::ovrControllerType_LTouch: {
					if( ! hasController( ci::vr::Controller::TYPE_LEFT ) ) {
						auto ctrl = ci::vr::oculus::ControllerTouch::create( ci::vr::Controller::TYPE_LEFT, this );
						addController( ctrl );
					}
				}
				break;

				case ::ovrControllerType_RTouch: {
					if( ! hasController( ci::vr::Controller::TYPE_RIGHT ) ) {
						auto ctrl = ci::vr::oculus::ControllerTouch::create( ci::vr::Controller::TYPE_RIGHT, this );
						addController( ctrl );
					}
				}
				break;

				case ::ovrControllerType_Remote: {
					if( ! hasController( ci::vr::Controller::TYPE_REMOTE ) ) {
						auto ctrl = ci::vr::oculus::ControllerRemote::create( this );
						addController( ctrl );
					}
				}
				break;

				case ::ovrControllerType_XBox: {
					if( ! hasController( ci::vr::Controller::TYPE_XBOX ) ) {
						auto ctrl = ci::vr::oculus::ControllerXbox::create( this );
						addController( ctrl );
					}
				}
				break;
			}
		}
	}
}

void Context::beginSession()
{
	if( nullptr != mSession ) {
		return;
	}

	ovrGraphicsLuid luid; // Not used in OpenGL
	checkResult( ::ovr_Create( &mSession, &luid ) );

	// Get connected controllers
	scanForControllers();
	
	// Create HMD
	mHmd = ci::vr::oculus::Hmd::create( this );

	// Set frame rate for VR
	ci::gl::enableVerticalSync( getSessionOptions().getVerticalSync() );
	ci::app::setFrameRate( getSessionOptions().getFrameRate() );
}

void Context::endSession()
{
	// Destroy HMD
	mHmd.reset();

	if( nullptr != mSession ) {
		::ovr_Destroy( mSession );
		mSession = nullptr;
	}
}

void Context::processEvents()
{
	for( auto& baseCtrl : mControllers ) {
		auto ctrl = std::dynamic_pointer_cast<ci::vr::oculus::Controller>( baseCtrl );
		::ovrControllerType ctrlType = ctrl->getInternalType();
		::ovrInputState inputState = {};
		auto result = ::ovr_GetInputState( mSession, ctrlType, &inputState );
		if( ::ovrSuccess != result ) {
			continue;
		}
		ctrl->processInputState( inputState );
	}
}

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )