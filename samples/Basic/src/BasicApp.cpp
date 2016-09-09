#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

#include "cinder/vr/vr.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void resize() override;
	void update() override;
	void draw() override;

private:
	ci::vr::Context*	mVrContext = nullptr;
	ci::vr::Hmd*		mHmd = nullptr;

	double				mTime;

	CameraPersp			mCamera;

	gl::GlslProgRef		mShader;
	gl::BatchRef		mTeapot;

	vec3				mViewerPosition;
	vec4				mLightWorldPosition;

	// Only applies to HTC Vive with standing tracking origin
	bool				mRecalcOrigin = false;

	void drawScene();
};

void BasicApp::setup()
{
	mCamera.lookAt( ci::vec3( 0, 0, 3 ), ci::vec3( 0, 0, 0 ) );

	mShader = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "phong.vert" ) ).fragment( loadAsset( "phong.frag" ) ) );
	mTeapot = gl::Batch::create( geom::Teapot().subdivisions( 12 ), mShader );

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
		mVrContext = ci::vr::beginSession( ci::vr::SessionOptions().setOriginOffset( vec3( 0, 0, -3 ) ) );
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
} 

void BasicApp::mouseDown( MouseEvent event )
{
}

void BasicApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void BasicApp::update()
{
	// Animate light position.
	mTime = getElapsedSeconds();
	float t = float( mTime ) * 0.4f;
	mLightWorldPosition = vec4( math<float>::sin( t ), math<float>::sin( t * 4.0f ), math<float>::cos( t ), 1 );

	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if( ( ci::vr::API_OPENVR == mVrContext->getApi() ) && ( ! mRecalcOrigin ) && ( mHmd->getElapsedFrames() > 60 ) ) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}
}

void BasicApp::drawScene()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();

	{
		gl::ScopedModelMatrix push;
		gl::rotate( (float)mTime, vec3( -0.3f, -1.0f, 0.2f ) );
		gl::scale( vec3( 0.5f ) );
		mTeapot->draw();
	}

	gl::lineWidth( 3.0f );
	gl::drawCoordinateFrame( 2 );
	gl::drawSphere( vec3( mLightWorldPosition ), 0.05f, 36 );
}

void BasicApp::draw()
{
	gl::clear( Color( 0.02f, 0.02f, 0.1f ) );
	if( mHmd ) {
		mHmd->bind();
		{
			for( auto eye : mHmd->getEyes() ) {
				mHmd->enableEye( eye );
		
				mShader->uniform( "uLightPosition", mLightWorldPosition );
				mShader->uniform( "uSkyDirection", vec4( 0, 1, 0, 0 ) );

				drawScene();

				mHmd->drawControllers( eye );
			}
		}
		mHmd->unbind();

		// Draw mirrored
		gl::viewport( getWindowSize() );
		gl::setMatricesWindow( getWindowSize() );
		mHmd->drawMirrored( getWindowBounds(), true );
	}
	else {
		gl::viewport( getWindowSize() );
		gl::setMatrices( mCamera );

		const mat4& view = mCamera.getViewMatrix();
		mShader->uniform( "uLightViewPosition", view * mLightWorldPosition );
		mShader->uniform( "uSkyDirection", view * vec4( 0, 1, 0, 0 ) );

		drawScene();
	}
}

void prepareSettings( App::Settings *settings )
{
	settings->setTitle( "Cinder VR Basic" );
	settings->setWindowSize( 1920/2, 1080/2 );
}

CINDER_APP( BasicApp, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )
