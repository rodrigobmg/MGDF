#include "StdAfx.h"

#include "MGDFApp.hpp"
#include "core.impl/MGDFHostImpl.hpp"
#include "core.impl/MGDFParameterConstants.hpp"
#include "core.impl/MGDFPreferenceConstants.hpp"

#if defined(_DEBUG)
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#pragma warning(disable:4291)
#endif

namespace MGDF
{
namespace core
{

MGDFApp::MGDFApp( Host* host, HINSTANCE hInstance )
	: D3DAppFramework( hInstance )
	, _stats( TIMER_SAMPLES )
	, _host( host )
	, _blackBrush( nullptr )
	, _whiteBrush( nullptr )
	, _textFormat( nullptr )
	, _dWriteFactory( nullptr )
	, _initialized( false )
	, _context( nullptr )
{
	_ASSERTE( host );

	const char *simFps = host->GetGame()->GetPreference( PreferenceConstants::SIM_FPS );
	UINT32 simulationFps = atoi( simFps );

	const char *interpolateFrames = host->GetGame()->GetPreference( PreferenceConstants::INTERPOLATE_FRAMES );
	_interpolateFrames = atoi( interpolateFrames ) == 1;
	_awaitFrame.test_and_set();

	if ( !simFps || !simulationFps ) {
	FATALERROR(_host, PreferenceConstants::SIM_FPS << " was not found in preferences or is not an integer");
	}

	if ( MGDF_OK != FrameLimiter::TryCreate( simulationFps, &_frameLimiter ) ) {
		FATALERROR( _host, "Unable to create frame limiter" );
	}
	_stats.SetExpectedSimTime( 1 / ( double ) simulationFps );

	_drawSystemOverlay.store( false );

	_ASSERTE( host );
	_host = host;
	_host->SetShutDownHandler( [this]() {
		CloseWindow();
	} );

	if ( FAILED( DWriteCreateFactory(
	            DWRITE_FACTORY_TYPE_SHARED,
	            __uuidof( IDWriteFactory1 ),
	            reinterpret_cast<IUnknown**>( &_dWriteFactory )
	        ) ) ) {
		FATALERROR( _host, "Unable to create IDWriteFactory" );
	}

	IDWriteFontCollection *fontCollection;
	if (FAILED(_dWriteFactory->GetSystemFontCollection( &fontCollection ))) {
		FATALERROR( _host, "Unable to get  font collection" );
	}

	if (FAILED(_dWriteFactory->CreateTextFormat( 
		L"Arial",
		fontCollection,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		14,L"",&_textFormat))) {
		FATALERROR( _host, "Unable to create text format" );
	}

	SAFE_RELEASE(fontCollection);
}

MGDFApp::~MGDFApp()
{
	delete _frameLimiter;
	SAFE_RELEASE( _context );
	SAFE_RELEASE(_blackBrush);
	SAFE_RELEASE(_whiteBrush);
	SAFE_RELEASE(_textFormat);
	SAFE_RELEASE(_dWriteFactory);
}

UINT32 MGDFApp::GetCompatibleD3DFeatureLevels( D3D_FEATURE_LEVEL *levels, UINT32 *featureLevelsSize )
{
	return _host->GetCompatibleD3DFeatureLevels( levels, featureLevelsSize );
}

void MGDFApp::OnInitDevices( HWND window, ID3D11Device *d3dDevice, ID2D1Device *d2dDevice, IDXGIAdapter1 *adapter )
{
	_ASSERTE( d3dDevice );
	_ASSERTE( d2dDevice );
	_ASSERTE( adapter );

	if ( FAILED( d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,&_context))) {
		FATALERROR( this, "Unable to create ID2D1DeviceContext" );
	}

	_host->RTSetDevices( window, d3dDevice, d2dDevice, adapter );
}

bool MGDFApp::IsBackBufferChangePending()
{
	return _host->GetRenderSettingsImpl().IsBackBufferChangePending();
}

bool MGDFApp::VSyncEnabled() const
{
	return _host->GetRenderSettingsImpl().GetVSync();
}

bool MGDFApp::OnInitWindow( RECT &window )
{
	window.top = 0;
	window.left = 0;
	window.right = atoi(_host->GetGame()->GetPreference( PreferenceConstants::WINDOW_SIZEX ));
	window.bottom = atoi(_host->GetGame()->GetPreference( PreferenceConstants::WINDOW_SIZEY ));
	const char *windowResize = _host->GetGame()->GetPreference( PreferenceConstants::WINDOW_RESIZE );
	return atoi( windowResize ) == 1;
}

void MGDFApp::OnResetSwapChain( DXGI_SWAP_CHAIN_DESC1 &swapDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC &fullscreenDesc, const RECT& windowSize )
{
	_host->GetRenderSettingsImpl().OnResetSwapChain( swapDesc, fullscreenDesc, windowSize );
}

void MGDFApp::OnSwitchToFullScreen( DXGI_MODE_DESC1 &desc )
{
	_host->GetRenderSettingsImpl().OnSwitchToFullScreen( desc );
}

void MGDFApp::OnSwitchToWindowed()
{
	_host->GetRenderSettingsImpl().SetFullscreen( false );
}

void MGDFApp::OnResize( UINT32 width, UINT32 height )
{
	_host->GetRenderSettingsImpl().OnResize( width, height );
}

void MGDFApp::OnBeforeDeviceReset()
{
	SAFE_RELEASE( _context );
	SAFE_RELEASE(_blackBrush);
	SAFE_RELEASE(_whiteBrush);
	_host->RTBeforeDeviceReset();
}

void MGDFApp::OnBeforeBackBufferChange()
{
	_context->SetTarget( nullptr );
	_host->RTBeforeBackBufferChange();
}

void MGDFApp::OnBackBufferChange( ID3D11Texture2D *backBuffer )
{
	_ASSERTE( backBuffer );

	_host->RTBackBufferChange( backBuffer );
	_host->SetBackBufferRenderTarget(_context);
}

void MGDFApp::OnBeforeFirstDraw()
{
	// wait for one sim frame to finish before
	// we start drawing
	while ( _awaitFrame.test_and_set() ) {
		Sleep( 1 );
	}
	_host->RTBeforeFirstDraw();
}

void MGDFApp::OnDraw()
{
	_renderStart = _host->GetTimer()->GetCurrentTimeTicks();

	// We can minimize visual 'stuttering' due to the fact that 
	// the sim and render threads may not be in sync using two methods.
	// We can either interpolate between render frames or we can
	// prevent rendering until we guarentee that a new frame is ready.
	// if the sim FPS is greater than or equal to 60FPS then the latter
	// is preferred. However if sim FPS is much lower than 60FPS, then 
	// the former method may be a better option.
	double alpha = 0;
	if ( _interpolateFrames ) {
		alpha = _frameLimiter->ProgressThroughCurrentFrame();
	} else {
		while ( _awaitFrame.test_and_set() ) {
			Sleep( 1 );
		}
	}
	_host->RTDraw( alpha ); 

	bool exp = true;
	if ( _drawSystemOverlay.compare_exchange_weak( exp, true ) ) {
		DrawSystemOverlay();
	}

	_activeRenderEnd = _host->GetTimer()->GetCurrentTimeTicks();
}

void MGDFApp::OnAfterPresent()
{
	LARGE_INTEGER renderEnd = _host->GetTimer()->GetCurrentTimeTicks();
	_stats.AppendRenderTimes(
		_host->GetTimer()->ConvertDifferenceToSeconds( renderEnd, _renderStart ),
		_host->GetTimer()->ConvertDifferenceToSeconds( _activeRenderEnd, _renderStart ) );
}

void MGDFApp::DrawSystemOverlay()
{
	if ( !_whiteBrush ) {
		InitBrushes();
	}

	const float margin = 5.0f;

	std::wstringstream stream;
	_host->GetHostInfo( _stats, stream );
	std::wstring information( stream.str() );

	IDWriteTextLayout *textLayout;
	if (FAILED(_dWriteFactory->CreateTextLayout(
		information.c_str(),
		static_cast<UINT32>(information.size()),
		_textFormat,
		static_cast<float>(_host->GetRenderSettingsImpl().GetScreenX() ),
		static_cast<float>(_host->GetRenderSettingsImpl().GetScreenY() ),
		&textLayout))) {
		FATALERROR(_host,"Unable to create text layout");
	}

	DWRITE_TEXT_METRICS metrics;
	SecureZeroMemory(&metrics,sizeof(metrics));
	if (FAILED(textLayout->GetMetrics(&metrics))) {
		FATALERROR(_host,"Unable to get text overhang metrics");
	}

	_context->BeginDraw();

	D2D1_ROUNDED_RECT rect;
	rect.radiusX = margin;
	rect.radiusY = margin;
	rect.rect.top = margin;
	rect.rect.left = margin;
	rect.rect.bottom = ( margin * 3 ) + metrics.height;
	rect.rect.right = ( margin * 3 ) + metrics.width;
	_context->FillRoundedRectangle(&rect,_blackBrush);
	_context->DrawRoundedRectangle(&rect,_whiteBrush);

	D2D_POINT_2F origin;
	origin.x = ( 2 * margin );
	origin.y = ( 2 * margin );
	_context->DrawTextLayout(origin,textLayout,_whiteBrush);

	_context->EndDraw();

	SAFE_RELEASE(textLayout);
}

void MGDFApp::InitBrushes()
{
	D2D1_COLOR_F color;
	color.a = color.r = color.g = color.b = 1.0f;
	if (FAILED(_context->CreateSolidColorBrush(color,&_whiteBrush))) {
		FATALERROR(_host,"Unable to create white color brush");
	}

	color.r = color.g = color.b = 0.0f;
	color.a = 0.85f;
	if (FAILED(_context->CreateSolidColorBrush(color,&_blackBrush))) {
		FATALERROR(_host,"Unable to create black color brush");
	}
}

void MGDFApp::OnUpdateSim()
{
	_simulationStart = _host->GetTimer()->GetCurrentTimeTicks();

	if ( !_initialized ) {
		LOG( "Creating Module...", LOG_LOW );
		_host->STCreateModule();
		_initialized = true;
	}

	//execute one frame of game logic as per the current module
	_host->STUpdate( _stats.ExpectedSimTime(), _stats );
	_awaitFrame.clear();

	LARGE_INTEGER activeSimulationEnd = _host->GetTimer()->GetCurrentTimeTicks();
	_stats.AppendActiveSimTime( _host->GetTimer()->ConvertDifferenceToSeconds( activeSimulationEnd, _simulationStart ) );
}


void MGDFApp::OnSimIdle()
{
	//wait until the next frame to begin if we have any spare time left over
	_frameLimiter->LimitFps();

	LARGE_INTEGER simulationEnd = _host->GetTimer()->GetCurrentTimeTicks();
	_stats.AppendSimTime( _host->GetTimer()->ConvertDifferenceToSeconds( simulationEnd, _simulationStart ) );
}


void MGDFApp::OnRawInput( RAWINPUT *input )
{
	_ASSERTE( input );
	_host->GetInputManagerImpl().HandleInput( input );
}

void MGDFApp::OnClearInput()
{
	_host->GetInputManagerImpl().ClearInput();
}

void MGDFApp::OnMouseInput( INT32 x, INT32 y )
{
	_host->GetInputManagerImpl().HandleInput( x, y );
}

void MGDFApp::OnInputIdle()
{
	_host->GetInputManagerImpl().ProcessInput();
}

void MGDFApp::OnExternalClose()
{
	_host->QueueShutDown();
}

LRESULT MGDFApp::OnHandleMessage( HWND hwnd, UINT32 msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg) {
		case WM_SYSKEYDOWN:
			switch ( wParam ) {
			case VK_F12: {
				//Toggle  stats overlay with alt f12
				bool exp = true;
				//if its true set it to false.
				if ( !_drawSystemOverlay.compare_exchange_weak( exp, false ) ) {
					//otherwise it must be false so set it to true
					_drawSystemOverlay.store( true );
				}
			}
			break;
			}
			return 0;
		default:
			return DefWindowProc( hwnd, msg, wParam, lParam );
	}
}

void MGDFApp::FatalError( const char *sender, const char *message )
{
	_ASSERTE( sender );
	_ASSERTE( message );
	_host->FatalError( sender, message );
}

}
}