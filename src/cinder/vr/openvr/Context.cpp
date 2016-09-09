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

#include "cinder/vr/openvr/Context.h"
#include "cinder/vr/openvr/Controller.h"
#include "cinder/vr/openvr/DeviceManager.h"
#include "cinder/vr/openvr/Hmd.h"
#include "cinder/vr/openvr/OpenVr.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

namespace cinder { namespace vr { namespace openvr {

Context::Context( const ci::vr::SessionOptions& sessionOptions, ci::vr::openvr::DeviceManager* deviceManager )
	: ci::vr::Context( sessionOptions, deviceManager )
{ 
	mDeviceManager = deviceManager;
	mVrSystem = mDeviceManager->getVrSystem();
	
	// Allocate pose and and coord sys mat rices
	mPoses.resize( ::vr::k_unMaxTrackedDeviceCount );
	mDeviceToTrackingMatrices.resize( ::vr::k_unMaxTrackedDeviceCount );
	mTrackingToDeviceMatrices.resize( ::vr::k_unMaxTrackedDeviceCount );

	// These start out with the events disabled
	mViveControllers[ci::vr::Controller::TYPE_LEFT] = ci::vr::openvr::Controller::create( UINT32_MAX, ci::vr::Controller::TYPE_LEFT, this );
	mViveControllers[ci::vr::Controller::TYPE_RIGHT] = ci::vr::openvr::Controller::create( UINT32_MAX, ci::vr::Controller::TYPE_RIGHT, this );
}

Context::~Context()
{
	endSession();
}

ContextRef Context::create( const ci::vr::SessionOptions& sessionOptions, ci::vr::openvr::DeviceManager* deviceManager  )
{
	ContextRef result = ContextRef( new Context( sessionOptions, deviceManager ) );
	return result;
}

bool Context::hasController( const ci::vr::Controller::Type type ) const
{
	bool result = false;
	switch( type ) {
		case ci::vr::Controller::TYPE_LEFT: {
		}
		break;

		case ci::vr::Controller::TYPE_RIGHT: {
		}
		break;

		default: {
			result = ci::vr::Context::hasController( type );
		}
		break;
	}
	return result;
}

ci::vr::Controller *Context::getController( ci::vr::Controller::Type type ) const
{
	ci::vr::Controller *result = nullptr;
	return result;
}

void Context::scanForControllers()
{
	// Not used. OpenVR uses events to handle controller connections.
}

void Context::updatePoseData()
{
	::vr::VRCompositor()->WaitGetPoses( mPoses.data(), ::vr::k_unMaxTrackedDeviceCount, nullptr, 0 );

	for( ::vr::TrackedDeviceIndex_t deviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex )	{
		if( mPoses[deviceIndex].bPoseIsValid ) {
			mDeviceToTrackingMatrices[deviceIndex] = ci::vr::openvr::fromOpenVr( mPoses[deviceIndex].mDeviceToAbsoluteTracking );
			mTrackingToDeviceMatrices[deviceIndex] = glm::affineInverse( mDeviceToTrackingMatrices[deviceIndex] );
		}
	}

	// Update controller input rays
	for( ::vr::TrackedDeviceIndex_t deviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex ) {
		if( ::vr::TrackedDeviceClass_Controller != mVrSystem->GetTrackedDeviceClass( deviceIndex ) ) {
			continue;
		}

		ci::vr::Controller::Type ctrlType = ci::vr::Controller::TYPE_UNKNOWN;
		::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
		switch( role ) {
			case ::vr::TrackedControllerRole_LeftHand  : ctrlType = ci::vr::Controller::TYPE_LEFT; break;
			case ::vr::TrackedControllerRole_RightHand : ctrlType = ci::vr::Controller::TYPE_RIGHT; break;
		}

		if( ( ci::vr::Controller::TYPE_UNKNOWN != ctrlType ) && ( mPoses[deviceIndex].bPoseIsValid ) && mViveControllers[ctrlType]->isEventsEnabled() ) {
			const ci::mat4& inverseLookMatrix = mHmd->getInverseLookMatrix();
			const ci::mat4& inverseOriginMatrix = mHmd->getInverseOriginMatrix();
			const ci::mat4& deviceToTracking = getDeviceToTrackingMatrix( deviceIndex );
			const ci::mat4& trackingToDevice = getDeviceToTrackingMatrix( ::vr::k_unTrackedDeviceIndex_Hmd );
			mViveControllers[ctrlType]->processControllerPose( inverseLookMatrix, inverseOriginMatrix, deviceToTracking, trackingToDevice );
		}
	}
}

void Context::beginSession()
{
	// Create HMD
	mHmd = ci::vr::openvr::Hmd::create( this );

	// Set frame rate for VR
	ci::gl::enableVerticalSync( getSessionOptions().getVerticalSync() );
	ci::app::setFrameRate( getSessionOptions().getFrameRate() );

	// Get connected controllers
	updateControllerConnections();
}

void Context::endSession()
{
}

void Context::updateControllerConnections()
{
	for( ::vr::TrackedDeviceIndex_t deviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex ) {
		if( ::vr::TrackedDeviceClass_Controller == mVrSystem->GetTrackedDeviceClass( deviceIndex ) ) {
			ci::vr::Controller::Type ctrlType = ci::vr::Controller::TYPE_UNKNOWN;
			::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
			switch( role ) {
				case ::vr::TrackedControllerRole_LeftHand  : ctrlType = ci::vr::Controller::TYPE_LEFT; break;
				case ::vr::TrackedControllerRole_RightHand : ctrlType = ci::vr::Controller::TYPE_RIGHT; break;
				default: break;
			}

			if( ci::vr::Controller::TYPE_UNKNOWN != ctrlType ) {
				mViveControllers[ctrlType]->setEventsEnabled();
				addController( mViveControllers[ctrlType] );
				getSignalControllerConnected().emit( mViveControllers[ctrlType].get() );
			}
		}
	}
}

void Context::processTrackedDeviceEvents( const ::vr::VREvent_t &event )
{
	switch( event.eventType ) {
		case ::vr::VREvent_TrackedDeviceActivated: {
			CI_LOG_D( "EVENT: VREvent_TrackedDeviceActivated" );
			// Activate the render models in the HMD in case they need to be drawn.
			auto hmd = std::dynamic_pointer_cast<ci::vr::openvr::Hmd>( mHmd );
			if( hmd ) {
				hmd->activateRenderModel( event.trackedDeviceIndex );
			}

			/*
			// Enable, add, and connect the controller specified by event.trackedDeviceIndex.
			if( ::vr::TrackedDeviceClass_Controller == mVrSystem->GetTrackedDeviceClass( event.trackedDeviceIndex ) ) {
				ci::vr::Controller::HandId handId = ci::vr::Controller::HAND_UNKNOWN;
				::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( event.trackedDeviceIndex );
				switch( role ) {
					case ::vr::TrackedControllerRole_LeftHand  : handId = ci::vr::Controller::HAND_LEFT; break;
					case ::vr::TrackedControllerRole_RightHand : handId = ci::vr::Controller::HAND_RIGHT; break;
					default: break;
				}

				if( ci::vr::Controller::HAND_UNKNOWN != handId ) {
					mViveControllers[handId]->setEventsEnabled();
					addController( mViveControllers[handId] );
					getSignalControllerConnected().emit( mViveControllers[handId].get() );
				}
			}
			*/

			// Disconnect, remove, and disable all controllers
			for( auto& ctrlIt : mViveControllers ) {
				auto& ctrl = ctrlIt.second;
				getSignalControllerDisconnected().emit( ctrl.get() );
				removeController( ctrl );
				ctrl->setEventsEnabled( false );
				ctrl->clearInputRay();
			}
			// Rescan for controllers
			updateControllerConnections();
		}
		break;

		case ::vr::VREvent_TrackedDeviceDeactivated: {
			CI_LOG_D( "EVENT: VREvent_TrackedDeviceDeactivated" );
			// Disconnect, remove, and disable the controller specified by event.trackedDeviceIndex.
			if( ::vr::TrackedDeviceClass_Controller == mVrSystem->GetTrackedDeviceClass( event.trackedDeviceIndex ) ) {
				ci::vr::Controller::Type ctrlType = ci::vr::Controller::TYPE_UNKNOWN;
				::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( event.trackedDeviceIndex );
				switch( role ) {
					case ::vr::TrackedControllerRole_LeftHand  : ctrlType = ci::vr::Controller::TYPE_LEFT; break;
					case ::vr::TrackedControllerRole_RightHand : ctrlType = ci::vr::Controller::TYPE_RIGHT; break;
					default: break;
				}

				if( ci::vr::Controller::TYPE_UNKNOWN != ctrlType ) {
					getSignalControllerDisconnected().emit( mViveControllers[ctrlType].get() );
					removeController( mViveControllers[ctrlType] );
					mViveControllers[ctrlType]->setEventsEnabled( false );
					mViveControllers[ctrlType]->clearInputRay();
				}
			}
		}
		break;

		case ::vr::VREvent_TrackedDeviceUserInteractionStarted: {
			CI_LOG_D( "EVENT: VREvent_TrackedDeviceUserInteractionStarted" );
			// Disconnect, remove, and disable all controllers
			for( auto& ctrlIt : mViveControllers ) {
				auto& ctrl = ctrlIt.second;
				getSignalControllerDisconnected().emit( ctrl.get() );
				removeController( ctrl );
				ctrl->setEventsEnabled( false );
				ctrl->clearInputRay();
			}
			// Rescan for controllers
			updateControllerConnections();
		}
		break;

		case ::vr::VREvent_TrackedDeviceUserInteractionEnded: {
		}
		break;

		case ::vr::VREvent_TrackedDeviceRoleChanged: {
			CI_LOG_D( "EVENT: VREvent_TrackedDeviceRoleChanged" );
			// Disconnect, remove, and disable all both controllers
			for( auto& ctrlIt : mViveControllers ) {
				auto& ctrl = ctrlIt.second;
				getSignalControllerDisconnected().emit( ctrl.get() );
				removeController( ctrl );
				ctrl->setEventsEnabled( false );
				ctrl->clearInputRay();
			}
			// Rescan for controllers
			updateControllerConnections();
		}
		break;

		default: break;
	}

	// Process input
	for( ::vr::TrackedDeviceIndex_t deviceIndex = ::vr::k_unTrackedDeviceIndex_Hmd; deviceIndex < ::vr::k_unMaxTrackedDeviceCount; ++deviceIndex ) {
		if( ::vr::TrackedDeviceClass_Controller != mVrSystem->GetTrackedDeviceClass( deviceIndex ) ) {
			continue;
		}

		ci::vr::Controller::Type ctrlType = ci::vr::Controller::TYPE_UNKNOWN;
		::vr::ETrackedControllerRole role = mVrSystem->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
		switch( role ) {
			case ::vr::TrackedControllerRole_LeftHand  : ctrlType = ci::vr::Controller::TYPE_LEFT; break;
			case ::vr::TrackedControllerRole_RightHand : ctrlType = ci::vr::Controller::TYPE_RIGHT; break;
			default: break;
		}

		if( ci::vr::Controller::TYPE_UNKNOWN != ctrlType ) {
			::vr::VRControllerState_t state = {};
			if( mVrSystem->GetControllerState( deviceIndex, &state ) && mViveControllers[ctrlType]->isEventsEnabled() ) {
				mViveControllers[ctrlType]->processControllerState( state );
			}
		}
	}
}

void Context::processEvents()
{
	::vr::VREvent_t event = {};
	while( mVrSystem->PollNextEvent( &event, sizeof( ::vr::VREvent_t ) ) ) {
		processTrackedDeviceEvents( event );
	}
}

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )