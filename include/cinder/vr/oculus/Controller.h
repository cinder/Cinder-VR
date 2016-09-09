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

#if defined( CINDER_VR_ENABLE_OCULUS )

#include <OVR_CAPI.h>

namespace cinder { namespace vr { namespace oculus  {

class Context;

class Controller;
using ControllerRef = std::shared_ptr<Controller>;

//! \class Controller
//!
//!
class Controller : public vr::Controller {
public:

	~Controller();

	::ovrControllerType						getInternalType() const { return mInternalType; }

	virtual ::ovrButton						toOvr( ci::vr::Controller::ButtonId value ) const = 0;
	virtual ci::vr::Controller::ButtonId	fromOvr( ::ovrButton value ) const = 0;

protected:
	Controller( ci::vr::Controller::Type type, ::ovrControllerType internalType, ci::vr::Context *context );
	friend class ci::vr::oculus::Context;

	::ovrControllerType						mInternalType = ::ovrControllerType_None;

	virtual void							processInputState( const ::ovrInputState& state ) = 0;
	virtual void							processButtons( const ::ovrInputState& state );
	virtual void							processTriggers( const ::ovrInputState& state );
	virtual void							processAxes( const ::ovrInputState& state );
};

//! \class ControllerRemote
//!
//!
class ControllerRemote : public Controller {
public:

	~ControllerRemote();

	static ci::vr::oculus::ControllerRef	create( ci::vr::Context *context );

	virtual std::string						getName() const override;
	
	virtual std::string						getButtonName( ci::vr::Controller::ButtonId id ) const;
	virtual std::string						getTriggerName( ci::vr::Controller::TriggerId id ) const;
	virtual std::string						getAxisName( ci::vr::Controller::AxisId id ) const;

	virtual ::ovrButton						toOvr( ci::vr::Controller::ButtonId value ) const;
	virtual ci::vr::Controller::ButtonId	fromOvr( ::ovrButton value ) const;

protected:
	ControllerRemote( ci::vr::Context *context );
	friend class ci::vr::oculus::Context;

	virtual void							processInputState( const ::ovrInputState& state );
};

//! \class ControllerXbox
//!
//!
class ControllerXbox : public Controller {
public:

	~ControllerXbox();

	static ci::vr::oculus::ControllerRef	create( ci::vr::Context *context );

	virtual std::string						getName() const override;

	virtual std::string						getButtonName( ci::vr::Controller::ButtonId id ) const;
	virtual std::string						getTriggerName( ci::vr::Controller::TriggerId id ) const;
	virtual std::string						getAxisName( ci::vr::Controller::AxisId id ) const;

	virtual ::ovrButton						toOvr( ci::vr::Controller::ButtonId value ) const;
	virtual ci::vr::Controller::ButtonId	fromOvr( ::ovrButton value ) const;

protected:
	ControllerXbox( ci::vr::Context *context );
	friend class ci::vr::oculus::Context;

	virtual void							processInputState( const ::ovrInputState& state );
};

//! \class ControllerTouch
//!
//!
class ControllerTouch : public Controller {
public:

	~ControllerTouch();

	static ci::vr::oculus::ControllerRef	create( ci::vr::Controller::Type type, ci::vr::Context *context );
	
	virtual std::string						getName() const override;

	virtual std::string						getButtonName( ci::vr::Controller::ButtonId id ) const;
	virtual std::string						getTriggerName( ci::vr::Controller::TriggerId id ) const;
	virtual std::string						getAxisName( ci::vr::Controller::AxisId id ) const;

	virtual ::ovrButton						toOvr( ci::vr::Controller::ButtonId value ) const;
	virtual ci::vr::Controller::ButtonId	fromOvr( ::ovrButton value ) const;

protected:
	ControllerTouch( ci::vr::Controller::Type type, ci::vr::Context *context );
	friend class ci::vr::oculus::Context;

	virtual void							processInputState( const ::ovrInputState& state );
};

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )