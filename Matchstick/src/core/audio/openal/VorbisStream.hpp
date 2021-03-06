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
public:
	virtual ~VorbisStream();
	static MGDFError TryCreate( IFile *source, OpenALSoundManagerComponentImpl *manager, VorbisStream **stream );

	const wchar_t * GetName() const override final;
	float GetVolume() const override final;
	void SetVolume( float volume ) override final;
	void Stop() override final;
	void Pause() override final;
	MGDFError Play() override final;
	bool IsStopped() const override final;
	bool IsPaused() const override final;
	bool IsPlaying() const override final;
	UINT32 GetPosition() override final;
	UINT32 GetLength() override final;

	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject ) override final;
	ULONG STDMETHODCALLTYPE AddRef() override final;
	ULONG STDMETHODCALLTYPE Release() override final;
	ULONG RefCount() const { return _streamReferences; }

	void Update();
	void SetGlobalVolume( float globalVolume );

private:
	VorbisStream( IFile *source, OpenALSoundManagerComponentImpl *manager );
	MGDFError InitStream();
	void UninitStream();

	ULONG			_streamReferences;
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

	INT32			_initLevel;
	VorbisStreamState _state;
	float			_volume;
	float			_globalVolume;

	OpenALSoundManagerComponentImpl *_soundManager;

	static INT32 _references;
	static HINSTANCE _vorbisInstance;
	static LPOVCLEAR fn_ov_clear;
	static LPOVREAD fn_ov_read;
	static LPOVPCMTOTAL fn_ov_pcm_total;
	static LPOVINFO fn_ov_info;
	static LPOVCOMMENT fn_ov_comment;
	static LPOVOPENCALLBACKS fn_ov_open_callbacks;

	static unsigned long DecodeOgg( OggVorbis_File *vorbisFile, char *decodeBuffer, unsigned long bufferSize, unsigned long channels );

	static MGDFError InitVorbis();
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