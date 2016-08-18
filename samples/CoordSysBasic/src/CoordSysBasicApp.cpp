#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/Log.h"
#include "cinder/Sphere.h"
#include "cinder/Text.h"

#include "cinder/vr/vr.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//! \class CoordSysBasicApp
//!
//!
class CoordSysBasicApp : public App {
public:
	void	setup() override;
	void	update() override;
	void	draw() override;

private:
	ci::vr::Context				*mVrContext = nullptr;
	ci::vr::Hmd					*mHmd = nullptr;
	CameraPersp					mCamera;

	bool						mRecalcOrigin = false;

	const ci::vr::Controller	*mController1 = nullptr;
	const ci::vr::Controller	*mController2 = nullptr;

	gl::BatchRef				mGridBox;
	gl::BatchRef				mGridLines;

	gl::TextureRef				mWorldLabelTex;
	gl::BatchRef				mWorldLabelBatch;

	gl::TextureRef				mTrackingLabelTex;
	gl::BatchRef				mTrackingLabelBatch;

	void	initGrid( const gl::GlslProgRef &shader );

	void	drawScene();
};

void CoordSysBasicApp::initGrid( const gl::GlslProgRef &shader )
{
	vec3 size = vec3( 100, 20, 100 );

	// Box
	{
		mGridBox = gl::Batch::create( geom::Cube().size( 1.01f*size ) >> geom::Translate( 0.0f, 0.0f, 0.0f ), shader );
	}

	// Lines
	{
		Rectf bounds = Rectf( -size.x/2.0f, -size.z/2.0f, size.x/2.0f, size.z/2.0f );
		int nx = static_cast<int>( size.x );
		int nz = static_cast<int>( size.z );
		float dx = bounds.getWidth() / static_cast<float>( nx );
		float dz = bounds.getHeight() / static_cast<float>( nz );
		float y = 0.01f;

		std::vector<vec3> vertices;

		for( int i = 0; i <= nx; ++i ) {
			float x = bounds.x1 + i * dx;
			vec3 p0 = vec3( x, y, bounds.y1 );
			vec3 p1 = vec3( x, y, bounds.y2 );
			vertices.push_back( p0 );
			vertices.push_back( p1 );
		}

		for( int i = 0; i <= nz; ++i ) {
			float z = bounds.y1 + i * dz;
			vec3 p0 = vec3( bounds.x1, y, z );
			vec3 p1 = vec3( bounds.x2, y, z );
			vertices.push_back( p0 );
			vertices.push_back( p1 );
		}

		gl::VboRef vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices );
		geom::BufferLayout layout = geom::BufferLayout( { geom::AttribInfo( geom::Attrib::POSITION, 3, sizeof( vec3 ), 0 ) } );
		gl::VboMeshRef vboMesh = gl::VboMesh::create( static_cast<uint32_t>( vertices.size() ), GL_LINES, { std::make_pair( layout, vbo ) } );
		mGridLines = gl::Batch::create( vboMesh, shader );
	}
}

void CoordSysBasicApp::setup()
{
	mCamera.lookAt( vec3( 0, 0, 3 ), vec3( 0, 0, 0 ) );

	gl::disableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color( Color::white() );

	try {
		ci::vr::initialize();
	}
	catch( const std::exception& e ) {
		CI_LOG_E( "VR failed: " << e.what() );
	}

	try {
		mVrContext = ci::vr::beginSession( 
			ci::vr::SessionOptions()
				.setTrackingOrigin( ci::vr::TRACKING_ORIGIN_SEATED )
				.setOriginOffset( vec3( 0, -1, -3 ) )
				.setControllersScanInterval( 0.25f )
		);
	}
	catch( const std::exception& e ) {
		CI_LOG_E( "Session failed: " << e.what() );
	}

	try {
		mHmd = mVrContext->getHmd();
	}
	catch( const std::exception& e ) {
		CI_LOG_E( "Hmd failed: " << e.what() );
	}

	auto shader = gl::getStockShader( gl::ShaderDef().color() );
	initGrid( shader );

	shader = gl::getStockShader( gl::ShaderDef().texture() );
	// World
	{
		TextBox tb;
		tb.setBackgroundColor( Color( 1, 1, 1 ) );
		tb.setColor( Color( 0, 0, 0 ) );
		tb.setFont( Font( "Arial", 100.0f ) );
		tb.appendText( "World Origin" );
		mWorldLabelTex = gl::Texture2d::create( tb.render() );
		float ratio = mWorldLabelTex->getAspectRatio();
		mWorldLabelBatch = gl::Batch::create( geom::Plane().axes( vec3( 1, 0, 0 ), vec3( 0, 1, 0 ) ).size( vec2( ratio, 1.0f ) ) >> geom::Scale( 0.2f ) >> geom::Translate( 0.0f, 1.0f + 0.1f ), shader );
	}

	// Tracking
	{
		TextBox tb;
		tb.setBackgroundColor( Color( 1, 1, 1 ) );
		tb.setColor( Color( 0, 0, 0 ) );
		tb.setFont( Font( "Arial", 100.0f ) );
		tb.appendText( "Tracking Origin" );
		mTrackingLabelTex = gl::Texture2d::create( tb.render() );
		float ratio = mTrackingLabelTex->getAspectRatio();
		mTrackingLabelBatch = gl::Batch::create( geom::Plane().axes( vec3( 1, 0, 0 ), vec3( 0, 1, 0 ) ).size( vec2( ratio, 1.0f ) ) >> geom::Scale( 0.2f ) >> geom::Translate( 0.0f, 1.0f + 0.1f ), shader );
	}
} 

void CoordSysBasicApp::update()
{
	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if( ( ci::vr::API_OPENVR == mVrContext->getApi() ) && ( ! mRecalcOrigin ) && ( mHmd->getElapsedFrames() > 60 ) ) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}
}

void CoordSysBasicApp::drawScene()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();			
	// Draw ground grid
	{
		gl::ScopedLineWidth scopedLineWidth( 1.0f );

		gl::color( 0.8f, 0.8f, 0.8f );
		mGridBox->draw();

		gl::color( 0.7f, 0.7f, 0.7f );
		mGridLines->draw();
	}

	if( ci::vr::API_OCULUS == mVrContext->getApi() ) {
	}
	else if( ci::vr::API_OPENVR == mVrContext->getApi() ) {
	}

	// Draw coordinate frame
	{
		gl::drawCoordinateFrame( 1.0f );
	}
}

void CoordSysBasicApp::draw()
{
	gl::clear( Color( 0.02f, 0.02f, 0.1f ) );

	float t = static_cast<float>( getElapsedSeconds() );

	if( mHmd ) {
		mHmd->bind();

		for( auto eye : mHmd->getEyes() ) {
			mHmd->enableEye( eye, ci::vr::COORD_SYS_WORLD );	

			// Draw this in world coordinates
			drawScene();
			
			// Draw a red sphere in world coordinates
			{
				gl::color( 1, 0, 0 );
				gl::drawSphere( vec3( 0, 0, 0 ), 0.10f );

				gl::ScopedModelMatrix scopedModelMatrix;
				gl::rotate( t, vec3( 0, 1, 0 ) );
				mWorldLabelTex->bind( 0 );
				mWorldLabelBatch->getGlslProg()->uniform( "uTex0", 0 );
				mWorldLabelBatch->draw();
				mWorldLabelTex->unbind( 0 );
			}

			// Draw a blue sphere in tracking coordinates
			{
				mHmd->setMatricesEye( eye, ci::vr::COORD_SYS_TRACKING );

				gl::drawCoordinateFrame( 1.0f );

				gl::color( 0, 0, 1 );
				gl::drawSphere( vec3( 0, 0, 0 ), 0.15f );

				gl::ScopedModelMatrix scopedModelMatrix;
				gl::rotate( t + 1.57f, vec3( 0, 1, 0 ) );
				mTrackingLabelTex->bind( 0 );
				mTrackingLabelBatch->getGlslProg()->uniform( "uTex0", 0 );
				mTrackingLabelBatch->draw();
				mTrackingLabelTex->unbind( 0 );
			}

			// Draw a green sphere in device coordinates - this will be locked to the HMD
			{
				mHmd->setMatricesEye( eye, ci::vr::COORD_SYS_DEVICE );
				gl::color( 0, 1, 0 );
				gl::drawSphere( vec3( 0, 0, -4 ), 0.15f );
			}
		}
		mHmd->unbind();

		// Draw mirrored
		gl::viewport( getWindowSize() );
		gl::setMatricesWindow( getWindowSize() );
		mHmd->drawMirrored( getWindowBounds() );

		// Submit frame
		mHmd->submitFrame();
	}
	else {
		gl::viewport( getWindowSize() );
		gl::setMatrices( mCamera );
		drawScene();
	}
}

void prepareSettings( App::Settings *settings )
{
	settings->setTitle( "Cinder VR CoordSysBasic" );
	settings->setWindowSize( 1920/2, 1080/2 );
}

CINDER_APP( CoordSysBasicApp, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )
