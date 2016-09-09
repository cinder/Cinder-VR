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

#include "cinder/vr/Controller.h"
#include "cinder/vr/Context.h"
#include "cinder/Signals.h"

#include <algorithm>
#include <sstream>

namespace cinder { namespace vr {

// -------------------------------------------------------------------------------------------------
// Controller::ActionState
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
// Controller::Button
// -------------------------------------------------------------------------------------------------
ci::vr::Controller::ButtonRef Controller::Button::create( ci::vr::Controller::ButtonId id, ci::vr::Controller *controller )
{
	ci::vr::Controller::ButtonRef result = ci::vr::Controller::ButtonRef( new ci::vr::Controller::Button( id, controller ) );
	return result;
}

void Controller::Button::setState( ci::vr::Controller::State state )
{
	if( state != mState ) {
		mState = state;
		if( ci::vr::Controller::STATE_DOWN == mState ) {
			mController->getContext()->getSignalControllerButtonDown().emit( this );
	}
		else if( ci::vr::Controller::STATE_UP == mState ) {
			mController->getContext()->getSignalControllerButtonUp().emit( this );
		}
	}	
}

std::string Controller::Button::getInfo() const
{
	std::stringstream ss; 
	ss << "Name: " << mController->getButtonName( mId ) << ", ";
	ss << "State: " << ( ci::vr::Controller::STATE_DOWN == mState ? "DOWN" : ( ci::vr::Controller::STATE_UP == mState ? "UP" : "UNKONWN" ) );
	return ss.str();
}

// -------------------------------------------------------------------------------------------------
// Controller::Trigger
// -------------------------------------------------------------------------------------------------
ci::vr::Controller::TriggerRef Controller::Trigger::create( ci::vr::Controller::TriggerId id, ci::vr::Controller *controller, float minLim, float maxLim )
{
	ci::vr::Controller::TriggerRef result = ci::vr::Controller::TriggerRef( new ci::vr::Controller::Trigger( id, controller, minLim, maxLim ) );
	return result;
}

void Controller::Trigger::setValue( float value )
{
	value = std::max( mMinLimit, std::min( mMaxLimit, value ) );
	value = ( value - mMinLimit ) / ( mMaxLimit - mMinLimit );

	float delta = fabs( mValue - value );
	if( ( delta > 0.0f ) || ( value > 0.0f ) ) {
		mValue = value;
		mController->getContext()->getSignalControllerTrigger().emit( this );
	}
}

std::string Controller::Trigger::getInfo() const
{
	std::stringstream ss; 
	ss << "Name: " << mController->getTriggerName( mId ) << ", ";
	ss << "Value: " << mValue;
	return ss.str();
}

// -------------------------------------------------------------------------------------------------
// Controller::Axis
// -------------------------------------------------------------------------------------------------
ci::vr::Controller::AxisRef Controller::Axis::create( ci::vr::Controller::AxisId id, ci::vr::Controller *controller )
{
	ci::vr::Controller::AxisRef result = ci::vr::Controller::AxisRef( new ci::vr::Controller::Axis( id, controller ) );
	return result;
}

void Controller::Axis::setValue( const ci::vec2 &value )
{
	float delta = ci::length( mValue - value );
	if( delta > 0.0f ) {
		mValue = value;
		mController->getContext()->getSignalControllerAxis().emit( this );
	}
}

std::string Controller::Axis::getInfo() const
{
	std::stringstream ss; 
	ss << "Name: " << mController->getAxisName( mId ) << ", ";
	ss << "State: " << mValue;
	return ss.str();
}

// -------------------------------------------------------------------------------------------------
// Controller
// -------------------------------------------------------------------------------------------------
Controller::Controller( ci::vr::Controller::Type type, ci::vr::Context *context )
	: mContext( context ), mType( type )
{
}

Controller::~Controller()
{
}

ci::vr::Api Controller::getApi() const
{
	return mContext->getApi();
}

ci::vr::Controller::Button* Controller::getButton( ci::vr::Controller::ButtonId id )
{
	ci::vr::Controller::Button* result = nullptr;
	auto it = std::find_if( std::begin( mButtons ), std::end( mButtons ),
		[id]( const ci::vr::Controller::ButtonRef& elem ) -> bool {
			return id == elem->getId();
		}
	);
	if( std::end( mButtons ) != it ) {
		result = it->get();
	}
	return result;
}

const ci::vr::Controller::Button* Controller::getButton( ci::vr::Controller::ButtonId id ) const
{
	const ci::vr::Controller::Button* result = nullptr;
	auto it = std::find_if( std::begin( mButtons ), std::end( mButtons ),
		[id]( const ci::vr::Controller::ButtonRef& elem ) -> bool {
			return id == elem->getId();
		}
	);
	if( std::end( mButtons ) != it ) {
		result = it->get();
	}
	return result;
}

ci::vr::Controller::Trigger* Controller::getTrigger( ci::vr::Controller::TriggerId id )
{
	ci::vr::Controller::Trigger* result = nullptr;
	if( ci::vr::Controller::TRIGGER_ANY == id ) {
		if( ! mTriggers.empty() ) {
			result = mTriggers[0].get();
		}
	}
	else {
		auto it = std::find_if( std::begin( mTriggers ), std::end( mTriggers ),
			[id]( const ci::vr::Controller::TriggerRef& elem ) -> bool {
				return id == elem->getId();
			}
		);
		if( std::end( mTriggers ) != it ) {
			result = it->get();
		}
	}
	return result;
}

const ci::vr::Controller::Trigger* Controller::getTrigger( ci::vr::Controller::TriggerId id ) const
{
	const ci::vr::Controller::Trigger* result = nullptr;
	if( ci::vr::Controller::TRIGGER_ANY == id ) {
		if( ! mTriggers.empty() ) {
			result = mTriggers[0].get();
		}
	}
	else {
		auto it = std::find_if( std::begin( mTriggers ), std::end( mTriggers ),
			[id]( const ci::vr::Controller::TriggerRef& elem ) -> bool {
				return id == elem->getId();
			}
		);
		if( std::end( mTriggers ) != it ) {
			result = it->get();
		}
	}
	return result;
}

ci::vr::Controller::Axis* Controller::getAxis( ci::vr::Controller::AxisId id )
{
	ci::vr::Controller::Axis* result = nullptr;
	if( ci::vr::Controller::AXIS_ANY == id ) {
		if( ! mAxes.empty() ) {
			result = mAxes[0].get();
		}
	}
	else {
		auto it = std::find_if( std::begin( mAxes ), std::end( mAxes ),
			[id]( const ci::vr::Controller::AxisRef& elem ) -> bool {
				return id == elem->getId();
			}
		);
		if( std::end( mAxes ) != it ) {
			result = it->get();
		}
	}
	return result;
}

const ci::vr::Controller::Axis* Controller::getAxis( ci::vr::Controller::AxisId id ) const
{
	const ci::vr::Controller::Axis* result = nullptr;
	if( ci::vr::Controller::AXIS_ANY == id ) {
		if( ! mAxes.empty() ) {
			result = mAxes[0].get();
		}
	}
	else {
		auto it = std::find_if( std::begin( mAxes ), std::end( mAxes ),
			[id]( const ci::vr::Controller::AxisRef& elem ) -> bool {
				return id == elem->getId();
			}
		);
		if( std::end( mAxes ) != it ) {
			result = it->get();
		}
	}
	return result;
}

void Controller::setButtonState( ci::vr::Controller::Button *button, ci::vr::Controller::State state )
{
	button->setState( state );
}

void Controller::setTriggerValue( ci::vr::Controller::Trigger *trigger, float value )
{
	trigger->setValue( value );
}

void Controller::setAxisValue( ci::vr::Controller::Axis *axis, const ci::vec2 &value )
{
	axis->setValue( value );
}

}} // namespace cinder::vr