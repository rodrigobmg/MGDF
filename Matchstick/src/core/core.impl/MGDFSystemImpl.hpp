#pragma once

#include <list>
#include <hash_map> //stdext::hash_map
#include <boost/signals.hpp>
#include <boost/thread/thread.hpp>

#include <MGDF/MGDF.hpp>
#include <MGDF/MGDFSystem.hpp>
#include <MGDF/MGDFModule.hpp>

#include "../common/MGDFListImpl.hpp"
#include "../common/MGDFSystemComponent.hpp"
#include "../common/MGDFLoggerImpl.hpp"
#include "../common/MGDFParameterManager.hpp"
#include "../storage/MGDFStorageFactoryComponentImpl.hpp"
#include "../input/MGDFInputManagerComponentImpl.hpp"
#include "../audio/MGDFSoundManagerComponentImpl.hpp"
#include "../vfs/MGDFVirtualFileSystemComponentImpl.hpp"

#include "MGDFGameImpl.hpp"
#include "MGDFStatisticsManagerImpl.hpp"
#include "MGDFGraphicsManagerImpl.hpp"
#include "MGDFModuleFactory.hpp"
#include "MGDFSystemStats.hpp"
#include "MGDFTimer.hpp"

namespace MGDF
{
namespace core
{

/**
this class represents a slightly more functional version of the system interface with a number of internal
implementation additions, this class should be inherited to create mock system objects
*/
class ISystemImpl: public ISystem, public IErrorHandler
{
public:
	virtual ~ISystemImpl( void ) {};

	//callbacks
	typedef boost::signal<void ( void ) > ShutDownFunction; //shutDown callback function signature
	typedef boost::signal<void ( std::string, std::string ) > FatalErrorFunction; //fatal error callback function signature

	virtual IModule *GetModule() const = 0;

	void AddShutDownCallback( const ShutDownFunction::slot_type& callback );
	void AddFatalErrorCallback( const FatalErrorFunction::slot_type& callback );
protected:
	//event callbacks
	boost::signal<void ( void ) > _shutDownFunction;
	boost::signal<void ( std::string, std::string ) > _fatalErrorFunction;
};

typedef ListImpl<IStringList, const char *> StringList;

/**
 reference implementation of the ISystem interface
\author gcconner
*/
class System: public ISystemImpl
{
public:
	System( Game *game );
	void Initialize();
	virtual ~System( void );

	void UpdateScene( double simulationTime, SystemStats &stats );
	void DrawScene( double alpha );
	void BackBufferChanged();
	UINT32 GetCompatibleD3DFeatureLevels( D3D_FEATURE_LEVEL *levels, UINT32 *featureLevelsSize );
	void SetD3DDevice( ID3D11Device *d3dDevice );
	void CreateGraphicsImpl( ID3D11Device *device, IDXGIAdapter1 *adapter );
	GraphicsManager *GetGraphicsImpl() const;
	input::IInputManagerComponent *GetInputManagerImpl() const;
	std::string GetSystemInformation( SystemStats &stats );
	void DisposeModule();

	virtual IModule *GetModule() const;
	virtual void QueueShutDown();

	virtual INT32 Load( const char *saveName, wchar_t *loadBuffer, UINT32 *size, Version &version );
	virtual INT32 BeginSave( const char *saveName, wchar_t *saveBuffer, UINT32 *size );
	virtual bool CompleteSave( const char *saveName );

	//error handling functions
	virtual void FatalError( const char *, const char * );
	virtual void SetLastError( const char *sender, UINT32 code, const char *description );

	virtual ILogger *GetLogger() const;
	virtual ITimer * GetTimer() const;
	virtual IVirtualFileSystem *GetVFS() const;
	virtual ISoundManager *GetSound() const;
	virtual IGraphicsManager *GetGraphics() const;
	virtual IStatisticsManager *GetStatistics() const;
	virtual IGame *GetGame() const;
	virtual IInputManager *GetInput() const;
	virtual ID3D11Device *GetD3DDevice() const;
	virtual void ShutDown();
	virtual const IStringList *GetSaves() const;
	virtual void RemoveSave( const char *saveName );
	virtual const Version * GetMGDFVersion() const;
	virtual const Error *GetLastError() const;
private:
	IModule *CreateModule();

	void ClearWorkingDirectory();

	storage::IStorageFactoryComponent *_storage;
	input::IInputManagerComponent *_input;
	audio::ISoundManagerComponent *_sound;
	vfs::IVirtualFileSystemComponent *_vfs;
	Game *_game;
	StringList *_saves;
	GraphicsManager *_graphics;
	StatisticsManager *_stats;
	ID3D11Device *_d3dDevice;
	Version _version;
	Error _lastError;
	boost::mutex _mutex;
	Timer _timer;

	IModule * _module; //the currently executing module
	ModuleFactory *_moduleFactory;
};

}
}