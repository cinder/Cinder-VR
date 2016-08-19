#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "cinder/vr/vr.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ControllerBasicApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;

private:
	ci::vr::Context*			mVrContext = nullptr;
	ci::vr::Hmd*				mHmd = nullptr;
	CameraPersp					mCamera;

	bool						mRecalcOrigin = false;

	gl::BatchRef				mGridBox;
	gl::BatchRef				mGridLines;

	const ci::vr::Controller	*mOculusXbox = nullptr;
	const ci::vr::Controller	*mOculusRemote = nullptr;

	const ci::vr::Controller	*mViveLeft = nullptr;
	const ci::vr::Controller	*mViveRight = nullptr;

	void	onControllerConnect( const ci::vr::Controller *controller );
	void	onControllerDisconnect( const ci::vr::Controller *controller );
	void	onButtonDown( const ci::vr::Controller::Button *button );
	void	onButtonUp( const ci::vr::Controller::Button *button );
	void	onTrigger( const ci::vr::Controller::Trigger *trigger );
	void	onAxis( const ci::vr::Controller::Axis *axis );

	void	initGrid( const gl::GlslProgRef &shader );

	void	drawOculusXbox( const ci::vr::Controller *controller );
	void	drawOcculusRemote( const ci::vr::Controller *controller );
	void	drawViveController( const ci::vr::Controller *controller );
	void	drawScene();
};

void ControllerBasicApp::initGrid( const gl::GlslProgRef &shader )
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

void ControllerBasicApp::setup()
{
	mCamera.lookAt( ci::vec3( 0, 0, 3 ), ci::vec3( 0, 0, 0 ) );

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
				.setControllerConnected( std::bind( &ControllerBasicApp::onControllerConnect, this, std::placeholders::_1 ) )
				.setControllerDisconnected( std::bind( &ControllerBasicApp::onControllerDisconnect, this, std::placeholders::_1 ) )
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

	mVrContext->getSignalControllerButtonDown().connect( std::bind( &ControllerBasicApp::onButtonDown, this, std::placeholders::_1 ) );
	mVrContext->getSignalControllerButtonUp().connect( std::bind( &ControllerBasicApp::onButtonUp, this, std::placeholders::_1 ) );
	mVrContext->getSignalControllerTrigger().connect( std::bind( &ControllerBasicApp::onTrigger, this, std::placeholders::_1 ) );
	mVrContext->getSignalControllerAxis().connect( std::bind( &ControllerBasicApp::onAxis, this, std::placeholders::_1 ) );

	auto shader = gl::getStockShader( gl::ShaderDef().color() );
	initGrid( shader );
} 

void ControllerBasicApp::onControllerConnect( const ci::vr::Controller* controller )
{
	if( ! controller ) {
		return;
	}

	bool connected = false;
	if( ci::vr::API_OCULUS == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_REMOTE: {
				mOculusRemote = controller;
				connected = true;
			}
			break;

			case ci::vr::Controller::TYPE_XBOX: {
				mOculusXbox = controller;
				connected = true;
			}
			break;
		}
	}
	else if( ci::vr::API_OPENVR == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_LEFT: {
				mViveLeft = controller;
				connected = true;
			}
			break;

			case ci::vr::Controller::TYPE_RIGHT: {
				mViveRight = controller;
				connected = true;
			}
			break;
		}
	}

	if( connected ) {
		CI_LOG_I( "Controller connected: " << controller->getName() );
	}
}

void ControllerBasicApp::onControllerDisconnect( const ci::vr::Controller* controller )
{
	if( ! controller ) {
		return;
	}

	bool disconnected = false;
	if( ci::vr::API_OCULUS == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_REMOTE: {
				mOculusRemote = nullptr;
				disconnected = true;
			}
			break;

			case ci::vr::Controller::TYPE_XBOX: {
				mOculusXbox = nullptr;
				disconnected = true;
			}
			break;
		}
	}
	else if( ci::vr::API_OPENVR == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_LEFT: {
				mViveLeft = nullptr;
				disconnected = true;
			}
			break;

			case ci::vr::Controller::TYPE_RIGHT: {
				mViveRight = nullptr;
				disconnected = true;
			}
			break;
		}
	}

	if( disconnected ) {
		CI_LOG_I( "Controller disconnected: " << controller->getName() );
	}
}

void ControllerBasicApp::onButtonDown( const ci::vr::Controller::Button *button )
{
	CI_LOG_I( button->getInfo() );
}

void ControllerBasicApp::onButtonUp( const ci::vr::Controller::Button *button )
{
	CI_LOG_I( button->getInfo() );
}

void ControllerBasicApp::onTrigger( const ci::vr::Controller::Trigger *trigger )
{
	CI_LOG_I( trigger->getInfo() );
}

void ControllerBasicApp::onAxis( const ci::vr::Controller::Axis *axis )
{
	CI_LOG_I( axis->getInfo() );
}

void ControllerBasicApp::update()
{
	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if( ( ci::vr::API_OPENVR == mVrContext->getApi() ) && ( ! mRecalcOrigin ) && ( mHmd->getElapsedFrames() > 60 ) ) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}
}

void drawButton( const vec2& pos, float radius, const ci::vr::Controller::Button *button, const Color& activeColor )
{
	if( button->isDown() ) {
		gl::color( activeColor );
		gl::drawSolidCircle( pos, radius, 16 );
	}
	else {
		gl::color( 0.3f, 0.3f, 0.3f );
		gl::drawStrokedCircle( pos, radius, 16 );
	}
}

void drawTrigger( const vec2& pos, float radius, const ci::vr::Controller::Trigger *trigger, const Color& activeColor )
{
	float s = mix( 0.25f, 1.0f, trigger->getValue() );

	gl::color( s*activeColor );
	gl::drawSolidCircle( pos, s*radius, 16 );
} 

void drawAxis( const vec2& pos, float radius, const ci::vr::Controller::Axis *axis, const Color& activeColor )
{
	ci::vec2 dv = axis->getValue();
	float minorRadius = 0.2f*radius;

	gl::color( activeColor );
	gl::drawSolidCircle( pos + dv*radius, minorRadius, 16 );
}

void ControllerBasicApp::drawOculusXbox( const ci::vr::Controller *controller )
{
	if( ! controller ) {
		return;
	}

	// XYAB
	drawButton( vec2( 0.6f, 0.7f ) + vec2( -0.17f, 0 ), 0.06f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_X ), Color( 0.0f, 0.0f, 0.9f ) );
	drawButton( vec2( 0.6f, 0.7f ) + vec2(  0.17f, 0 ), 0.06f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_B ), Color( 0.9f, 0.0f, 0.0f ) );
	drawButton( vec2( 0.6f, 0.7f ) + vec2( 0,  0.17f ), 0.06f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_Y ), Color( 0.9f, 0.9f, 0.0f ) );
	drawButton( vec2( 0.6f, 0.7f ) + vec2( 0, -0.17f ), 0.06f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_A ), Color( 0.0f, 0.9f, 0.0f ) );

	// DPad
	drawButton( vec2( -0.4f, 0.2f ) + vec2( -0.16f, 0 ), 0.04f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_LEFT ),  Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( -0.4f, 0.2f ) + vec2(  0.16f, 0 ), 0.04f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_RIGHT ), Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( -0.4f, 0.2f ) + vec2( 0,  0.16f ), 0.04f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_UP ),    Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( -0.4f, 0.2f ) + vec2( 0, -0.16f ), 0.04f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_DOWN ),  Color( 0.9f, 0.9f, 0 ) );
	
	// Left/right thumbstick buttons
	drawButton( vec2( -0.90f, 0.7f ), 0.21f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_LTHUMBSTICK ), Color( 0.3f, 0.3f, 0.4f ) );
	drawButton( vec2(  0.40f, 0.2f ), 0.21f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_RTHUMBSTICK ), Color( 0.3f, 0.3f, 0.4f ) );

	// Left/right shoulder buttons
	drawButton( vec2( -0.90f, 1.15f ), 0.1f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_LSHOULDER ), Color( 0.9f, 0.4f, 0.0f ) );
	drawButton( vec2(  0.60f, 1.15f ), 0.1f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_RSHOULDER ), Color( 0.9f, 0.4f, 0.0f ) );

	// Enter/Back
	drawButton( vec2(  0.15f, 0.7f ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_ENTER ), Color( 0, 0.8f, 0.8f ) );
	drawButton( vec2( -0.19f, 0.7f ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_XBOX_BACK ), Color( 0, 0.8f, 0.8f ) );

	// Left/right triggers
	drawTrigger( vec2( -0.90f, 1.45f ), 0.12f, controller->getTrigger( ci::vr::Controller::TRIGGER_OCULUS_XBOX_LEFT ), Color( 0.9f, 0.1f, 0.9f ) );
	drawTrigger( vec2(  0.60f, 1.45f ), 0.12f, controller->getTrigger( ci::vr::Controller::TRIGGER_OCULUS_XBOX_RIGHT ), Color( 0.9f, 0.1f, 0.9f ) );

	// Left/right thumbsticks
	{
		gl::ScopedModelMatrix scopedModel;
		gl::translate( 0, 0, 0.01f );
		drawAxis( vec2( -0.90f, 0.7f ), 0.21f, controller->getAxis( ci::vr::Controller::AXIS_OCULUS_XBOX_LTHUMBSTICK ), Color( 0.9f, 0.1f, 0.0f ) );
		drawAxis( vec2(  0.40f, 0.2f ), 0.21f, controller->getAxis( ci::vr::Controller::AXIS_OCULUS_XBOX_RTHUMBSTICK ), Color( 0.9f, 0.1f, 0.0f ) );
	}
}

