#include "StdAfx.h"

#include "../common/MGDFLoggerImpl.hpp"
#include "../common/MGDFVersionHelper.hpp"
#include "../common/MGDFResources.hpp"
#include "MGDFGameImpl.hpp"
#include "MGDFSystemImpl.hpp"

//this snippet ensures that the location of memory leaks is reported correctly in debug mode
#if defined(DEBUG) |defined(_DEBUG)
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#pragma warning(disable:4291)
#endif

namespace MGDF { namespace core {

Game::Game(const std::string &uid,const std::string &name,INT32 interfaceVersion,const Version *version,storage::IStorageFactoryComponent *storageFactory)
{
	_uid = uid;
	_name = name;
	_interfaceVersion = interfaceVersion;
	_version = VersionHelper::Copy(version);
	_storageFactory = storageFactory;
}

Game::~Game(void)
{
}

const char *Game::GetUid() const
{
	return _uid.c_str();
}

const char *Game::GetName() const
{
	return _name.c_str();
}

INT32 Game::GetInterfaceVersion() const
{
	return _interfaceVersion;
}

const Version *Game::GetVersion() const
{
	return &_version;
}

bool Game::HasPreference(const char *name) const
{
	std::string n(name);
	return _preferences.find(n)!=_preferences.end();
}

const char *Game::GetPreference(const char *name) const
{
	std::string n(name);
	auto iter = _preferences.find(n);
	if (iter!=_preferences.end()) {
		return iter->second.c_str();
	}
	else {
		return nullptr;
	}
}

void Game::SetPreference(const char *name,const char *value)
{
	std::string n(name);
	//only set the value if the key exists (prevents adding new prefs by accident)
	if (_preferences.find(n)!=_preferences.end()) {
		_preferences[n] = value;
	}
}

void Game::ResetPreferences()
{
	LoadPreferences(_preferencesFile.c_str());
}

void Game::SavePreferences() const
{
	std::auto_ptr<storage::IPreferenceConfigStorageHandler> handler(_storageFactory->CreatePreferenceConfigStorageHandler());
	for (auto iter = _preferences.begin();iter!=_preferences.end();++iter) {
		handler->Add(iter->first,iter->second);
	}
	handler->Save(_preferencesFile);
	GetLoggerImpl()->Add(THIS_NAME,"saved preferences to '"+Resources::ToString(_preferencesFile)+"' successfully");
}

void Game::SavePreferences(const std::wstring &filename) {
	_preferencesFile = filename;
	SavePreferences();
}

void Game::LoadPreferences(const std::wstring &filename)
{
	std::auto_ptr<storage::IPreferenceConfigStorageHandler> handler(_storageFactory->CreatePreferenceConfigStorageHandler());
	handler->Load(filename);
	for (auto iter = handler->Begin();iter!=handler->End();++iter)
	{
		_preferences[iter->first] = iter->second;
	}
	GetLoggerImpl()->Add(THIS_NAME,"loaded preferences from '"+Resources::ToString(filename)+"' successfully",LOG_MEDIUM);
}

}
}