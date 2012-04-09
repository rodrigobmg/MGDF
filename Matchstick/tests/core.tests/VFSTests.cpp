#include <winunit.h>

#pragma warning( push )
#pragma warning( disable:4996 )
#include <boost/algorithm/string.hpp>
#pragma warning( pop )

#include "MGDFMockLogger.hpp"
#include "MGDFMockErrorHandler.hpp"
#include "../../src/core/common/MGDFResources.hpp"
#include "../../src/core/vfs/MGDFVirtualFileSystemComponentImpl.hpp"
#include "../../src/core/vfs/archive/zip/ZipArchiveHandlerImpl.hpp"

using namespace MGDF;
using namespace MGDF::core;
using namespace MGDF::core::vfs;

FIXTURE( VFSTestFixture )

namespace VFSTests {
	IVirtualFileSystemComponent *_vfs;
	MGDF::core::tests::MockLogger *_logger;
	MGDF::core::tests::MockErrorHandler *_errorHandler;
}
using namespace VFSTests;

SETUP(VFSTestFixture)
{
	HINSTANCE inst;
	inst=(HINSTANCE)GetModuleHandleW(L"core.tests.dll");
	Resources::Instance(inst);
	Resources::Instance().SetUserBaseDir(true,"junkship");

	_logger = new MGDF::core::tests::MockLogger();
	_errorHandler = new MGDF::core::tests::MockErrorHandler();

	_vfs = CreateVirtualFileSystemComponentImpl(NULL,NULL,(ILogger *)_logger);
	_vfs->RegisterArchiveHandler(CreateZipArchiveHandlerImpl((ILogger *)_logger,(IErrorHandler *)_errorHandler));	
}

TEARDOWN(VFSTestFixture)
{
	delete _vfs;
	delete _logger;
	delete _errorHandler;
}

/**
check that zip archives are enumerated correctly by the vfs
*/
BEGIN_TESTF(ZipArchiveTests,VFSTestFixture)
{
	_vfs->MapDirectory((Resources::Instance().RootDir()+L"../../tests/content/test.zip").c_str(),L"",NULL,false);

	WIN_ASSERT_STRING_EQUAL(L"test.zip",_vfs->GetRoot()->GetFirstChild()->GetName());
	WIN_ASSERT_EQUAL(6,_vfs->GetRoot()->GetFirstChild()->GetChildCount());
	WIN_ASSERT_STRING_EQUAL(L"game.xml",_vfs->GetFile(L"./test.zip/game.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"gameicon.png",_vfs->GetFile(L"./test.zip/gameIcon.png")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"preferences.xml",_vfs->GetFile(L"./test.zip/preferences.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"preferencetemplates.xml",_vfs->GetFile(L"test.zip/preferenceTemplates.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"gamestate.xml",_vfs->GetFile(L"test.zip/boot/gameState.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"persistency.xml",_vfs->GetFile(L"test.zip/boot/persistency.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"test.lua",_vfs->GetFile(L"test.zip/content/test.lua")->GetName());
	WIN_ASSERT_TRUE(_vfs->GetFile(L"test.zip/content")->IsFolder());
}
END_TESTF

/*
check that files inside enumeratoed archives can be read correctly
*/
BEGIN_TESTF(ZipArchiveContentTests,VFSTestFixture)
{
	_vfs->MapDirectory((Resources::Instance().RootDir()+L"../../tests/content/test.zip").c_str(),L"",NULL,false);

	IFile *file = _vfs->GetFile(L"test.zip/content/test.lua");
	file->OpenFile();
	char* data = new char[file->GetSize()];
	file->Read((void *)data,file->GetSize());
	std::string contents(data,file->GetSize());
	delete[] data;
	file->CloseFile();

	std::vector<std::string> list;
	boost::algorithm::replace_all(contents,"\r","");
	boost::algorithm::replace_all(contents,"\t","");
	boost::split(list,contents,boost::is_any_of("\n"));

	//see if the file has as many lines as we expect
	WIN_ASSERT_EQUAL(27,list.size());
	//check to see the first and last lines are as expected
	WIN_ASSERT_EQUAL("class 'ConsoleStorageListener'(MGDF.StorageListener)",list[0]);
	WIN_ASSERT_EQUAL("end",list[26]);
}
END_TESTF

/**
check that vfs filters and aliases work as expected
*/
BEGIN_TESTF(AliasAndFilterTests,VFSTestFixture)
{
	_vfs->MapDirectory((Resources::Instance().RootDir()+L"../../tests/content/test.zip").c_str(),L"",NULL,false);

	_vfs->AddAlias(L"testData",L"./test.zip");
	WIN_ASSERT_STRING_EQUAL(L"game.xml",_vfs->GetFile(L"%Testdata%/game.xml")->GetName());

	IFileIterator *files = _vfs->FindFiles(L".",_vfs->GetFilterFactory()->CreateFileExtensionInclusionFilter(L"xml"),true);
	unsigned int size=0;
	while (files->HasNext()) 
	{
		files->Next();
		++size;
	}
	WIN_ASSERT_EQUAL(5,size);
	delete files;

	files = _vfs->FindFiles(L"%testdata%",_vfs->GetFilterFactory()->CreateFileExtensionInclusionFilter(L"xml"),false);
	size=0;
	while (files->HasNext()) 
	{
		files->Next();
		++size;
	}
	WIN_ASSERT_EQUAL(3,size);
	delete files;

	files = _vfs->FindFiles(L"%testdata%",_vfs->GetFilterFactory()->CreateFileExtensionExclusionFilter(L"xml")->ChainFilter(
								 _vfs->GetFilterFactory()->CreateFileExtensionInclusionFilter(L"png")),false);
	size=0;
	while (files->HasNext()) 
	{
		files->Next();
		++size;
	}
	WIN_ASSERT_EQUAL(1,size);
	delete files;

	files = _vfs->FindFiles(L"test.zip",NULL,false);
	size=0;
	while (files->HasNext()) 
	{
		files->Next();
		++size;
	}
	WIN_ASSERT_EQUAL(6,size);
	delete files;
}
END_TESTF

/**
check that the standard filesystem is enumerated correctly by the vfs
*/
BEGIN_TESTF(FileSystemTests,VFSTestFixture)
{
	_vfs->MapDirectory((Resources::Instance().RootDir()+L"../../tests/content").c_str(),L"",NULL,false);

	WIN_ASSERT_EQUAL(6,_vfs->GetRoot()->GetChildCount());
	WIN_ASSERT_STRING_EQUAL(L"test.zip",_vfs->GetFile(L"./test.zip")->GetName());
	WIN_ASSERT_EQUAL(true,_vfs->GetFile(L"./test.zip")->IsArchive());
	WIN_ASSERT_STRING_EQUAL(L"console.xml",_vfs->GetFile(L"./console.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"preferences.xml",_vfs->GetFile(L"./preferences.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"gamestate.xml",_vfs->GetFile(L"gameState.xml")->GetName());
	WIN_ASSERT_STRING_EQUAL(L"playlist.xml",_vfs->GetFile(L"playlist.xml")->GetName());
}
END_TESTF

/**
check that files in the standard filesystem can be read from the vfs correctly
*/
BEGIN_TESTF(FileSystemContentTests,VFSTestFixture)
{
	_vfs->MapDirectory((Resources::Instance().RootDir()+L"../../tests/content").c_str(),L"",NULL,false);

	IFile *file = _vfs->GetFile(L"console.xml");
	file->OpenFile();
	char *data = new char[file->GetSize()];
	file->Read(data,file->GetSize());
	std::string contents(data,file->GetSize());
	delete[] data;
	file->CloseFile();

	std::vector<std::string> list;
	boost::algorithm::replace_all(contents,"\r","");
	boost::algorithm::replace_all(contents,"\t","");
	boost::split(list,contents,boost::is_any_of("\n"));

	//see if the file has as many lines as we expect
	WIN_ASSERT_EQUAL(15,list.size());
	//check to see the first and last lines are as expected
	WIN_ASSERT_EQUAL("<?xml version=\"1.0\" encoding=\"UTF-8\"?>",list[0]);
	WIN_ASSERT_EQUAL("</mgdf:game>",list[14]);
}
END_TESTF