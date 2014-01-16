#pragma once

#include <MGDF/MGDF.hpp>
#include <al.h>
#include <alc.h>
#include <AL/alut.h>
#include <Vorbis/vorbisfile.h>
#include "OpenALSoundManagerComponent.hpp"

namespace MGDF
{
namespace core
{
namespace audio
{
namespace openal_audio
{

#define VORBIS_BUFFER_COUNT 4
#define FADE_DURATION 5000

typedef INT32( *LPOVCLEAR )( OggVorbis_File *vf );
typedef long( *LPOVREAD )( OggVorbis_File *vf, char *buffer, INT32 length, INT32 bigendianp, INT32 word, INT32 sgned, INT32 *bitstream );
typedef ogg_int64_t ( *LPOVPCMTOTAL )( OggVorbis_File *vf, INT32 i );
typedef vorbis_info * ( *LPOVINFO )( OggVorbis_File *vf, INT32 link );
typedef vorbis_comment * ( *LPOVCOMMENT )( OggVorbis_File *vf, INT32 link );
typedef INT32( *LPOVOPENCALLBACKS )( void *datasource, OggVorbis_File *vf, char *initial, long ibytes, ov_callbacks callbacks );
enum VorbisStreamState {NOT_STARTED, PLAY, PAUSE, STOP};

class VorbisStream: public ISoundStream
{
	friend class OpenALSoundManagerComponentImpl;
public:
	virtual ~VorbisStream();
	VorbisStream( IFile *source, OpenALSoundManagerComponentImpl *manager );

	const wchar_t * GetName() const override;
	float GetVolume() const override;
	void SetVolume( float volume ) override;
	void Stop() override;
	void Pause() override;
	MGDFError Play() override;
	bool IsStopped() const override;
	bool IsPaused() const override;
	bool IsPlaying() const override;
	UINT32 GetPosition() override;
	UINT32 GetLength() override;

	void Dispose() override;
	void Update();
private:
	IFile			*_dataSource;
	IFileReader		*_reader;
	ALuint		    _buffers[VORBIS_BUFFER_COUNT];
	ALuint		    _source;
	ALint			_totalBuffersProcessed;
	unsigned long	_frequency;
	unsigned long	_format;
	unsigned long	_channels;
	unsigned long	_bufferSize;
	char			*_decodeBuffer;
	OggVorbis_File	_vorbisFile;
	vorbis_info		*_vorbisInfo;

	INT32 _initLevel;
	VorbisStreamState _state;
	float _volume, _globalVolume;

	OpenALSoundManagerComponentImpl *_soundManager;

	void InitStream();
	void UninitStream();
	void SetGlobalVolume( float globalVolume );

	static INT32 _references;
	static HINSTANCE _vorbisInstance;
	static LPOVCLEAR fn_ov_clear;
	static LPOVREAD fn_ov_read;
	static LPOVPCMTOTAL fn_ov_pcm_total;
	static LPOVINFO fn_ov_info;
	static LPOVCOMMENT fn_ov_comment;
	static LPOVOPENCALLBACKS fn_ov_open_callbacks;

	static unsigned long DecodeOgg( OggVorbis_File *vorbisFile, char *decodeBuffer, unsigned long bufferSize, unsigned long channels );

	static bool InitVorbis();
	static void UninitVorbis();
	static void Swap( short &s1, short &s2 );

	//vorbis callbacks to read from the MGDF virtual file
	static size_t ov_read_func( void *ptr, size_t size, size_t nmemb, void *datasource );
	static int ov_seek_func( void *datasource, ogg_int64_t offset, int whence );
	static int ov_close_func( void *datasource );
	static long ov_tell_func( void *datasource );
};

}
}
}
}