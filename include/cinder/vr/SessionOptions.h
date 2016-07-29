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

#include <algorithm>

namespace cinder { namespace vr { 

class Controller;

//! \class SessionOptions
//!
//!
class SessionOptions {
public:
	SessionOptions() {}
	virtual ~SessionOptions() {}

	bool								getVerticalSync() const { return mVerticalSync; }
	SessionOptions&						setVerticalSync( bool value ) { mVerticalSync = value; return *this; }

	// NOTE: On some devices, setting the frame rate above the hardware spec can actually slow it down.
	float								getFrameRate() const { return mFrameRate; }
	SessionOptions&						setFrameRate( float value ) { mFrameRate = value; return *this; }

	ci::vr::TrackingOrigin				getTrackingOrigin() const { return mTrackingOrigin; }
	SessionOptions&						setTrackingOrigin( ci::vr::TrackingOrigin value ) { mTrackingOrigin = value; return *this; }

	ci::vr::OriginMode					getOriginMode() const { return mOriginMode; }
	SessionOptions&						setOriginMode( ci::vr::OriginMode value ) { mOriginMode = value; return *this; }

	const ci::vec3&						getOriginOffset() const { return mOriginOffset; }
	SessionOptions&						setOriginOffset( const ci::vec3 &value ) { mOriginOffset = value; return *this; }

	uint32_t							getSampleCount() const { return mSampleCount; }
	SessionOptions&						setSampleCount( uint32_t value ) { mSampleCount = value; return *this; }

	uint32_t							getMipLevels() const { return mMipLevels; }
	SessionOptions&						setMipLevels( uint32_t value ) { mMipLevels = value; return *this; }

	float								getNearClip() const { return mNearClip; }
	SessionOptions&						setNearClip( float value ) { mNearClip = value; return *this; }

	float								getFarClip() const { return mFarClip; }
	SessionOptions&						setFarClip( float value ) { mFarClip = value; return *this; }

	std::pair<float, float>				getClip() const { return std::make_pair( mNearClip, mFarClip ); }
	SessionOptions&						setClip( float nearClip, float farClip ) { mNearClip = nearClip; mFarClip = farClip; return *this; }

	double								getControllersScanInterval() const { return mControllersScanInterval; }
	SessionOptions&						setControllersScanInterval( double value ) { mControllersScanInterval = std::max( value, 0.0 ); return *this; }

	std::function<void(const ci::vr::Controller*)>	getControllerConnected() const { return mControllerConnected; }
	SessionOptions&									setControllerConnected( std::function<void(const ci::vr::Controller*)> value ) {  mControllerConnected = value; return *this; }
	std::function<void(const ci::vr::Controller*)>	getControllerDisconnected() const { return mControllerDisconnected; }
	SessionOptions&									setControllerDisconnected( std::function<void(const ci::vr::Controller*)> value ) {  mControllerDisconnected = value; return *this; }

private:
	bool								mVerticalSync = false;
	float								mFrameRate = 90.0f;

	ci::vr::TrackingOrigin				mTrackingOrigin = ci::vr::TRACKING_ORIGIN_DEVICE_DEFAULT;
	ci::vr::OriginMode					mOriginMode = ci::vr::ORIGIN_MODE_HMD_ORIENTED;
	ci::vec3							mOriginOffset = ci::vec3( 0, 0, -1 );

	uint32_t							mSampleCount = 1;
	uint32_t							mMipLevels = 1;

	float								mNearClip = 0.1f;
	float								mFarClip = 100.0f;

	// Default: 0 sec - no scans after initial scan at startup
	double											mControllersScanInterval = 0.0f;
	std::function<void(const ci::vr::Controller*)>	mControllerConnected;
	std::function<void(const ci::vr::Controller*)>	mControllerDisconnected;
};

}} // namespace cinder::vr