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

#include "cinder/vr/openvr/OpenVr.h"

#if defined( CINDER_VR_ENABLE_OPENVR )

#if defined( CINDER_MSW )
	#include "cinder/msw/CinderMsw.h"
#endif

#include <vector>

namespace cinder { namespace vr { namespace openvr  {

// -------------------------------------------------------------------------------------------------
// RenderModelData
// -------------------------------------------------------------------------------------------------
RenderModelData::RenderModelData( const std::string& name, ::vr::RenderModel_t* model, ::vr::RenderModel_TextureMap_t* texture )
	: mName( name )
{
	// Vertex data vbo
	ci::geom::BufferLayout layout = ci::geom::BufferLayout();
	layout.append( ci::geom::POSITION,    3, sizeof( ::vr::RenderModel_Vertex_t ),(size_t)offsetof( ::vr::RenderModel_Vertex_t , vPosition ),      0 );
	layout.append( ci::geom::NORMAL,      3, sizeof( ::vr::RenderModel_Vertex_t ),(size_t)offsetof( ::vr::RenderModel_Vertex_t , vNormal ),        0 );
	layout.append( ci::geom::TEX_COORD_0, 2, sizeof( ::vr::RenderModel_Vertex_t ),(size_t)offsetof( ::vr::RenderModel_Vertex_t , rfTextureCoord ), 0 );
	ci::gl::VboRef vertexDataVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, model->unVertexCount * sizeof( ::vr::RenderModel_t ), model->rVertexData, GL_STATIC_DRAW );
	std::vector<std::pair<ci::geom::BufferLayout, ci::gl::VboRef>> vertexArrayBuffers = { std::make_pair( layout, vertexDataVbo ) };
	
	// Indices vbo
	ci::gl::VboRef indicesVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, 3 * model->unTriangleCount * sizeof( uint16_t ), model->rIndexData, GL_STATIC_DRAW );

	// Vbo mesh
	mVboMesh = ci::gl::VboMesh::create( static_cast<uint32_t>( model->unVertexCount ), GL_TRIANGLES, vertexArrayBuffers, static_cast<uint32_t>( 3 * model->unTriangleCount ), GL_UNSIGNED_SHORT, indicesVbo );

	// Texture
	ci::gl::Texture::Format texFormat;
	texFormat.setInternalFormat( GL_RGBA );
	texFormat.setWrapS( GL_CLAMP_TO_EDGE );
	texFormat.setWrapT( GL_CLAMP_TO_EDGE );
	texFormat.setMinFilter( GL_LINEAR );
	texFormat.setMagFilter( GL_LINEAR );
	mTexture = ci::gl::Texture::create( texture->rubTextureMapData, GL_RGBA, texture->unWidth, texture->unHeight, texFormat );
}

RenderModelDataRef RenderModelData::create( const std::string& name, ::vr::RenderModel_t* model, ::vr::RenderModel_TextureMap_t* texture )
{
	RenderModelDataRef result = RenderModelDataRef( new RenderModelData( name, model, texture ) );
	return result;
}

// -------------------------------------------------------------------------------------------------
// RenderModel
// -------------------------------------------------------------------------------------------------
RenderModel::RenderModel( const ci::vr::openvr::RenderModelDataRef& data, const ci::gl::GlslProgRef& shader )
	: mData( data ), mShader( shader )
{
	mBatch = ci::gl::Batch::create( data->getVboMesh(), shader );
}

RenderModelRef RenderModel::create( const ci::vr::openvr::RenderModelDataRef& data, const ci::gl::GlslProgRef& shader )
{
	RenderModelRef result = RenderModelRef( new RenderModel( data, shader ) );
	return result;
}

void RenderModel::draw()
{
	mData->getTexture()->bind( 0 );
	mBatch->draw();
	mData->getTexture()->unbind( 0 );
}

// -------------------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------------------

std::string getTrackedDeviceString( ::vr::IVRSystem* vrSystem, ::vr::TrackedDeviceIndex_t trackedDeviceIndex, ::vr::TrackedDeviceProperty prop, ::vr::TrackedPropertyError *peError )
{
	std::string result;
	uint32_t requiredBufferLen = vrSystem->GetStringTrackedDeviceProperty( trackedDeviceIndex, prop, nullptr, 0, peError );
	if( requiredBufferLen > 0 ) {
		std::vector<char> buffer = std::vector<char>( requiredBufferLen );
		std::fill( std::begin( buffer ), std::end( buffer ), 0 );
		requiredBufferLen = vrSystem->GetStringTrackedDeviceProperty( trackedDeviceIndex, prop, buffer.data(), requiredBufferLen, peError );
		result = std::string( reinterpret_cast<const char *>( buffer.data() ) );		
	}
	return result;
}

ci::mat4 getHmdEyeProjectionMatrix( ::vr::IVRSystem* vrSystem, ::vr::Hmd_Eye eye, float nearClip, float farClip )
{
	::vr::HmdMatrix44_t mat = vrSystem->GetProjectionMatrix( eye, nearClip, farClip, ::vr::API_OpenGL );
	return ci::vr::openvr::fromOpenVr( mat );
}

ci::mat4 getHmdEyePoseMatrix( ::vr::IVRSystem* vrSystem, ::vr::Hmd_Eye eye )
{
	::vr::HmdMatrix34_t mat = vrSystem->GetEyeToHeadTransform( eye );
	return glm::affineInverse( ci::vr::openvr::fromOpenVr( mat ) );
}

void threadSleep( unsigned long nMilliseconds )
{
#if defined( CINDER_MSW )
	::Sleep( nMilliseconds );
#elif defined(POSIX)
	usleep( nMilliseconds * 1000 );
#endif
}

}}} // namespace cinder::vr::vive

#endif // defined( CINDER_VR_ENABLE_OPENVR )