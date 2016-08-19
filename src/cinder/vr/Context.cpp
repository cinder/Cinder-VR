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

#include "cinder/vr/Context.h"
#include "cinder/vr/Controller.h"
#include "cinder/vr/DeviceManager.h"
#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"

#include "IconLeftHand.h"
#include "IconRightHand.h"

namespace cinder { namespace vr {

Context::Context( const ci::vr::SessionOptions& sessionOptions, ci::vr::DeviceManager* deviceManager )
	: mSessionOptions( sessionOptions ), mDeviceManager( deviceManager )
{
	mControllerIconTextures[ci::vr::Controller::TYPE_LEFT] = ci::gl::Texture2d::create( getHandIcon( ci::vr::Controller::TYPE_LEFT ) );
	mControllerIconTextures[ci::vr::Controller::TYPE_RIGHT] = ci::gl::Texture2d::create( getHandIcon( ci::vr::Controller::TYPE_RIGHT ) );

	if( mSessionOptions.getControllerConnected() ) {
		mSignalControllerConnected.connect( mSessionOptions.getControllerConnected() );
	}

	if( mSessionOptions.getControllerDisconnected() ) {
		mSignalControllerDisconnected.connect( mSessionOptions.getControllerDisconnected() );
	}

	ci::app::App::get()->getSignalUpdate().connect( std::bind( &Context::update, this ) );
}

Context::~Context()
{
}

ci::vr::Api Context::getApi() const
{
	return mDeviceManager->getApi();
}

ci::vr::Hmd* Context::getHmd() const
{
	ci::vr::Hmd* result = mHmd ? mHmd.get() : nullptr;
	return result;
}

bool Context::hasController( const ci::vr::Controller::Type type ) const
{
	auto it = std::find_if( std::begin( mControllers ), std::end( mControllers ),
		[type]( const ci::vr::ControllerRef& elem ) -> bool {
			return type == elem->getType();
		}
	);

	bool result = ( std::end( mControllers ) != it );
	return result;
}

ci::vr::Controller* Context::getController( ci::vr::Controller::Type type ) const
{
	ci::vr::Controller* result = nullptr;
	auto it = std::find_if( std::begin( mControllers ), std::end( mControllers ),
		[type]( const ci::vr::ControllerRef& elem ) -> bool {
			return type == elem->getType();
		}
	);
	if( std::end ( mControllers ) != it ) {
		result = it->get();
	}
	return result;
}

ci::gl::Texture2dRef Context::getControllerIconTexture( ci::vr::Controller::Type type ) const
{
	ci::gl::Texture2dRef result;
	auto it = mControllerIconTextures.find( type );
	if( mControllerIconTextures.end() != it ) {
		result = it->second;
	}
	return result;
}

void Context::update()
{
	double currentTime = ci::app::getElapsedSeconds();

	// Scan for controllers
	{
		double dt = currentTime - mPrevControllersScanTime;
		double interval = mSessionOptions.getControllersScanInterval();
		if( ( interval > 0.0 ) && ( dt >= interval ) ) {
			scanForControllers();
			mPrevControllersScanTime = currentTime;
		}
	}

	processEvents();
}

void Context::addController( const ci::vr::ControllerRef& controller )
{
	if( ! controller ) {
		return;
	}

	ci::vr::Controller::Type type = controller->getType();
	auto it = std::find_if( std::begin( mControllers ), std::end( mControllers ),
		[type]( const ci::vr::ControllerRef& elem ) -> bool {
			return type == elem->getType();
		}
	);

	if( std::end( mControllers ) == it ) {
		mControllers.push_back( controller );
		CI_LOG_D( "CONTROLLER FOUND: " << controller->getName() );
		mSignalControllerConnected.emit( controller.get() );
	}
}

void Context::removeController( const ci::vr::ControllerRef& controller )
{
	if( ! controller ) {
		return;
	}

	mControllers.erase(
		std::remove_if( std::begin( mControllers ), std::end( mControllers ),
			[controller]( const ci::vr::ControllerRef& elem ) -> bool {
				return controller == elem;
			}
		),
		std::end( mControllers )
	);
}

ci::Surface8u getHandIcon( ci::vr::Controller::Type type )
{
	ci::Surface8u result;
	switch( type ) {
		case ci::vr::Controller::TYPE_LEFT: {
			ci::BufferRef buffer = ci::Buffer::create( sIconLeftHandLength );
			buffer->copyFrom( reinterpret_cast<const void *>( &sIconLeftHand[0] ), sIconLeftHandLength );
			result = ci::loadImage( ci::DataSourceBuffer::create( buffer ) );
		}
		break;

		case ci::vr::Controller::TYPE_RIGHT: {
			ci::BufferRef buffer = ci::Buffer::create( sIconRightHandLength );
			buffer->copyFrom( reinterpret_cast<const void *>( &sIconRightHand[0] ), sIconRightHandLength );
			result = ci::loadImage( ci::DataSourceBuffer::create( buffer ) );
		}
		break;

		default: {
			throw ci::vr::Exception( "Invalid hand id" );
		}
		break;
	} 
	return result;
}

}} // namespace cinder::vr