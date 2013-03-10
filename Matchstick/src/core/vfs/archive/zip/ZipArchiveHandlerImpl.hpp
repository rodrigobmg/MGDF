#pragma once

#include <string>
#include <vector>
#include <map>

#include <MGDF/MGDF.hpp>
#include <MGDF/MGDFVirtualFileSystem.hpp>

#include "ZipArchive.hpp"

namespace MGDF
{
namespace core
{
namespace vfs
{
namespace zip
{

/**
Creates zip archive handlers
*/
class ZipArchiveHandlerImpl: public IArchiveHandler
{
public:
	ZipArchiveHandlerImpl( IErrorHandler *errorHandler );
	virtual ~ZipArchiveHandlerImpl() {}
	virtual void Dispose();
	virtual void DisposeArchive( IFile *archive );
	virtual bool IsArchive( const wchar_t *physicalPath ) const;
	virtual IFile *MapArchive( const wchar_t * name, const wchar_t * physicalPath, IFile *parent );

private:
	std::map<ZipFileRoot *, ZipArchive *> _archives;
	std::vector<const wchar_t *> _fileExtensions;
	IErrorHandler *_errorHandler;

	/**
	get the extension of a file
	\return the extension (excluding the preceding '.' if possible, otherwise returns "" if no extension could be found
	*/
	const wchar_t *GetFileExtension( const wchar_t *file ) const;
};

IArchiveHandler *CreateZipArchiveHandlerImpl( IErrorHandler *errorHandler );

}
}
}
}