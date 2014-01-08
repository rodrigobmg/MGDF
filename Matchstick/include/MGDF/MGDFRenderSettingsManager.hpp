#pragma once

#include <MGDF/MGDFList.hpp>

namespace MGDF
{

/**
This class represents the display settings for a particular adaptor mode
\author gcconner
*/
struct AdaptorMode
{
	UINT32  Width;
	UINT32  Height;
	UINT32 RefreshRateNumerator;
	UINT32 RefreshRateDenominator;
};

/**
this class allows you to get and set the engines various display settings
\author gcconner
*/
class IRenderSettingsManager
{
public:
	/**
	gets the current vsync setting
	*/
	virtual bool GetVSync() const = 0;

	/**
	sets the current vsync setting
	*/
	virtual void SetVSync( bool vsync ) = 0;

	/**
	gets the current fullscreen setting
	*/
	virtual bool GetFullscreen() const = 0;

	/**
	sets the current fullscreen setting
	*/
	virtual void SetFullscreen( bool fullscreen ) = 0;
	
	/**
	get the number of supported multisample levels
	\return the number of supported multisample levels
	*/
	virtual UINT32 GetMultiSampleLevelCount() const = 0;

	/**
	get the available multisample level supported by the display adaptor at the given index ( 0 to GetMultiSampleLevelCount - 1 )
	\return true if a supported multisample level is present at the given index, false otherwise.
	*/
	virtual bool GetMultiSampleLevel( UINT32 index, UINT32 * level ) const = 0;

	/**
	set the display adaptors current multisample level, this changed setting is not applied until ApplySettings is called.
	\return returns false if the desired multisample level cannot be set.
	*/
	virtual bool SetBackBufferMultiSampleLevel( UINT32 multisampleLevel ) = 0;

	/**
	get the current multisample level in use by the adaptor
	*/
	virtual UINT32  GetBackBufferMultiSampleLevel() const = 0;

	/**
	set the desired multisample level for off screen render targets. This setting is not used directly
	by the framework but any client code should query this property when creating render targets that
	may require multisampling (see also GetCurrentMultiSampleLevel)
	\return returns false if the desired multisample level cannot be set.
	*/
	virtual bool SetCurrentMultiSampleLevel( UINT32 multisampleLevel ) = 0;

	/**
	get the current desired multisample level for off screen render targets
	\param quality if specified this parameter will be initialized with the maximum
	multisampling quality setting for the current multisample level
	*/
	virtual UINT32 GetCurrentMultiSampleLevel( UINT32 *quality ) const = 0;

	/**
	get the number of supported adaptor modes
	\return the number of supported adaptor modes
	*/
	virtual UINT32 GetAdaptorModeCount() const = 0;

	/**
	get a supported adaptor mode at the given index (0 to GetAdaptorModeCount - 1 )
	\return true if an adaptor mode exists at a particular index
	*/
	virtual bool GetAdaptorMode( UINT32 index, AdaptorMode * mode ) const = 0;

	/**
	get the adaptor mode (if any) matching the requested width and height, if no matching adaptor is found, false is returned
	\return true if a supported adaptor mode exists for the given width and height
	*/
	virtual bool GetAdaptorMode( UINT32 width, UINT32 height, AdaptorMode * mode ) const = 0;

	/**
	get the current adaptor mode being used
	*/
	virtual void GetCurrentAdaptorMode( AdaptorMode * mode ) const = 0;

	/**
	sets the current display adaptor mode, this changed setting is not applied until ApplyChanges is called.
	\return true if the adaptor mode is supported and can be applied, false otherwise
	*/
	virtual bool SetCurrentAdaptorMode( const AdaptorMode * mode ) = 0;

	/**
	get the current screen width, based on the current adaptor mode in fullscreen, or on the window dimensions otherwise
	*/
	virtual UINT32  GetScreenX() const = 0;

	/**
	get the current screen height, based on the current adaptor mode in fullscreen, or on the window dimensions otherwise
	*/
	virtual UINT32  GetScreenY() const = 0;

	/**
	Queues the swap chain to be reset on the beginning of the next frame. This applies any changed adaptor mode or multisample settings.
	*/
	virtual void  ApplyChanges() = 0;
};

}