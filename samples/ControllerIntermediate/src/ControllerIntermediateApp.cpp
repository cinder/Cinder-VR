#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/Log.h"
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
	
protected:
	gl::GlslProgRef		mShader;
	gl::BatchRef		mBatch;
	bool				mFocused = false;
	ci::Anim<float>		mFocusedValue = 0.0f;
	bool				mSelected = false;
	ci::Anim<float>		mSelectedValue = 0.0f;
};

//! \class UiSphere
//!
//!
class UiSphere : public UiIntersectable {
public:
	UiSphere() {}

	UiSphere( const vec3 &center, float radius, const gl::GlslProgRef &shader ) {
		mSphere = ci::Sphere( center, radius );
		mShader = shader;
		mBatch = gl::Batch::create( geom::Sphere( mSphere ), shader ); 
	}

	virtual bool intersects( const ci::Ray &ray ) const override {
		return mSphere.intersects( ray );
	}

	virtual void draw() {
		gl::ScopedDepth scopedDepth( false );

		gl::ScopedBlendAlpha scopeBlend;
		gl::ScopedColor scopedColor( getColor() );
		mBatch->draw();
	}
protected:
	ci::Sphere mSphere;
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
	}

	virtual bool intersects( const ci::Ray &ray ) const override {
		return mBox.intersects( ray );
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

//! \class ControllerIntermediateApp
//!
//!
class ControllerIntermediateApp : public App {
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

	std::vector<UiIntersectablePtr>	mShapes;

	void	onControllerConnect( const ci::vr::Controller *controller );
	void	onControllerDisconnect( const ci::vr::Controller *controller );
	void	onButtonDown( const ci::vr::Controller::Button *button );
	void	onButtonUp( const ci::vr::Controller::Button *button );
	
	void	initGrid( const gl::GlslProgRef &shader );
	void	initShapes( const gl::GlslProgRef &shader );

	void	drawScene();
};

void ControllerIntermediateApp::initGrid( const gl::GlslProgRef &shader )
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

void ControllerIntermediateApp::initShapes( const gl::GlslProgRef &shader )
{
	int m = 8;
	int n = 4;
	float dx = 2.5f;
	float dy = 2.0f;
	float x1 = -( ( m - 1 ) * dx ) / 2.0f;
	float y1 = ( n * dy ) - ( dy / 2.0f );
	for( int j = 0; j < n; ++j ) {
		for( int i = 0; i < m; ++i ) {
			float x = i * dx + x1;
			float y = -( j * dy ) + y1;
			float z = -4.0f;
			if( i & 1 ) {
				mShapes.push_back( UiIntersectablePtr( new UiBox( vec3( x, y, z ), vec3( 1.0f ), shader ) ) );
			}
			else {
				mShapes.push_back( UiIntersectablePtr( new UiSphere( vec3( x, y, z ), 0.80f, shader ) ) );
			}
		}
	}
}

void ControllerIntermediateApp::setup()
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
				.setControllerConnected( std::bind( &ControllerIntermediateApp::onControllerConnect, this, std::placeholders::_1 ) )
				.setControllerDisconnected( std::bind( &ControllerIntermediateApp::onControllerDisconnect, this, std::placeholders::_1 ) )
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

	mVrContext->getSignalControllerButtonDown().connect( std::bind( &ControllerIntermediateApp::onButtonDown, this, std::placeholders::_1 ) );
	mVrContext->getSignalControllerButtonUp().connect( std::bind( &ControllerIntermediateApp::onButtonUp, this, std::placeholders::_1 ) );

	auto shader = gl::getStockShader( gl::ShaderDef().color() );
	initGrid( shader );
	initShapes( shader );
} 

void ControllerIntermediateApp::onControllerConnect( const ci::vr::Controller* controller )
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

void ControllerIntermediateApp::onControllerDisconnect( const ci::vr::Controller* controller )
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

void ControllerIntermediateApp::onButtonDown( const ci::vr::Controller::Button *button )
{
	const ci::vr::Controller *controller = button->getController();
	for( auto& shape : mShapes ) {
		if( ci::vr::API_OCULUS == mVrContext->getApi() ) {
			const auto& ir = mHmd->getInputRay();
			if( shape->intersects( ir ) ) {
				shape->setSelected( ! shape->getSelected() );
			}
		}
		else if( ci::vr::API_OPENVR == mVrContext->getApi() ) {
			if( controller->hasInputRay() ) {
				const auto& ir = controller->getInputRay();
				if( shape->intersects( ir ) ) {
					shape->setSelected( ! shape->getSelected() );
				}
			}
		}
	}
}

void ControllerIntermediateApp::onButtonUp( const ci::vr::Controller::Button *button )
{
}

void ControllerIntermediateApp::update()
{
	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if( ( ci::vr::API_OPENVR == mVrContext->getApi() ) && ( ! mRecalcOrigin ) && ( mHmd->getElapsedFrames() > 60 ) ) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}

	for( auto& sphere : mShapes ) {
		bool focused1 = false;
		bool focused2 = false;
		bool focused3 = false; 

		if( mController1 && mController1->hasInputRay() ) {
			const auto& ir = mController1->getInputRay();
			focused1 = sphere->intersects( ir );
		}

		if( mController2 && mController2->hasInputRay() ) {
			const auto& ir = mController2->getInputRay();
			focused2 = sphere->intersects( ir );
		}

		if( ci::vr::API_OCULUS == mVrContext->getApi() ) {
			const auto& ir = mHmd->getInputRay();
			focused3 = sphere->intersects( ir );
		}

		sphere->setFocused( focused1 || focused2 || focused3 );
	}
}

void ControllerIntermediateApp::drawScene()
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

	// Draw shape
	for( auto& shape : mShapes ) {
		shape->draw();
	}

	if( ci::vr::API_OCULUS == mVrContext->getApi() ) {
	}
	else if( ci::vr::API_OPENVR == mVrContext->getApi() ) {
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

void ControllerIntermediateApp::draw()
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
	settings->setTitle( "Cinder VR ControllerIntermediate" );
	settings->setWindowSize( 1920/2, 1080/2 );
}

CINDER_APP( ControllerIntermediateApp, RendererGl( RendererGl::Options().msaa(0) ), prepareSettings )
