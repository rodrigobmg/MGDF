#pragma once

#include <al.h>
#include <alc.h>
#include <AL/alut.h>

#include <vector>

#include <MGDF/MGDF.hpp>
#include "../MGDFSoundManagerComponent.hpp"
#include "OpenALSoundManagerComponent.hpp"

using namespace DirectX;

namespace MGDF
{
namespace core
{
namespace audio
{
namespace openal_audio
{

class OpenALSound: public ISound
{
	friend class OpenALSoundManagerComponentImpl;
public:
	virtual ~OpenALSound();
	OpenALSound( IFile *source, OpenALSoundManagerComponentImpl *manager, INT32 priority );

	const wchar_t *GetName() const override;
	XMFLOAT3 *GetPosition() override;
	XMFLOAT3 *GetVelocity() override;
	float GetInnerRange() const override;
	void SetInnerRange( float sourceRelative ) override;
	float GetOuterRange() const override;
	void SetOuterRange( float sourceRelative ) override;
	bool GetSourceRelative() const override;
	void SetSourceRelative( bool sourceRelative ) override;
	float GetVolume() const override;
	void SetVolume( float volume ) override;
	float GetPitch() const override;
	void SetPitch( float pitch ) override;
	void SetPriority( INT32 priority ) override;
	INT32 GetPriority() const override;
	bool GetLooping() const override;
	void SetLooping( bool looping ) override;
	void Stop() override;
	void Pause() override;
	void Play() override;
	bool IsStopped() const override;
	bool IsPaused() const override;
	bool IsPlaying() const override;
	bool IsActive() const override;
	void Dispose() override;
private:
	float GetAttenuatedVolume();
	void Reactivate();
	void Deactivate();
	void SetGlobalVolume( float globalVolume );
	void Update( float attenuationFactor );

	const wchar_t *_name;
	OpenALSoundManagerComponentImpl *_soundManager;
	ALuint _sourceId, _bufferId;
	float _innerRange, _outerRange, _volume, _globalVolume, _attenuationFactor, _pitch;
	bool _isActive, _isSourceRelative, _isLooping, _wasPlaying, _startPlaying;
	INT32 _priority;
	XMFLOAT3 _position;
	XMFLOAT3 _velocity;
};

}
}
}
}