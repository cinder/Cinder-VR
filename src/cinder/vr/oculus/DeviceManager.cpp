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

#include "cinder/vr/oculus/DeviceManager.h"
#include "cinder/vr/oculus/Controller.h"
#include "cinder/vr/oculus/Context.h"
#include "cinder/vr/oculus/Oculus.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

#include <string>

namespace cinder { namespace vr { namespace oculus {

const std::string kDeviceVendorName = "Oculus Rift";

// -------------------------------------------------------------------------------------------------
// DeviceManager
// -------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager( ci::vr::Environment *env )
	: ci::vr::DeviceManager( ci::vr::API_OCULUS, kDeviceVendorName, env )
{
	// Detect service and HMD with 5 second time out
	::ovrDetectResult result = ::ovr_Detect( 5000 );
	// Error out if service isn't running
	if( ! result.IsOculusServiceRunning ) {
		throw ci::vr::oculus::Exception( "Oculus service is not running or is not installed" );
	}
	// Error out if HMD is not present
	if( ! result.IsOculusHMDConnected ) {
		throw ci::vr::oculus::Exception( "Oculus HMD is not present" );
	}
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::initialize()
{
	CI_LOG_I( "Initializing devices for Oculus Rift" );
	ci::vr::oculus::checkResult( ::ovr_Initialize( nullptr ) );
}

void DeviceManager::destroy()
{
	CI_LOG_I( "Destroying devices for Oculus Rift" );
	::ovr_Shutdown();
}

uint32_t DeviceManager::numDevices() const
{
	const uint32_t kMaxDevices = 1;
	return kMaxDevices;
}

ci::vr::ContextRef DeviceManager::createContext( const ci::vr::SessionOptions& sessionOptions, uint32_t deviceIndex )
{
	if( deviceIndex >= numDevices() ) {
		throw ci::vr::oculus::Exception( "Device index out of range, deviceIndex=" + std::to_string( deviceIndex ) + ", maxIndex=" + std::to_string( numDevices() ) );
	}

	ci::vr::ContextRef result = ci::vr::oculus::Context::create( sessionOptions, this );
	return result;
}

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )