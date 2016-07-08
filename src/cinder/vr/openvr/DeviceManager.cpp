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

#include "cinder/vr/openvr/DeviceManager.h"
#include "cinder/vr/openvr/Controller.h"
#include "cinder/vr/openvr/Context.h"
#include "cinder/vr/openvr/OpenVr.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

namespace cinder { namespace vr { namespace openvr {

const std::string kDeviceVendorName = "HTC Vive";

// -------------------------------------------------------------------------------------------------
// DeviceManager
// -------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager( ci::vr::Environment *env )
	: ci::vr::DeviceManager( ci::vr::API_OPENVR, kDeviceVendorName, env )
{
	if( ! ::vr::VR_IsHmdPresent() ) {
		throw ci::vr::openvr::Exception( "OpenVR HMD is not present" );
	}
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::initialize()
{
	CI_LOG_I( "Initializing devices for HTC Vive" );
	
	::vr::EVRInitError error = ::vr::VRInitError_None;
	mVrSystem = ::vr::VR_Init( &error, ::vr::VRApplication_Scene );
	if( ::vr::VRInitError_None != error ) {
		std::string errMsg = ::vr::VR_GetVRInitErrorAsEnglishDescription( error );
		throw openvr::Exception( errMsg );
	}

	mDriverName = ci::vr::openvr::getTrackedDeviceString( mVrSystem, ::vr::k_unTrackedDeviceIndex_Hmd, ::vr::Prop_TrackingSystemName_String );
	mDisplayName = ci::vr::openvr::getTrackedDeviceString( mVrSystem, ::vr::k_unTrackedDeviceIndex_Hmd, ::vr::Prop_SerialNumber_String );
}

void DeviceManager::destroy()
{
	CI_LOG_I( "Destroying devices for HTC Vive" );

	::vr::VR_Shutdown();
	mVrSystem = nullptr;
}

ci::vr::openvr::RenderModelDataRef DeviceManager::getRenderModelData( const std::string& renderModelName ) const
{
	ci::vr::openvr::RenderModelDataRef result;
	
	auto it = mRenderModelData.find( renderModelName );
	if( mRenderModelData.end() != it ) {
		result = it->second;
	}
	else {
		::vr::RenderModel_t* renderModelModel = nullptr;
		::vr::EVRRenderModelError error;
		while( 1 ) {
			error = ::vr::VRRenderModels()->LoadRenderModel_Async( renderModelName.c_str(), &renderModelModel );
			if( ::vr::VRRenderModelError_Loading != error ) {
				break;
			}

			threadSleep( 1 );
		}

		if( ::vr::VRRenderModelError_None != error ) {
			return ci::vr::openvr::RenderModelDataRef();
		}

		::vr::RenderModel_TextureMap_t* renderModelTexture = nullptr;
		while( 1 ) {
			error = ::vr::VRRenderModels()->LoadTexture_Async( renderModelModel->diffuseTextureId, &renderModelTexture );
			if( ::vr::VRRenderModelError_Loading != error ) {
				break;
			}

			threadSleep( 1 );
		}

		if( ::vr::VRRenderModelError_None != error ) {
			::vr::VRRenderModels()->FreeRenderModel( renderModelModel );
			return ci::vr::openvr::RenderModelDataRef();
		}

		auto renderModelData = ci::vr::openvr::RenderModelData::create( renderModelName, renderModelModel, renderModelTexture );
		if( renderModelData ) {
			mRenderModelData[renderModelName] = renderModelData;
			result = renderModelData;
		}
		else {
			CI_LOG_W( "Couldn't find render model data for: " << renderModelName );
		}

		::vr::VRRenderModels()->FreeRenderModel( renderModelModel );
		::vr::VRRenderModels()->FreeTexture( renderModelTexture );
	}

	return result;
}

uint32_t DeviceManager::numDevices() const
{
	const uint32_t kMaxDevices = 1;
	return kMaxDevices;
}

ci::vr::ContextRef DeviceManager::createContext( const ci::vr::SessionOptions& sessionOptions, uint32_t deviceIndex )
{
	if( deviceIndex >= numDevices() ) {
		throw ci::vr::openvr::Exception( "Device index out of range, deviceIndex=" + std::to_string( deviceIndex ) + ", maxIndex=" + std::to_string( numDevices() ) );
	}

	ci::vr::ContextRef result = ci::vr::openvr::Context::create( sessionOptions, this );
	return result;
}

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )