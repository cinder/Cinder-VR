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
#include "cinder/CinderGlm.h"
#include "cinder/CinderMath.h"

#if defined( CINDER_VR_ENABLE_OCULUS )

#include <OVR_CAPI.h>

namespace cinder { namespace vr { namespace oculus  {

class Exception : public ci::vr::Exception {
public:
	Exception() {}
	Exception( const std::string& msg ) : ci::vr::Exception( msg ) {}
	virtual ~Exception() {}
};

void checkResult( const ::ovrResult& result );

::ovrControllerType toOvr( ci::vr::Controller::Type type );

// -------------------------------------------------------------------------------------------------
// Conversions
// -------------------------------------------------------------------------------------------------
inline glm::ivec2 fromOvr( const ovrSizei& v )
{
	return glm::ivec2( v.w, v.h );
}

inline glm::ivec2 fromOvr( const ovrVector2i& v )
{
	return glm::ivec2( v.x, v.y );
}

inline glm::vec2 fromOvr( const ovrVector2f& v )
{
	return glm::vec2( v.x, v.y );
}

inline glm::vec3 fromOvr( const ovrVector3f& v )
{
	return glm::vec3( v.x, v.y, v.z );
}

inline glm::mat4 fromOvr( const ovrMatrix4f& m )
{
	return glm::transpose( glm::make_mat4( &m.M[0][0] ) );
}

inline glm::quat fromOvr( const ovrQuatf& q )
{
	return glm::quat( q.w, q.x, q.y, q.z );
}

inline glm::mat4 fromOvr( const ovrPosef& p )
{
	glm::mat4 rotationMatrix = glm::toMat4( fromOvr( p.Orientation ) );
	glm::mat4 positionMatrix = glm::translate( fromOvr( p.Position ) );
	return positionMatrix * rotationMatrix;
}

inline std::pair<glm::ivec2, glm::ivec2> fromOvr( const ovrRecti& r )
{
	return std::pair<glm::ivec2, glm::ivec2>( fromOvr( r.Pos ), fromOvr( r.Size ) );
}

inline ovrMatrix4f toOvr( const glm::mat4& m )
{
	ovrMatrix4f result;
	glm::mat4 mat = glm::transpose( m );
	memcpy( result.M, &mat, sizeof( glm::mat4 ) );
	return result;
}

inline ovrVector3f toOvr( const glm::vec3& v )
{
	ovrVector3f result;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	return result;
}

inline ovrVector2f toOvr( const glm::vec2& v )
{
	ovrVector2f result;
	result.x = v.x;
	result.y = v.y;
	return result;
}

inline ovrSizei toOvr( const glm::ivec2& v )
{
	ovrSizei result;
	result.w = v.x;
	result.h = v.y;
	return result;
}

inline ovrQuatf toOvr( const glm::quat& q )
{
	ovrQuatf result;
	result.x = q.x;
	result.y = q.y;
	result.z = q.z;
	result.w = q.w;
	return result;
}

}}} // namespace cinder::vr::oculus

#endif // defined( CINDER_VR_ENABLE_OCULUS )