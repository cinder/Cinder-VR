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

#include "cinder/vr/Platform.h"
#include "cinder/Ray.h"

#include <vector>

namespace cinder { namespace vr {

class Context;

//! \class Controller
//!
//!
class Controller {
public:

	enum Type {
		TYPE_UNKNOWN		= 0xFFFFFFFF,
		TYPE_LEFT			= 0x00000001,
		TYPE_RIGHT			= 0x00000002,
		TYPE_REMOTE			= 0x00000004,
		TYPE_XBOX			= 0x00000008,
		TYPE_CUSTOM_START	= 0x00001000,
	};

	enum ButtonId {
		BUTTON_UNKNOWN					= 0xFFFFFFFF,

		BUTTON_1						= 0x00000001,
		BUTTON_2						= 0x00000002,
		BUTTON_3						= 0x00000004,
		BUTTON_4						= 0x00000008,
		BUTTON_5						= 0x00000010,
		BUTTON_6						= 0x00000020,
		BUTTON_7						= 0x00000040,
		BUTTON_8						= 0x00000080,
		BUTTON_9						= 0x00000100,
		BUTTON_10						= 0x00000200,
		BUTTON_11						= 0x00000400,
		BUTTON_12						= 0x00000800,
		BUTTON_13						= 0x00001000,
		BUTTON_14						= 0x00002000,
		BUTTON_15						= 0x00004000,
		BUTTON_16						= 0x00008000,

		BUTTON_DPAD_LEFT				= 0x00010000,
		BUTTON_DPAD_UP					= 0x00020000,
		BUTTON_DPAD_RIGHT				= 0x00040000,
		BUTTON_DPAD_DOWN				= 0x00080000,

		BUTTON_APP_MENU					= 0x00100000,
		BUTTON_SYSTEM					= 0x00200000,

		BUTTON_OCULUS_REMOTE_ENTER		= BUTTON_1,
		BUTTON_OCULUS_REMOTE_BACK		= BUTTON_2,

		BUTTON_OCULUS_XBOX_A			= BUTTON_1,
		BUTTON_OCULUS_XBOX_B			= BUTTON_2,
		BUTTON_OCULUS_XBOX_X			= BUTTON_3,
		BUTTON_OCULUS_XBOX_Y			= BUTTON_4,
		BUTTON_OCULUS_XBOX_LSHOULDER	= BUTTON_5,
		BUTTON_OCULUS_XBOX_RSHOULDER	= BUTTON_6,
		BUTTON_OCULUS_XBOX_LTHUMBSTICK	= BUTTON_7,
		BUTTON_OCULUS_XBOX_RTHUMBSTICK	= BUTTON_8,
		BUTTON_OCULUS_XBOX_ENTER		= BUTTON_9,	// Start on Xbox 360 Controller
		BUTTON_OCULUS_XBOX_BACK			= BUTTON_10,
		BUTTON_OCULUS_XBOX_HOME			= BUTTON_11,
		BUTTON_OCULUS_XBOX_MENU			= BUTTON_OCULUS_XBOX_ENTER,
		BUTTON_OCULUS_XBOX_VIEW			= BUTTON_OCULUS_XBOX_BACK,

		BUTTON_VIVE_APPLICATION_MENU	= BUTTON_1,
		BUTTON_VIVE_GRIP				= BUTTON_2,
		BUTTON_VIVE_TOUCHPAD			= BUTTON_15,
		BUTTON_VIVE_TRIGGER				= BUTTON_16,
	};

	enum HandId {
		HAND_UNKNOWN = 0xFFFFFFFF,
		HAND_LEFT	 = 0,
		HAND_RIGHT   = 1,
		HAND_COUNT   = 2,
		HAND_ANY	 = 0x0000FFFF
	};

	enum State { 
		STATE_UNKNOWN, 
		STATE_DOWN, 
		STATE_UP 
	};

	// ---------------------------------------------------------------------------------------------

	class Axis;
	class Button;
	class Trigger;
	using AxisRef = std::shared_ptr<Axis>;
	using ButtonRef = std::shared_ptr<Button>;
	using TriggerRef = std::shared_ptr<Trigger>;

	// ---------------------------------------------------------------------------------------------

	//! \class ActionState
	//!
	//!
	class ActionState {
	public:
		virtual ~ActionState() {}
		ci::vr::Controller				*getController() { return mController; }
		const ci::vr::Controller		*getController() const { return mController; }
	protected:
		ActionState( ci::vr::Controller *controller ) 
			: mController( controller ) {}
		ci::vr::Controller				*mController = nullptr;
	};

	//! \class Button
	//!
	//!
	class Button : public ActionState {
	public:
		virtual ~Button() {}
		static ButtonRef				create( ci::vr::Controller::ButtonId id, ci::vr::Controller *controller );
		ci::vr::Controller::ButtonId	getId() const { return mId; }
		ci::vr::Controller::State		getState() const { return mState; }
		bool							isDown() const { return ci::vr::Controller::STATE_DOWN == mState; }
		bool							isUp() const { return ci::vr::Controller::STATE_UP == mState; }
		std::string						getInfo() const;
	private:
		Button( ci::vr::Controller::ButtonId id, ci::vr::Controller *controller ) 
			: ActionState( controller ), mId( id ) {}
		friend class Controller;
		ci::vr::Controller::ButtonId	mId = ci::vr::Controller::BUTTON_UNKNOWN;
		ci::vr::Controller::State		mState = ci::vr::Controller::STATE_UNKNOWN;
		void							setState( ci::vr::Controller::State state );
	};

	//! \class Trigger
	//!
	//!
	class Trigger : public ActionState {
	public:
		virtual ~Trigger() {}
		static TriggerRef				create( ci::vr::Controller::HandId id, ci::vr::Controller *controller );
		ci::vr::Controller::HandId		getId() const { return mId; }
		float							getValue() const { return mValue; }
		std::string						getInfo() const;
	private:
		Trigger( ci::vr::Controller::HandId id, ci::vr::Controller *controller )
			: ActionState( controller ), mId( id ) {}
		friend class Controller;
		ci::vr::Controller::HandId		mId = ci::vr::Controller::HAND_UNKNOWN;
		float							mValue = 0.0f;
		void							setValue( float value );
	};

	//! \class Axis
	//!
	//!
	class Axis : public ActionState {
	public:
		virtual ~Axis() {}
		static AxisRef					create( ci::vr::Controller::HandId id, ci::vr::Controller *controller );
		ci::vr::Controller::HandId		getId() const { return mId; }
		const ci::vec2&					getValue() const { return mValue; }
		std::string						getInfo() const;
	private:
		Axis( ci::vr::Controller::HandId id, ci::vr::Controller *controller )
			: ActionState( controller ), mId( id ) {}
		friend class Controller;
		ci::vr::Controller::HandId		mId = ci::vr::Controller::HAND_UNKNOWN;
		ci::vec2						mValue = ci::vec2( 0.0f );
		void							setValue( const ci::vec2 &value );
	};

	// ---------------------------------------------------------------------------------------------

	virtual ~Controller();

	ci::vr::Context						*getContext() const { return mContext; }
	ci::vr::Controller::Type			getType() const { return mType; }

	ci::vr::Api							getApi() const;
	virtual std::string					getName() const = 0;

	virtual std::string					getButtonName( ci::vr::Controller::ButtonId ) const = 0;
	virtual std::string					getTriggerName( ci::vr::Controller::HandId ) const = 0;
	virtual std::string					getAxisName( ci::vr::Controller::HandId ) const = 0;

	virtual ci::vr::Controller::Button*			getButton( ci::vr::Controller::ButtonId id );
	virtual const ci::vr::Controller::Button*	getButton( ci::vr::Controller::ButtonId id ) const;
	// Hand id is only relevant on controllers that have both left and right inputs, like the Xbox controller.
	virtual ci::vr::Controller::Trigger*		getTrigger( ci::vr::Controller::HandId id = ci::vr::Controller::HAND_ANY );
	virtual const ci::vr::Controller::Trigger*	getTrigger( ci::vr::Controller::HandId id = ci::vr::Controller::HAND_ANY ) const;
	virtual ci::vr::Controller::Axis*			getAxis( ci::vr::Controller::HandId id = ci::vr::Controller::HAND_ANY );
	virtual const ci::vr::Controller::Axis*		getAxis( ci::vr::Controller::HandId id = ci::vr::Controller::HAND_ANY ) const;


	const ci::mat4&						getDeviceToTrackingMatrix() const { return mDeviceToTrackingMatrix; }
	const ci::mat4&						getTrackingToDeviceMatrix() const { return mTrackingToDeviceMatrix; }

	virtual bool						hasInputRay() const { return false; }
	const ci::Ray&						getInputRay() const { return mInputRay; }

protected:
	Controller( ci::vr::Controller::Type type, ci::vr::Context *context );
	
	std::vector<ButtonRef>				mButtons;
	std::vector<TriggerRef>				mTriggers;
	std::vector<AxisRef>				mAxes;
	ci::mat4							mDeviceToTrackingMatrix;
	ci::mat4							mTrackingToDeviceMatrix;
	ci::Ray								mInputRay = ci::Ray( ci::vec3( 0 ), ci::vec3( 0 ) );

	void								setButtonState( ci::vr::Controller::Button *button, ci::vr::Controller::State state );
	void								setTriggerValue( ci::vr::Controller::Trigger *trigger, float value );
	void								setAxisValue( ci::vr::Controller::Axis *axis, const ci::vec2 &value );

private:
	ci::vr::Context						*mContext = nullptr;
	ci::vr::Controller::Type			mType =	ci::vr::Controller::TYPE_UNKNOWN;
};

}} // namespace cinder::vr