#pragma once

namespace MGDF
{

struct Error {
	UINT32 Code;
	char *Sender;
	char *Description;
};

//error codes
#define MGDF_ERR_ERROR_ALLOCATING_BUFFER 1001
#define MGDF_ERR_NO_FREE_SOURCES 1002
#define MGDF_ERR_VORBIS_LIB_LOAD_FAILED 1003
#define MGDF_ERR_INVALID_FORMAT 1004
#define MGDF_ERR_INVALID_ARCHIVE 1005
#define MGDF_ERR_INVALID_ARCHIVE_FILE 1006
#define MGDF_ERR_INVALID_FILE 1007
#define MGDF_ERR_NO_PENDING_SAVE 1008
#define MGDF_ERR_INVALID_SAVE_NAME 1009
#define MGDF_ERR_ARCHIVE_FILE_TOO_LARGE 1010
}