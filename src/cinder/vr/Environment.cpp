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

#include "cinder/vr/Environment.h"
#include "cinder/vr/Context.h"
#include "cinder/vr/oculus/DeviceManager.h"
#include "cinder/vr/openvr/DeviceManager.h"
#include "cinder/Log.h"

#include <algorithm>
#include <string>

namespace cinder { namespace vr {

static std::unique_ptr<Environment> sEnvironment;

Environment::Environment()
{
}

Environment::~Environment()
{
	// Destroy session
	for( auto& elem : mSessions ) {

	}
	// Clear sessions
	mSessions.clear();

	// Destroy device managers
	for( auto& elem : mDeviceManagers ) {
		auto& store = elem.second;

		if( ! store->mOwnedByEnvironment ) {
			continue;
		}

		store->mManager->destroy();

		delete store->mManager;
		store->mManager = nullptr;
	}
	// Clear device managers
	mDeviceManagers.clear();
}

void Environment::registerDevice( ci::vr::ApiFlags deviceVendorId, ci::vr::DeviceManager* deviceManager, bool assumeOwnership )
{
	auto it = std::find_if( std::begin( mDeviceManagers ), std::end( mDeviceManagers ),
		[deviceVendorId]( const std::pair<ci::vr::ApiFlags, DeviceManagerStoreRef>& elem ) -> bool {
			return elem.first == deviceVendorId;
		}
	);

	if( std::end( mDeviceManagers ) != it ) {
		throw std::runtime_error( "Device already registered, deviceVendorId=" + std::to_string( deviceVendorId ) );
	}

	Environment::DeviceManagerStoreRef dsm = Environment::DeviceManagerStoreRef( new Environment::DeviceManagerStore() );
	dsm->mOwnedByEnvironment = assumeOwnership;
	dsm->mManager = deviceManager;

	mDeviceManagers.push_back( std::make_pair( deviceVendorId, dsm ) );
}

ci::vr::Context* Environment::beginSession( const ci::vr::SessionOptions& options, ci::vr::ApiFlags apiFlags, uint32_t deviceIndex )
{
	if( mDeviceManagers.empty() ) {
		throw ci::vr::Exception( "No device managers available" );
	}

	// Check for active session using device vendor id and device index
	auto sessionIt = std::find_if( std::begin( mSessions ), std::end( mSessions ),
		[apiFlags, deviceIndex]( const SessionStoreRef& elem ) -> bool {
			return ( elem->mApiFlags == apiFlags ) && ( elem->mDeviceIndex == deviceIndex );
		}
	);

	if( std::end( mSessions ) != sessionIt ) {
		throw std::runtime_error( "Device is already active in session, deviceVendorId=" + std::to_string( apiFlags ) + ", deviceIndex=" + std::to_string( deviceIndex ) );
	}

	// Get device manager
	auto deviceManagerIt = std::find_if( std::begin( mDeviceManagers ), std::end( mDeviceManagers ),
		[apiFlags]( const std::pair<ci::vr::ApiFlags, DeviceManagerStoreRef>& elem ) -> bool {
			return ( elem.first == ( apiFlags & elem.first ) );
		}
	);

	if( std::end( mDeviceManagers ) == deviceManagerIt ) {
		throw std::runtime_error( "Device manager not found for deviceVendorId=" + std::to_string( apiFlags ) );
	}

	auto& deviceManager = deviceManagerIt->second->mManager;

	// Create context for session
	ci::vr::ContextRef context = deviceManager->createContext( options, deviceIndex );
	
	// Create session
	SessionStoreRef session = SessionStoreRef( new SessionStore() );
	session->mApiFlags = apiFlags;
	session->mDeviceIndex = deviceIndex;
	session->mContext = context;
	mSessions.push_back( session );

	// Start session
	session->mContext->beginSession();

	// Return the context
	return context.get();
}

void Environment::endSession( ci::vr::Context* context )
{
	context->endSession();

	mSessions.erase(
		std::remove_if( mSessions.begin(), mSessions.end(),
			[context]( const SessionStoreRef& elem ) -> bool {
				return elem->mContext.get() == context;
			}
		),
		mSessions.end()
	);
}

void registerDevice( ci::vr::ApiFlags deviceVendorId, ci::vr::DeviceManager* deviceFactory, bool assumeOwnership )
{
	if( ! sEnvironment ) {
		sEnvironment.reset( new ci::vr::Environment() );
		sEnvironment->registerDevice( deviceVendorId, deviceFactory, assumeOwnership );
	}
}

void initialize( ci::vr::ApiFlags apiFlags )
{
	if( ! sEnvironment ) {
		sEnvironment.reset( new ci::vr::Environment() );

		// NOTE: If Oculus is present, then don't start OpenVR. OpenVR will attempt
		//       to launch SteamVR, which assumes control of the VR environment.
		//
		bool isOculusPresent = false;

#if defined( CINDER_VR_ENABLE_OCULUS )
		if( ci::vr::API_OCULUS == ( apiFlags & ci::vr::API_OCULUS ) ) {
			try {
				ci::vr::oculus::DeviceManager* deviceManager = new ci::vr::oculus::DeviceManager( sEnvironment.get() );
				deviceManager->initialize();

				sEnvironment->registerDevice( ci::vr::API_OCULUS, deviceManager, true );

				isOculusPresent = true;
			}
			catch( const std::exception& e ) {
				CI_LOG_W( "Oculus Rift device manager registration failed: " << e.what() );
			}
		}
#endif

#if defined( CINDER_VR_ENABLE_OPENVR )
		if( ( ! isOculusPresent ) && ( ci::vr::API_OPENVR == ( apiFlags & ci::vr::API_OPENVR ) ) ) {
			try {
				ci::vr::openvr::DeviceManager* deviceManager = new ci::vr::openvr::DeviceManager( sEnvironment.get() );
				deviceManager->initialize();

				sEnvironment->registerDevice( ci::vr::API_OPENVR, deviceManager, true );
			}
			catch( const std::exception& e ) {
				CI_LOG_W( "HTC Vive device manager registration failed: " << e.what() );
			}
		}
#endif
	}
}

void destroy()
{
	if( sEnvironment ) {
		sEnvironment.reset();
	}
}

ci::vr::Context* beginSession( const ci::vr::SessionOptions& options, ci::vr::ApiFlags apiFlags, uint32_t deviceIndex )
{
	ci::vr::Context* result = sEnvironment->beginSession( options, apiFlags, deviceIndex );
	return result;
}

void endSession(ci::vr::Context* context)
{
	sEnvironment->endSession( context );
}

}} // namespace cinder::vr