void ControllerBasicApp::drawOcculusRemote( const ci::vr::Controller *controller )
{
	if( ! controller ) {
		return;
	}

	drawButton( vec2( 0, 0.8f ), 0.10f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_REMOTE_ENTER ), Color( 0, 0.9f, 0 ) );
	drawButton( vec2( 0, 0.3f ), 0.08f, controller->getButton( ci::vr::Controller::BUTTON_OCULUS_REMOTE_BACK ), Color( 0.9f, 0.3f, 0 ) );
	drawButton( vec2( 0, 0.8f ) + vec2( -0.18f, 0 ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_LEFT ),  Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( 0, 0.8f ) + vec2(  0.18f, 0 ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_RIGHT ), Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( 0, 0.8f ) + vec2( 0,  0.18f ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_UP ),    Color( 0.9f, 0.9f, 0 ) );
	drawButton( vec2( 0, 0.8f ) + vec2( 0, -0.18f ), 0.03f, controller->getButton( ci::vr::Controller::BUTTON_DPAD_DOWN ),  Color( 0.9f, 0.9f, 0 ) );
}

void ControllerBasicApp::drawViveController( const ci::vr::Controller *controller )
{
	if( ! controller ) {
		return;
	}

	// Application menu button
	drawButton( vec2( 0, 1.5f ), 0.10f, controller->getButton( ci::vr::Controller::BUTTON_VIVE_APPLICATION_MENU ), Color( 0, 0.9f, 0 ) );

	// Trigger
	drawTrigger( vec2( 0, 1.25f ), 0.12f, controller->getTrigger(), Color( 0.9f, 0.1f, 0.9f ) );

	// Trigger button
	drawButton( vec2( 0, 1.0f ), 0.10f, controller->getButton( ci::vr::Controller::BUTTON_VIVE_TRIGGER), Color( 0.9f, 0.3f, 0  ) );

	// Touchpad button
	drawButton( vec2( 0, 0.65f ), 0.21f, controller->getButton( ci::vr::Controller::BUTTON_VIVE_TOUCHPAD ), Color( 0.3f, 0.3f, 0.4f ) );

	// Grip button
	drawButton( vec2( 0, 0.25f ), 0.10f, controller->getButton( ci::vr::Controller::BUTTON_VIVE_GRIP ),  Color( 0.9f, 0.9f, 0 ) );

	// Touchpad
	{
		gl::ScopedModelMatrix scopedModel;
		gl::translate( 0, 0, 0.01f );
		drawAxis( vec2( 0, 0.65f ), 0.21f, controller->getAxis(), Color( 0.9f, 0.1f, 0.0f ) );
	}
}

void ControllerBasicApp::drawScene()
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

	gl::lineWidth( 3.0f );

	if( ci::vr::API_OCULUS == mVrContext->getApi() ) {
		{
			gl::ScopedModelMatrix scopedModel;
			gl::translate( -1.25f, 0, 0 );
			drawOculusXbox( mOculusXbox );
		}

		{
			gl::ScopedModelMatrix scopedModel;
			gl::translate( 0.5f, 0, 0 );
			drawOcculusRemote( mOculusRemote );
		}
	}
	else if( ci::vr::API_OPENVR == mVrContext->getApi() ) {
		{
			gl::ScopedModelMatrix scopedModel;
			gl::translate( -1.0f, 0, 0 );
			drawViveController( mViveLeft );
		}

		{
			gl::ScopedModelMatrix scopedModel;
			gl::translate( 1.0f, 0, 0 );
			drawViveController( mViveRight );
		}

		// Draw controller input rays
		{
			float s = 1000.0f;

			if( mViveLeft ) {
				const auto& ir = mViveLeft->getInputRay();
				vec3 dir = ir.getDirection();
				vec3 orig = ir.getOrigin() + ( 0.055f * dir );
				gl::color( 0.9f, 0.5f, 0.0f );
				gl::drawLine( orig, orig + ( s * dir ) );
			}

			if( mViveRight ) {
				const auto& ir = mViveRight->getInputRay();
				vec3 dir = ir.getDirection();
				vec3 orig = ir.getOrigin() + ( 0.055f * dir );
				gl::color( 0.9f, 0.5f, 0.0f );
				gl::drawLine( orig, orig + ( s * dir ) );
			}
		}
	}

	// Draw coordinate frame
	{
		gl::drawCoordinateFrame( 1.0f );
	}

	// Draw HMD input ray (cyan sphere)
	{
		const auto& ir = mHmd->getInputRay();
		vec3 dir = ir.getDirection();
		vec3 P = ir.getOrigin() + ( 5.0f * dir );
		gl::color( 0.01f, 0.7f, 0.9f );
		gl::drawSphere( P, 0.04f );
	}
}

void ControllerBasicApp::draw()
{
	gl::clear( Color( 0.02f, 0.02f, 0.1f ) );
	if( mHmd ) {
		mHmd->bind();

		for( auto eye : mHmd->getEyes() ) {
			mHmd->enableEye( eye );	
			drawScene();
			mHmd->drawControllers( eye );
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
	settings->setTitle( "Cinder VR ControllerBasic" );
	settings->setWindowSize( 1920/2, 1080/2 );
}

CINDER_APP( ControllerBasicApp, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )
