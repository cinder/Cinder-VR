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

#include "cinder/vr/Controller.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

#include <openvr.h>

namespace cinder { namespace vr { namespace openvr  {

class Context;

class Controller;
using ControllerRef = std::shared_ptr<Controller>;

//! \class Controller
//!
//!
class Controller : public vr::Controller {
public:
	enum AxisIndex{
		AXIS_INDEX_TOUCHPAD = 0,
		AXIS_INDEX_TRIGGER	= 1
	};

	~Controller();

	static ci::vr::openvr::ControllerRef	create( ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ci::vr::Controller::Type type, ci::vr::Context *context );

	virtual std::string						getName() const override;

	virtual std::string						getButtonName( ci::vr::Controller::ButtonId ) const override;
	virtual std::string						getTriggerName( ci::vr::Controller::TriggerId ) const override;
	virtual std::string						getAxisName( ci::vr::Controller::AxisId ) const override;

	virtual bool							hasInputRay() const override { return mEventsEnabled ? true : false; }

	::vr::TrackedDeviceIndex_t				getTrackedDeviceIndex() const { return mTrackedDeviceIndex; }

	virtual ::vr::EVRButtonId				toOpenVr( ci::vr::Controller::ButtonId value ) const;
	virtual ci::vr::Controller::ButtonId	fromOpenVr( ::vr::EVRButtonId value ) const;

protected:
	Controller( ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ci::vr::Controller::Type type, ci::vr::Context *context );
	friend class ci::vr::openvr::Context;

	virtual void							processControllerState( const ::vr::VRControllerState_t& state);
	virtual void							processButtons( const ::vr::VRControllerState_t& state );
	virtual void							processTriggers( const ::vr::VRControllerState_t& state );
	virtual void							processAxes( const ::vr::VRControllerState_t& state );
	virtual void							processControllerPose( const ci::mat4& inverseLookMatrix, const ci::mat4& inverseOriginMatrix, const ci::mat4& deviceToTrackingMatrix, const ci::mat4& trackingToDeviceMatrix );

	bool									mEventsEnabled = false;
	bool									isEventsEnabled() const { return mEventsEnabled; }
	void									setEventsEnabled( bool value = true ) { mEventsEnabled = value; }
	void									clearInputRay() { mInputRay = ci::Ray( ci::vec3( 0 ), ci::vec3(0 ) ); }

	::vr::TrackedDeviceIndex_t				mTrackedDeviceIndex = UINT32_MAX;
	//ci::vr::Controller::HandId				mHandId = ci::vr::Controller::HAND_UNKNOWN;

	uint32_t								mPacketNum = UINT32_MAX;
	bool									mTrackPadTouched = false;
	bool									mTriggerTouched = false;
};

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )