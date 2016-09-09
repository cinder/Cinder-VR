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

#include "cinder/vr/openvr/Controller.h"
#include "cinder/vr/openvr/OpenVr.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

namespace cinder { namespace vr { namespace openvr {

Controller::Controller( ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ci::vr::Controller::Type type, ci::vr::Context *context )
	: ci::vr::Controller( type, context ), mTrackedDeviceIndex( trackedDeviceIndex )
{
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_VIVE_APPLICATION_MENU, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_VIVE_GRIP, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_VIVE_TOUCHPAD, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_VIVE_TRIGGER, this ) );

	// Default to button states being up
	{
		const std::vector<::vr::EVRButtonId> buttonIds = {
			::vr::k_EButton_ApplicationMenu,
			::vr::k_EButton_Grip,
			::vr::k_EButton_SteamVR_Touchpad,
			::vr::k_EButton_SteamVR_Trigger
		};

		for( const auto& buttonId : buttonIds ) {
			auto button = getButton( fromOpenVr( buttonId ) );
			if( button ) {
				setButtonState( button,  ci::vr::Controller::STATE_UP );
			}
		}
	}

	switch( type ) {
		case ci::vr::Controller::TYPE_LEFT: {
			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_VIVE_LEFT, this ) );
			mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_VIVE_LEFT, this ) );
		}
		break;

		case ci::vr::Controller::TYPE_RIGHT: {
			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_VIVE_RIGHT, this ) );
			mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_VIVE_RIGHT, this ) );
		}
		break;

		default : break;
	}
}

Controller::~Controller()
{
}

ci::vr::openvr::ControllerRef Controller::create( ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ci::vr::Controller::Type type, ci::vr::Context *context )
{
	ci::vr::openvr::ControllerRef result = ci::vr::openvr::ControllerRef( new ci::vr::openvr::Controller( trackedDeviceIndex, type, context ) );
	return result;
}

std::string Controller::getName() const
{
	std::string result = "HTC Vive Controller";
	if( ci::vr::Controller::TYPE_LEFT == getType() ) {
		result += " (Left)";
	}
	else if( ci::vr::Controller::TYPE_RIGHT == getType() ) {
		result += " (Right)";
	}
	return result;
}

std::string Controller::getButtonName( ci::vr::Controller::ButtonId id ) const
{
	std::string result = "BUTTON_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::BUTTON_VIVE_APPLICATION_MENU : result = "BUTTON_VIVE_APPLICATION_MENU"; break;
		case ci::vr::Controller::BUTTON_VIVE_GRIP             : result = "BUTTON_VIVE_GRIP"; break;
		case ci::vr::Controller::BUTTON_VIVE_TOUCHPAD         : result = "BUTTON_VIVE_TOUCHPAD"; break;
		case ci::vr::Controller::BUTTON_VIVE_TRIGGER          : result = "BUTTON_VIVE_TRIGGER"; break;
		default: break;
	}

	if( ci::vr::Controller::TYPE_LEFT == getType() ) {
		result += " (Left)";
	}
	else if( ci::vr::Controller::TYPE_RIGHT == getType() ) {
		result += " (Right)";
	}

	return result;
}

std::string Controller::getTriggerName( ci::vr::Controller::TriggerId id ) const
{
	std::string result = "TRIGGER_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::TRIGGER_VIVE_LEFT	: result = "TRIGGER_VIVE_LEFT"; break;
		case ci::vr::Controller::TRIGGER_VIVE_RIGHT	: result = "TRIGGER_VIVE_RIGHT"; break;
		default: break;
	}

	if( ci::vr::Controller::TYPE_LEFT == getType() ) {
		result += " (Left)";
	}
	else if( ci::vr::Controller::TYPE_RIGHT == getType() ) {
		result += " (Right)";
	}

	return result;
}

std::string Controller::getAxisName( ci::vr::Controller::AxisId id ) const
{
	std::string result = "TOUCHPAD_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::AXIS_VIVE_LEFT		: result = "AXIS_VIVE_LEFT"; break;
		case ci::vr::Controller::AXIS_VIVE_RIGHT	: result = "AXIS_VIVE_RIGHT"; break;
		default: break;
	}

	if( ci::vr::Controller::TYPE_LEFT == getType() ) {
		result += " (Left)";
	}
	else if( ci::vr::Controller::TYPE_RIGHT == getType() ) {
		result += " (Right)";
	}

	return result;
}

::vr::EVRButtonId Controller::toOpenVr( ci::vr::Controller::ButtonId value ) const
{
	::vr::EVRButtonId result = static_cast<::vr::EVRButtonId>( UINT32_MAX );
	switch( value ) {
		case ci::vr::Controller::BUTTON_VIVE_APPLICATION_MENU : result = ::vr::k_EButton_ApplicationMenu; break;
		case ci::vr::Controller::BUTTON_VIVE_GRIP             : result = ::vr::k_EButton_Grip; break;
		case ci::vr::Controller::BUTTON_VIVE_TOUCHPAD         : result = ::vr::k_EButton_SteamVR_Touchpad; break;
		case ci::vr::Controller::BUTTON_VIVE_TRIGGER          : result = ::vr::k_EButton_SteamVR_Trigger; break;
		default: break;
	}
	return result;
}

ci::vr::Controller::ButtonId Controller::fromOpenVr( ::vr::EVRButtonId value ) const
{
	ci::vr::Controller::ButtonId result = ci::vr::Controller::BUTTON_UNKNOWN;
	switch( value ) {
		case ::vr::k_EButton_ApplicationMenu  : result = ci::vr::Controller::BUTTON_VIVE_APPLICATION_MENU; break;
		case ::vr::k_EButton_Grip             : result = ci::vr::Controller::BUTTON_VIVE_GRIP; break;
		case ::vr::k_EButton_SteamVR_Touchpad : result = ci::vr::Controller::BUTTON_VIVE_TOUCHPAD; break;
		case ::vr::k_EButton_SteamVR_Trigger  : result = ci::vr::Controller::BUTTON_VIVE_TRIGGER; break;
		default: break;
	}
	return result;
}

void Controller::processControllerState( const ::vr::VRControllerState_t& state )
{
	// Skip if the packet number is the same
	if( mPacketNum == state.unPacketNum ) {
		return;
	}

	// Update packet number
	mPacketNum = state.unPacketNum;

	// Process it all
	processButtons( state );
	processTriggers( state );
	processAxes( state );
}

void Controller::processButtons( const ::vr::VRControllerState_t& state )
{
	const std::vector<::vr::EVRButtonId> buttonIds = {
		::vr::k_EButton_ApplicationMenu,
		::vr::k_EButton_Grip,
		::vr::k_EButton_SteamVR_Touchpad,
		::vr::k_EButton_SteamVR_Trigger
	};

	for( const auto& buttonId : buttonIds ) {
		uint64_t buttonMask = ::vr::ButtonMaskFromId( buttonId );
		bool isPressed = ( buttonMask == ( state.ulButtonPressed & buttonMask ) );
		auto button = getButton( fromOpenVr( buttonId ) );
		if( button ) {
			auto state = ( isPressed ? ci::vr::Controller::STATE_DOWN : ci::vr::Controller::STATE_UP );
			setButtonState( button, state );
		}
	}
}

void Controller::processTriggers( const ::vr::VRControllerState_t& state )
{
	const uint64_t buttonMask = ::vr::ButtonMaskFromId( ::vr::k_EButton_SteamVR_Trigger );
	const bool isTouched = ( buttonMask == ( state.ulButtonTouched & buttonMask ) );
	float value = 0.0f;
	if( isTouched ) {
		value = state.rAxis[ci::vr::openvr::Controller::AXIS_INDEX_TRIGGER].x;
	}
	if( isTouched != mTriggerTouched ) {
		mTriggerTouched = isTouched;
		setTriggerValue( mTriggers[0].get(), value );
	}
}

void Controller::processAxes( const ::vr::VRControllerState_t& state )
{
	const uint64_t buttonMasdk = ::vr::ButtonMaskFromId( ::vr::k_EButton_SteamVR_Touchpad );
	const bool isTouched = ( buttonMasdk == ( state.ulButtonTouched & buttonMasdk ) );
	ci::vec2 value = ci::vec2( 0.0f );
	if(  isTouched ) {
		value.x = state.rAxis[ci::vr::openvr::Controller::AXIS_INDEX_TOUCHPAD].x;
		value.y = state.rAxis[ci::vr::openvr::Controller::AXIS_INDEX_TOUCHPAD].y;
	}
	if( isTouched != mTrackPadTouched ) {
		mTrackPadTouched = isTouched;
		setAxisValue( mAxes[0].get(), value );
	}
}

void Controller::processControllerPose( const ci::mat4& inverseLookMatrix, const ci::mat4& inverseOriginMatrix, const ci::mat4& deviceToTrackingMatrix, const ci::mat4& trackingToDeviceMatrix )
{
	// Ray components
	mDeviceToTrackingMatrix = deviceToTrackingMatrix;
	mTrackingToDeviceMatrix = trackingToDeviceMatrix;
	ci::mat4 coordSysMatrix = inverseLookMatrix * inverseOriginMatrix * mDeviceToTrackingMatrix;
	//ci::mat4 coordSysMatrix = inverseOriginMatrix * mDeviceToTrackingMatrix;
	ci::vec3 p0 = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, 0, 1 ) );
	ci::vec3 dir = ci::vec3( coordSysMatrix * ci::vec4( 0, 0, -1, 0 ) );
	// Input ray
	mInputRay = ci::Ray( p0, dir );
}

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )