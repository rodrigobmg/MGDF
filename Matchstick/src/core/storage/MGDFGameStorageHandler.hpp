#pragma once

#include <exception>
#include <string>
#include <map>
#include <MGDF/MGDFVersion.hpp>
#include <MGDF/MGDFError.hpp>

namespace MGDF
{
namespace core
{
namespace storage
{

class IGameStorageHandler
{
public:
	virtual ~IGameStorageHandler() {}
	virtual std::string GetGameName() const = 0;
	virtual std::string GetGameUid() const = 0;
	virtual const MGDF::Version *GetVersion() const = 0;
	virtual INT32 GetInterfaceVersion() const = 0;
	virtual MGDFError Load( const std::wstring &load ) = 0;
	virtual const std::map<std::string, std::string> &GetPreferences() const = 0;

};

}
}
}