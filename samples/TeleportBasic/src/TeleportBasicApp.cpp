#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/Sphere.h"
#include "cinder/Timeline.h"

#include "cinder/vr/vr.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class UiIntersectable;
using UiIntersectablePtr = std::shared_ptr<UiIntersectable>;

//! \class UiIntersectable
//!
//!
class UiIntersectable {
public:

	virtual bool intersects( const ci::Ray &ray ) const = 0;
	virtual void draw() = 0;

	bool getFocused() const { 
		return mFocused;
	}

	void setFocused( bool value ) { 
		if( value != mFocused ) {
			mFocused = value; 
			float target = ( mFocused ? 1.0f : 0.0f );
			ci::app::App::get()->timeline().apply( &mFocusedValue, target, 0.25f );
		}
	}

	bool getSelected() const {
		return mSelected;
	}

	virtual void setSelected( bool value ) {
		if( value != mSelected ) {
			mSelected = value;
			float target = ( mSelected ? 1.0f : 0.0f );
			ci::app::App::get()->timeline().apply( &mSelectedValue, target, 0.25f );
		}
	}

	ColorA getColor() const { 
		float t = mSelectedValue.value();
		Color color = (1.0f - t)*Color( 0.4f, 0.4f, 0.42f ) + t*Color( 0.95f, 0.5f, 0.0f );
		t = mFocusedValue.value();
		color = (1.0f - t)*color + t*Color( 0.9f, 0.9f, 0.0f );
		return ColorA( color ); 
	}

	const ci::vec3& getPosition() const {
		return mPosition;
	}
	
protected:
	gl::GlslProgRef		mShader;
	gl::BatchRef		mBatch;
	bool				mFocused = false;
	ci::Anim<float>		mFocusedValue = 0.0f;
	bool				mSelected = false;
	ci::Anim<float>		mSelectedValue = 0.0f;
	ci::vec3			mPosition;
};

//! \class UiBox
//!
//!
class UiBox : public UiIntersectable {
public:
	UiBox() {}

	UiBox( const vec3 &center, const vec3 &size, const gl::GlslProgRef &shader ) {
		vec3 min = center - 0.5f*size;
		vec3 max = center + 0.5f*size;
		mBox = AxisAlignedBox( min, max );
		mShader = shader;
		mBatch = gl::Batch::create( geom::Cube().size( size ) >> geom::Translate( center ), shader ); 
		mPosition = center;
	}

	virtual bool intersects( const ci::Ray &ray ) const override {
		bool result = false;
		float t0 = std::numeric_limits<float>::min();
		float t1 = std::numeric_limits<float>::min();
		if( mBox.intersect( ray, &t0, &t1 ) && ( t0 > 0.0f ) ) {
			result = true;
		}
		return result;
	}

	virtual void draw() {
		gl::ScopedDepth scopedDepth( false );

		gl::ScopedBlendAlpha scopeBlend;
		gl::ScopedColor scopedColor( getColor() );
		mBatch->draw();
	}
protected:
	AxisAlignedBox mBox;
};

//! \class TeleportBasicApp
//!
//!
class TeleportBasicApp : public App {
public:
	void	setup() override; 
	void	keyDown( KeyEvent event );
	void	mouseDown( MouseEvent event );
	void	update() override;
	void	draw() override;

private:
	ci::vr::Context				*mVrContext = nullptr;
	ci::vr::Hmd					*mHmd = nullptr;
	bool						mCyclopsMirroring = false;
	CameraPersp					mCamera;

	bool						mRecalcOrigin = false;

	const ci::vr::Controller	*mController1 = nullptr;
	const ci::vr::Controller	*mController2 = nullptr;

	gl::BatchRef				mGridBox;
	gl::BatchRef				mGridLines;

	std::vector<UiIntersectablePtr>	mShapes;
	uint32_t						mShapeIndex = 0;

	void	onControllerConnect( const ci::vr::Controller *controller );
	void	onControllerDisconnect( const ci::vr::Controller *controller );
	void	onButtonDown( const ci::vr::Controller::Button *button );
	void	onButtonUp( const ci::vr::Controller::Button *button );
	
	void	initGrid( const gl::GlslProgRef &shader );
	void	initShapes( const gl::GlslProgRef &shader );

	void	drawScene();
};

void TeleportBasicApp::initGrid( const gl::GlslProgRef &shader )
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

void TeleportBasicApp::initShapes( const gl::GlslProgRef &shader )
{
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   0, -0.0625f,   3 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  -4, -0.0625f,  -4 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   4, -0.0625f,  -4 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   0, -0.0625f, -10 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  -4, -0.0625f,  -4 ) + vec3( 0, 5, -12 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   4, -0.0625f,  -4 ) + vec3( 0, 8, -12 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  -8, -0.0625f,   3 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   8, -0.0625f,   3 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3( -16, -0.0625f,   3 ) + vec3( 0, 4, 0 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  16, -0.0625f,   3 ) + vec3( 0, 4, 0 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3( -16, -0.0625f,   3 ) + vec3( 0, 6.0f, -9 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  16, -0.0625f,   3 ) + vec3( 0, 2.5f, -9 ), vec3( 4, 0.125f, 4 ), shader ) ) );

	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(  -4, -0.0625f,   4 ) + vec3( 0, 8,  12 ), vec3( 4, 0.125f, 4 ), shader ) ) );
	mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( 0, -0.001f, 0 ) + vec3(   4, -0.0625f,   4 ) + vec3( 0, 5,  12 ), vec3( 4, 0.125f, 4 ), shader ) ) );
}

void TeleportBasicApp::setup()
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
				.setControllerConnected( std::bind( &TeleportBasicApp::onControllerConnect, this, std::placeholders::_1 ) )
				.setControllerDisconnected( std::bind( &TeleportBasicApp::onControllerDisconnect, this, std::placeholders::_1 ) )
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

	mVrContext->getSignalControllerButtonDown().connect( std::bind( &TeleportBasicApp::onButtonDown, this, std::placeholders::_1 ) );
	mVrContext->getSignalControllerButtonUp().connect( std::bind( &TeleportBasicApp::onButtonUp, this, std::placeholders::_1 ) );

	auto shader = gl::getStockShader( gl::ShaderDef().color() );
	initGrid( shader );
	initShapes( shader );
} 

void TeleportBasicApp::keyDown( KeyEvent event )
{
	switch( event.getChar() ) {
		case '1': {
			mCyclopsMirroring = false;
			if( mHmd ) {
				mHmd->setMirrorMode( ci::vr::Hmd::MirrorMode::MIRROR_MODE_STEREO );
			}
		}
		break;

		case '2': {
			mCyclopsMirroring = false;
			if( mHmd ) {
				mHmd->setMirrorMode( ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_STEREO );
			}
		}
		break;

		case '3': {
			mCyclopsMirroring = false;
			if( mHmd ) {
				mHmd->setMirrorMode( ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_LEFT );
			}
		}
		break;

		case '4': {
			mCyclopsMirroring = false;
			if( mHmd ) {
				mHmd->setMirrorMode( ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_RIGHT );
			}
		}
		break;

		case '5': {
			mCyclopsMirroring = true;
			if( mHmd ) {
				mHmd->setMirrorMode( ci::vr::Hmd::MirrorMode::MIRROR_MODE_NONE );
			}
		}
		break;
	}
}

void TeleportBasicApp::mouseDown( MouseEvent event )
{
}

void TeleportBasicApp::onControllerConnect(const ci::vr::Controller* controller)
{
	if( ! controller ) {
		return;
	}

	bool connected = false;
	if( ci::vr::API_OCULUS == controller->getApi() ) {
	}
	else if( ci::vr::API_OPENVR == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_LEFT: {
				mController1 = controller;
				connected = true;
			}
			break;

			case ci::vr::Controller::TYPE_RIGHT: {
				mController2 = controller;
				connected = true;
			}
			break;
		}
	}

	if( connected ) {
		CI_LOG_I( "Controller connected: " << controller->getName() );
	}
}

void TeleportBasicApp::onControllerDisconnect( const ci::vr::Controller* controller )
{
	if( ! controller ) {
		return;
	}

	bool disconnected = false;
	if( ci::vr::API_OCULUS == controller->getApi() ) {
	}
	else if( ci::vr::API_OPENVR == controller->getApi() ) {
		switch( controller->getType() ) {
			case ci::vr::Controller::TYPE_LEFT: {
				mController1 = nullptr;
				disconnected = true;
			}
			break;

			case ci::vr::Controller::TYPE_RIGHT: {
				mController2 = nullptr;
				disconnected = true;
			}
			break;
		}
	}

	if( disconnected ) {
		CI_LOG_I( "Controller disconnected: " << controller->getName() );
	}
}

void TeleportBasicApp::onButtonDown( const ci::vr::Controller::Button *button )
{
	uint32_t shapdeIndex = mShapeIndex;
	for( uint32_t i = 0; i < mShapes.size(); ++i ) {
		auto& shape = mShapes[i];
		if( shape->intersects( mHmd->getInputRay() ) ) {
			shapdeIndex = i;
			break;
		}
	}

	if( shapdeIndex != mShapeIndex ) {
		mShapeIndex = shapdeIndex;
		vec3 position = mShapes[mShapeIndex]->getPosition();
		mHmd->setLookAt( position );
	}
}

void TeleportBasicApp::onButtonUp( const ci::vr::Controller::Button *button )
{
}

void TeleportBasicApp::update()
{
	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if( ( ci::vr::API_OPENVR == mVrContext->getApi() ) && ( ! mRecalcOrigin ) && ( mHmd->getElapsedFrames() > 60 ) ) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}

	for( auto& shape : mShapes ) {
		bool focused = shape->intersects( mHmd->getInputRay() );
		shape->setFocused( focused );
	}
}

void TeleportBasicApp::drawScene()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Draw ground grid
	{
		gl::ScopedLineWidth scopedLineWidth( 1.0f );

		gl::color( 0.8f, 0.8f, 0.8f );
		mGridBox->draw();

		gl::color( 0.65f, 0.65f, 0.65f );
		mGridLines->draw();
	}

	// Draw shape
	for( auto& shape : mShapes ) {
		shape->draw();
	}

	// Draw coordinate frame
	{
		gl::drawCoordinateFrame( 1.0f );
	}

	// Draw controller input rays
	{
		gl::lineWidth( 3.0f );

		float s = 1000.0f;

		if( mController1 && mController1->hasInputRay() ) {
			const auto& ir = mController1->getInputRay();
			vec3 dir = ir.getDirection();
			vec3 orig = ir.getOrigin() + ( 0.055f * dir );
			gl::color( 0.9f, 0.5f, 0.0f );
			gl::drawLine( orig, orig + ( s * dir ) );
		}

		if( mController2 && mController2->hasInputRay() ) {
			const auto& ir = mController2->getInputRay();
			vec3 dir = ir.getDirection();
			vec3 orig = ir.getOrigin() + ( 0.055f * dir );
			gl::color( 0.9f, 0.5f, 0.0f );
			gl::drawLine( orig, orig + ( s * dir ) );
		}
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

void TeleportBasicApp::draw()
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
		if( mCyclopsMirroring ) {
			mHmd->submitFrame();
			gl::viewport( getWindowSize() );
			mHmd->enableEye( ci::vr::EYE_HMD );
			drawScene();
		}
		else {
			gl::viewport( getWindowSize() );
			gl::setMatricesWindow( getWindowSize() );
			mHmd->drawMirrored( getWindowBounds(), true );
		}
	}
	else {
		gl::viewport( getWindowSize() );
		gl::setMatrices( mCamera );
		drawScene();
	}
}

void prepareSettings( App::Settings *settings )
{
	settings->setTitle( "Cinder VR ControllerIntermediate" );
	settings->setWindowSize( 1920/2, 1080/2 );
}

CINDER_APP( TeleportBasicApp, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )
