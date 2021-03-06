#pragma once

#include <MGDF/MGDF.hpp>

namespace MGDF
{
namespace core
{

typedef bool ( *GetCustomArchiveHandlersPtr )( IArchiveHandler **list, UINT32 *length, ILogger *logger, IErrorHandler *errorHandler );
typedef IModule * ( *GetModulePtr )();
typedef bool ( *IsCompatibleInterfaceVersionPtr )( int );
typedef UINT32( *GetCompatibleFeatureLevelsPtr )( D3D_FEATURE_LEVEL *, UINT32 * );

class ModuleFactory
{
public:
	virtual ~ModuleFactory();
	static MGDFError TryCreate( ModuleFactory ** );

	bool GetCustomArchiveHandlers( IArchiveHandler **list, UINT32 *length, ILogger *logger, IErrorHandler *errorHandler ) const;
	IModule *GetModule() const;
	bool IsCompatibleInterfaceVersion( int ) const;
	UINT32 GetCompatibleFeatureLevels( D3D_FEATURE_LEVEL *levels, UINT32 *levelSize ) const;
	bool GetLastError( std::string& error ) const;

private:
	ModuleFactory();
	MGDFError Init();

	HINSTANCE _moduleInstance;
	GetCustomArchiveHandlersPtr _getCustomArchiveHandlers;
	GetModulePtr _getModule;
	IsCompatibleInterfaceVersionPtr _isCompatibleInterfaceVersion;
	GetCompatibleFeatureLevelsPtr _getCompatibleFeatureLevels;
	std::string _lastError;
};

}
}