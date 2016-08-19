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

#include "cinder/vr/Context.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

#include <openvr.h>
 
namespace cinder { namespace vr { namespace openvr  {

class DeviceManager;

class Context;
class Controller;
using ContextRef = std::shared_ptr<Context>;
using ControllerRef = std::shared_ptr<Controller>;

//! \class Context
//!
//!
class Context : public ci::vr::Context {
public:

	virtual ~Context();

	static ContextRef					create( const ci::vr::SessionOptions& sessionOptions, ci::vr::openvr::DeviceManager* deviceManager );

	ci::vr::openvr::DeviceManager		*getDeviceManager() const { return mDeviceManager; }
	::vr::IVRSystem						*getVrSystem() const { return mVrSystem; }

	virtual bool						hasController( const ci::vr::Controller::Type type ) const override;
	virtual ci::vr::Controller			*getController( ci::vr::Controller::Type type ) const override;
	virtual void						scanForControllers() override;

	void											updatePoseData();
	const ::vr::TrackedDevicePose_t&				getPose( ::vr::TrackedDeviceIndex_t deviceIndex ) const { return mPoses[deviceIndex]; }
	const ci::mat4&									getDeviceToTrackingMatrix( ::vr::TrackedDeviceIndex_t deviceIndex ) const { return mDeviceToTrackingMatrices[deviceIndex]; }
	const ci::mat4&									getTrackingToDeviceMatrix( ::vr::TrackedDeviceIndex_t deviceIndex ) const { return mTrackingToDeviceMatrices[deviceIndex]; }

protected:
	Context( const ci::vr::SessionOptions& sessionOptions, ci::vr::openvr::DeviceManager* deviceManager );
	friend class ci::vr::Environment;

	virtual void						beginSession() override;
	virtual void						endSession() override;

	virtual void						processEvents() override;
	virtual void						processTrackedDeviceEvents( const ::vr::VREvent_t &event );

private:
	ci::vr::openvr::DeviceManager		*mDeviceManager = nullptr;
	::vr::IVRSystem						*mVrSystem = nullptr;
	
	std::vector<::vr::TrackedDevicePose_t>	mPoses;
	std::vector<ci::mat4>					mDeviceToTrackingMatrices;
	std::vector<ci::mat4>					mTrackingToDeviceMatrices;

	//// Don't rename it this to mControllers - mControllers already exists in the base class.
	//ci::vr::openvr::ControllerRef		mViveControllers[ci::vr::Controller::HAND_COUNT];
	std::map<ci::vr::Controller::Type, ci::vr::openvr::ControllerRef>	mViveControllers;

	void								updateControllerConnections();
};

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )