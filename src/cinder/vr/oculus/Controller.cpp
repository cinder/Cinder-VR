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

#include "cinder/vr/oculus/Controller.h"
#include "cinder/vr/oculus/Oculus.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

namespace cinder { namespace vr { namespace oculus {

// -------------------------------------------------------------------------------------------------
// Controller
// -------------------------------------------------------------------------------------------------
Controller::Controller( ci::vr::Controller::Type type, ::ovrControllerType internalType, ci::vr::Context *context )
	: ci::vr::Controller( type, context ), mInternalType( internalType )
{
}

Controller::~Controller()
{
}

void Controller::processButtons( const ::ovrInputState& state )
{
	// Process down 
	for( uint32_t i = 0; i < 32; ++i ) {
		::ovrButton buttonMask = static_cast<::ovrButton>( 1u << i );
		if( buttonMask == ( state.Buttons & buttonMask ) ) {
			ci::vr::Controller::ButtonId buttonId = fromOvr( buttonMask );
			auto button = getButton( buttonId );
			if( button ) {
				setButtonState( button, ci::vr::Controller::STATE_DOWN );
			}
		}
	}

	// Process up
	for( auto button : mButtons ) {
		if( ci::vr::Controller::STATE_UNKNOWN == button->getState() ) {
			continue;
		}

		::ovrButton buttonMask = toOvr( button->getId() );
		if( buttonMask != ( state.Buttons & buttonMask ) ) {
			setButtonState( button.get(), ci::vr::Controller::STATE_UP );
		}
	}
}

void Controller::processTriggers( const ::ovrInputState& state )
{
	switch( mInternalType ) {
		case ::ovrControllerType_LTouch:
		case ::ovrControllerType_RTouch: {
			for( auto& trigger : mTriggers ) {
				switch( trigger->getId() ) {
					case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_INDEX: {
						setTriggerValue( trigger.get(), state.IndexTrigger[::ovrHand_Left] );
					}
					break;

					case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_INDEX: {
						setTriggerValue( trigger.get(), state.IndexTrigger[::ovrHand_Right] );
					}
					break;

					case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_HAND: {
						setTriggerValue( trigger.get(), state.HandTrigger[::ovrHand_Left] );
					}
					break;

					case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_HAND: {
						setTriggerValue( trigger.get(), state.HandTrigger[::ovrHand_Right] );
					}
					break;
				}
			}
		}
		break;

		case ::ovrControllerType_XBox: {
			for( auto& trigger : mTriggers ) {
				switch( trigger->getId() ) {
					case ci::vr::Controller::TRIGGER_OCULUS_XBOX_LEFT: {
						setTriggerValue( trigger.get(), state.IndexTrigger[::ovrHand_Left] );
					}
					break;

					case ci::vr::Controller::TRIGGER_OCULUS_XBOX_RIGHT: {
						setTriggerValue( trigger.get(), state.IndexTrigger[::ovrHand_Right] );
					}
					break;
				}
			}
		}
		break;
	}
}

void Controller::processAxes( const ::ovrInputState& state )
{
	switch( mInternalType ) {
		case ::ovrControllerType_LTouch:
		case ::ovrControllerType_RTouch: {
			for( auto& axis : mAxes ) {
				switch( axis->getId() ) {
					case ci::vr::Controller::AXIS_OCULUS_TOUCH_LTHUMBSTICK: {
						setAxisValue( axis.get(), ci::vr::oculus::fromOvr( state.Thumbstick[::ovrHand_Left] ) );
					}
					break;

					case ci::vr::Controller::AXIS_OCULUS_TOUCH_RTHUMBSTICK: {
						setAxisValue( axis.get(), ci::vr::oculus::fromOvr( state.Thumbstick[::ovrHand_Right] ) );
					}
					break;
				}
			}
		}
		break;

		case ::ovrControllerType_XBox: {
			for( auto& axis : mAxes ) {
				switch( axis->getId() ) {
					case ci::vr::Controller::AXIS_OCULUS_XBOX_LTHUMBSTICK: {
						setAxisValue( axis.get(), ci::vr::oculus::fromOvr( state.Thumbstick[::ovrHand_Left] ) );
					}
					break;

					case ci::vr::Controller::AXIS_OCULUS_XBOX_RTHUMBSTICK: {
						setAxisValue( axis.get(), ci::vr::oculus::fromOvr( state.Thumbstick[::ovrHand_Right] ) );
					}
					break;
				}
			}
		}
		break;
	}
}

// -------------------------------------------------------------------------------------------------
// ControllerRemote
// -------------------------------------------------------------------------------------------------
ControllerRemote::ControllerRemote( ci::vr::Context *context )
	: ci::vr::oculus::Controller( ci::vr::Controller::TYPE_REMOTE, ::ovrControllerType_Remote, context )
{
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_ENTER, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_BACK, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_LEFT, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_UP, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_RIGHT, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_DOWN, this ) );
}

ControllerRemote::~ControllerRemote()
{
}

ci::vr::oculus::ControllerRef ControllerRemote::create( ci::vr::Context *context )
{
	ci::vr::oculus::ControllerRef result = ci::vr::oculus::ControllerRef( new ci::vr::oculus::ControllerRemote( context ) );
	return result;
}

std::string ControllerRemote::getName() const
{
	return std::string( "Oculus Remote" );
}

std::string ControllerRemote::getButtonName( ci::vr::Controller::ButtonId id ) const
{
	std::string result = "BUTTON_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_ENTER			: result = "BUTTON_OCULUS_REMOTE_ENTER"; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_BACK			: result = "BUTTON_OCULUS_REMOTE_BACK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_LEFT		: result = "BUTTON_OCULUS_REMOTE_DPAD_LEFT"; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_UP		: result = "BUTTON_OCULUS_REMOTE_DPAD_UP"; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_RIGHT	: result = "BUTTON_OCULUS_REMOTE_DPAD_RIGHT"; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_DOWN		: result = "BUTTON_OCULUS_REMOTE_DPAD_DOWN"; break;
		default: break;
	}
	return result;
}

std::string ControllerRemote::getTriggerName( ci::vr::Controller::TriggerId id ) const
{
	std::string result = "TRIGGER_UNKNOWN";
	return result;
}

std::string ControllerRemote::getAxisName( ci::vr::Controller::AxisId id ) const
{
	std::string result = "AXIS_UNKNOWN";
	return result;
}

::ovrButton ControllerRemote::toOvr( ci::vr::Controller::ButtonId value ) const
{
	::ovrButton result = static_cast<::ovrButton>( 0 );
	switch( value ) {
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_ENTER			: result = ::ovrButton_Enter; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_BACK			: result = ::ovrButton_Back; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_LEFT		: result = ::ovrButton_Left; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_UP		: result = ::ovrButton_Up; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_RIGHT	: result = ::ovrButton_Right; break;
		case ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_DOWN		: result = ::ovrButton_Down; break;
		default: break;
	}
	return result;
}

ci::vr::Controller::ButtonId ControllerRemote::fromOvr( ::ovrButton value ) const
{
	ci::vr::Controller::ButtonId result = ci::vr::Controller::BUTTON_UNKNOWN;
	switch( value ) {
		case ::ovrButton_Enter : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_ENTER; break;
		case ::ovrButton_Back  : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_BACK; break;
		case ::ovrButton_Left  : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_LEFT; break;
		case ::ovrButton_Up    : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_UP; break;
		case ::ovrButton_Right : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_RIGHT; break;
		case ::ovrButton_Down  : result = ci::vr::Controller::BUTTON_OCULUS_REMOTE_DPAD_DOWN; break;
		default: break;
	}
	return result;
}

void ControllerRemote::processInputState( const ::ovrInputState& state )
{
	processButtons( state );
}

// -------------------------------------------------------------------------------------------------
// ControllerXbox
// -------------------------------------------------------------------------------------------------
ControllerXbox::ControllerXbox( ci::vr::Context *context )
	: ci::vr::oculus::Controller( ci::vr::Controller::TYPE_XBOX, ::ovrControllerType_XBox, context )
{
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_A, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_B, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_X, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_Y, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_LTHUMBSTICK, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_RTHUMBSTICK, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_LSHOULDER, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_RSHOULDER, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_ENTER, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_BACK, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_XBOX_HOME, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_DPAD_LEFT, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_DPAD_UP, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_DPAD_RIGHT, this ) );
	mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_DPAD_DOWN, this ) );

	mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_XBOX_LEFT, this ) );
	mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_XBOX_RIGHT, this ) );

	mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_OCULUS_XBOX_LTHUMBSTICK, this ) );
	mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_OCULUS_XBOX_RTHUMBSTICK, this ) );
}

ControllerXbox::~ControllerXbox()
{
}

ci::vr::oculus::ControllerRef ControllerXbox::create( ci::vr::Context *context )
{
	ci::vr::oculus::ControllerRef result = ci::vr::oculus::ControllerRef( new ci::vr::oculus::ControllerXbox( context ) );
	return result;
}

std::string ControllerXbox::getName() const
{
	return std::string( "Oculus Xbox" );
}

std::string ControllerXbox::getButtonName( ci::vr::Controller::ButtonId id ) const
{
	std::string result = "BUTTON_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_A			: result = "BUTTON_OCULUS_XBOX_A"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_B			: result = "BUTTON_OCULUS_XBOX_B"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_X			: result = "BUTTON_OCULUS_XBOX_X"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_Y			: result = "BUTTON_OCULUS_XBOX_Y"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_LTHUMBSTICK	: result = "BUTTON_OCULUS_XBOX_LTHUMBSTICK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_RTHUMBSTICK	: result = "BUTTON_OCULUS_XBOX_RTHUMBSTICK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_LSHOULDER	: result = "BUTTON_OCULUS_XBOX_LSHOULDER"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_RSHOULDER	: result = "BUTTON_OCULUS_XBOX_RSHOULDER"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_ENTER		: result = "BUTTON_OCULUS_XBOX_ENTER"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_BACK		: result = "BUTTON_OCULUS_XBOX_BACK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_HOME		: result = "BUTTON_OCULUS_XBOX_HOME"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_LEFT	: result = "BUTTON_OCULUS_XBOX_DPAD_LEFT"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_UP		: result = "BUTTON_OCULUS_XBOX_DPAD_UP"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_RIGHT	: result = "BUTTON_OCULUS_XBOX_DPAD_RIGHT"; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_DOWN	: result = "BUTTON_OCULUS_XBOX_DPAD_DOWN"; break;
	}
	return result;
}

std::string ControllerXbox::getTriggerName( ci::vr::Controller::TriggerId id ) const
{
	std::string result = "TRIGGER_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::TRIGGER_OCULUS_XBOX_LEFT	: result = "TRIGGER_OCULUS_XBOX_LEFT"; break;
		case ci::vr::Controller::TRIGGER_OCULUS_XBOX_RIGHT	: result = "TRIGGER_OCULUS_XBOX_RIGHT"; break;
		default: break;
	}
	return result;
}

std::string ControllerXbox::getAxisName( ci::vr::Controller::AxisId id ) const
{
	std::string result = "AXIS_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::AXIS_OCULUS_XBOX_LTHUMBSTICK	: result = "AXIS_OCULUS_XBOX_LTHUMBSTICK"; break;
		case ci::vr::Controller::AXIS_OCULUS_XBOX_RTHUMBSTICK	: result = "AXIS_OCULUS_XBOX_RTHUMBSTICK"; break;
		default: break;
	}
	return result;
}

::ovrButton ControllerXbox::toOvr( ci::vr::Controller::ButtonId value ) const
{
	::ovrButton result = static_cast<::ovrButton>( 0 );
	switch( value ) {
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_A			: result = ::ovrButton_A; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_B			: result = ::ovrButton_B; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_X			: result = ::ovrButton_X; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_Y			: result = ::ovrButton_Y; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_LTHUMBSTICK	: result = ::ovrButton_LThumb; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_RTHUMBSTICK	: result = ::ovrButton_RThumb; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_LSHOULDER	: result = ::ovrButton_LShoulder; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_RSHOULDER	: result = ::ovrButton_RShoulder; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_ENTER		: result = ::ovrButton_Enter; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_BACK		: result = ::ovrButton_Back; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_HOME		: result = ::ovrButton_Home; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_LEFT	: result = ::ovrButton_Left; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_UP		: result = ::ovrButton_Up; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_RIGHT	: result = ::ovrButton_Right; break;
		case ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_DOWN	: result = ::ovrButton_Down; break;

	}
	return result;
}

ci::vr::Controller::ButtonId ControllerXbox::fromOvr( ::ovrButton value ) const
{
	ci::vr::Controller::ButtonId result = ci::vr::Controller::BUTTON_UNKNOWN;
	switch( value ) {
		case ::ovrButton_A			: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_A; break;
		case ::ovrButton_B			: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_B; break;
		case ::ovrButton_X			: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_X; break;
		case ::ovrButton_Y			: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_Y; break;
		case ::ovrButton_LThumb		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_LTHUMBSTICK; break;
		case ::ovrButton_RThumb		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_RTHUMBSTICK; break;
		case ::ovrButton_LShoulder	: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_LSHOULDER; break;
		case ::ovrButton_RShoulder	: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_RSHOULDER; break;
		case ::ovrButton_Enter		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_ENTER; break;
		case ::ovrButton_Back		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_BACK; break;
		case ::ovrButton_Home		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_HOME; break;
		case ::ovrButton_Left		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_LEFT; break;
		case ::ovrButton_Up			: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_UP; break;
		case ::ovrButton_Right		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_RIGHT; break;
		case ::ovrButton_Down		: result = ci::vr::Controller::BUTTON_OCULUS_XBOX_DPAD_DOWN; break;
	}
	return result;
}

void ControllerXbox::processInputState( const ::ovrInputState& state )
{
	 processButtons( state );
	 processTriggers( state );
	 processAxes( state );
}

// -------------------------------------------------------------------------------------------------
// ControllerTouch
// -------------------------------------------------------------------------------------------------
ControllerTouch::ControllerTouch( ci::vr::Controller::Type type, ci::vr::Context *context )
	: ci::vr::oculus::Controller( type, ci::vr::oculus::toOvr( type ), context )
{
	if( ( ci::vr::Controller::TYPE_LEFT != type ) && ( ci::vr::Controller::TYPE_RIGHT != type ) ) {
		throw ci::vr::oculus::Exception( "Invalid touch controller type" );
	}

	const float kTouchHandMinLimit = 0.2f;
	const float kTouchHandMaxLimit = 0.94f;

	switch( type ) {
		case ci::vr::Controller::TYPE_LEFT: {
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_X, this ) );
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_Y, this ) );
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_LTHUMBSTICK, this ) );
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_ENTER, this ) );

			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_INDEX, this ) );
			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_HAND, this, kTouchHandMinLimit, kTouchHandMaxLimit ) );

			mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_OCULUS_TOUCH_LTHUMBSTICK, this ) );
		}
		break;

		case ci::vr::Controller::TYPE_RIGHT: {
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_A, this ) );
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_B, this ) );
			mButtons.push_back( ci::vr::Controller::Button::create( ci::vr::Controller::BUTTON_OCULUS_TOUCH_RTHUMBSTICK, this ) );

			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_INDEX, this ) );
			mTriggers.push_back( ci::vr::Controller::Trigger::create( ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_HAND, this, kTouchHandMinLimit, kTouchHandMaxLimit ) );

			mAxes.push_back( ci::vr::Controller::Axis::create( ci::vr::Controller::AXIS_OCULUS_TOUCH_RTHUMBSTICK, this ) );
		}
		break;
	}
}

ControllerTouch::~ControllerTouch()
{
}

ci::vr::oculus::ControllerRef ControllerTouch::create( ci::vr::Controller::Type type, ci::vr::Context *context )
{
	ci::vr::oculus::ControllerRef result = ci::vr::oculus::ControllerRef( new ci::vr::oculus::ControllerTouch( type, context ) );
	return result;
}

std::string ControllerTouch::getName() const
{
	std::string result = "Oculus Unknown Touch";
	if( ci::vr::Controller::TYPE_LEFT == getType() ) {
		result = "Oculus Left Touch";
	}
	else if( ci::vr::Controller::TYPE_RIGHT == getType() ) {
		result = "Oculus Right Touch";
	}
	return result;
}

std::string ControllerTouch::getButtonName( ci::vr::Controller::ButtonId id ) const
{
	std::string result = "BUTTON_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_A           : result = "BUTTON_OCULUS_A"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_B           : result = "BUTTON_OCULUS_B"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_X           : result = "BUTTON_OCULUS_X"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_Y           : result = "BUTTON_OCULUS_Y"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_LTHUMBSTICK : result = "BUTTON_OCULUS_LTHUMBSTICK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_RTHUMBSTICK : result = "BUTTON_OCULUS_RTHUMBSTICK"; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_ENTER       : result = "BUTTON_OCULUS_ENTER"; break;
	}
	return result;
}

std::string ControllerTouch::getTriggerName( ci::vr::Controller::TriggerId id ) const
{
	std::string result = "TRIGGER_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_INDEX	: result = "TRIGGER_OCULUS_TOUCH_LEFT_INDEX"; break;
		case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_LEFT_HAND		: result = "TRIGGER_OCULUS_TOUCH_LEFT_HAND"; break;
		case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_INDEX	: result = "TRIGGER_OCULUS_TOUCH_RIGHT_INDEX"; break;
		case ci::vr::Controller::TRIGGER_OCULUS_TOUCH_RIGHT_HAND	: result = "TRIGGER_OCULUS_TOUCH_RIGHT_HAND"; break;
		default: break;
	}
	return result;
}

std::string ControllerTouch::getAxisName( ci::vr::Controller::AxisId id ) const
{
	std::string result = "AXIS_UNKNOWN";
	switch( id ) {
		case ci::vr::Controller::AXIS_OCULUS_TOUCH_LTHUMBSTICK	: result = "AXIS_OCULUS_TOUCH_LTHUMBSTICK"; break;
		case ci::vr::Controller::AXIS_OCULUS_TOUCH_RTHUMBSTICK	: result = "AXIS_OCULUS_TOUCH_RTHUMBSTICK"; break;
		default: break;
	}
	return result;
}

::ovrButton ControllerTouch::toOvr( ci::vr::Controller::ButtonId value ) const
{
	::ovrButton result = static_cast<::ovrButton>( 0 );
	switch( value ) {
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_A           : result = ::ovrButton_A; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_B           : result = ::ovrButton_B; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_X           : result = ::ovrButton_X; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_Y           : result = ::ovrButton_Y; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_LTHUMBSTICK : result = ::ovrButton_LThumb; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_RTHUMBSTICK : result = ::ovrButton_RThumb; break;
		case ci::vr::Controller::BUTTON_OCULUS_TOUCH_ENTER       : result = ::ovrButton_Enter; break;
	}
	return result;
}

ci::vr::Controller::ButtonId ControllerTouch::fromOvr( ::ovrButton value ) const
{
	ci::vr::Controller::ButtonId result = ci::vr::Controller::BUTTON_UNKNOWN;
	switch( value ) {
		case ::ovrButton_A         : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_A; break;
		case ::ovrButton_B         : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_B; break;
		case ::ovrButton_X         : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_X; break;
		case ::ovrButton_Y         : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_Y; break;
		case ::ovrButton_LThumb    : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_LTHUMBSTICK; break;
		case ::ovrButton_RThumb    : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_RTHUMBSTICK; break;
		case ::ovrButton_Enter     : result = ci::vr::Controller::BUTTON_OCULUS_TOUCH_ENTER; break;
	}
	return result;
}

void ControllerTouch::processInputState( const ::ovrInputState& inputState )
{
	 processButtons( inputState );
	 processTriggers( inputState );
	 processAxes( inputState );
}

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )