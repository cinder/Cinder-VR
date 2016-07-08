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

#include "cinder/vr/SessionOptions.h"

#include <vector>

namespace cinder { namespace vr {

class Context;
class DeviceManager;
using ContextRef = std::shared_ptr<Context>;
using DeviceManagerRef = std::shared_ptr<DeviceManager>;

//! \class Environment
//!
//!
class Environment {
public:
	
	virtual ~Environment();

private:
	Environment();
	friend void					registerDevice( ci::vr::ApiFlags deviceVendorId, ci::vr::DeviceManager* deviceFactory, bool assumeOwnership );
	friend void					initialize( ci::vr::ApiFlags deviceVendor );
	friend ci::vr::Context*		beginSession( const ci::vr::SessionOptions& options, ci::vr::ApiFlags apiFlags, uint32_t deviceIndex );
	friend void					endSession( ci::vr::Context* context );

	// ---------------------------------------------------------------------------------------------
	// Device Managers
	// ---------------------------------------------------------------------------------------------
	struct DeviceManagerStore {
		bool					mOwnedByEnvironment = false;
		ci::vr::DeviceManager*	mManager = nullptr;
	};
	using DeviceManagerStoreRef = std::shared_ptr<DeviceManagerStore>;

	std::vector<std::pair<ci::vr::ApiFlags, DeviceManagerStoreRef>>	mDeviceManagers;
		
	void registerDevice( ci::vr::ApiFlags deviceVendorId, ci::vr::DeviceManager* deviceManager, bool assumeOwnership );

	// ---------------------------------------------------------------------------------------------
	// Sessions 
	// ---------------------------------------------------------------------------------------------
	struct SessionStore {
		ci::vr::ApiFlags		mApiFlags = ci::vr::API_UNKNOWN;
		uint32_t				mDeviceIndex = UINT32_MAX;
		ci::vr::ContextRef		mContext;
	};
	using SessionStoreRef = std::shared_ptr<SessionStore>;

	std::vector<SessionStoreRef>	mSessions;

	ci::vr::Context* beginSession( const ci::vr::SessionOptions& options, ci::vr::ApiFlags apiFlags, uint32_t deviceIndex );
	void endSession( ci::vr::Context* context );
};

//! Registers a vendor device with factor used to create device. Does not assume ownership of device manager.
void registerDevice( ci::vr::ApiFlags deviceVendorId, ci::vr::DeviceManager* deviceManager, bool assumeOwnership  );

//! Initialize vr environment using a device vendor id. Custom devices require explicit device vendor id.
void initialize( ci::vr::ApiFlags deviceVendorId = ci::vr::API_ANY );

//! Destroys vr environment
void destroy();

//! Start session
ci::vr::Context* beginSession( const ci::vr::SessionOptions& options = ci::vr::SessionOptions(), ci::vr::ApiFlags apiFlags = ci::vr::API_ANY, uint32_t deviceIndex = 0 );
//! End session
void endSession( ci::vr::Context* context );

}} // namespace cinder::vr