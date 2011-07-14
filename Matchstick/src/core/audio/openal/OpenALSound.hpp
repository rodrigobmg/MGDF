#pragma once

#include "al.h"
#include "alc.h"
#include "AL/alut.h"

#include <hash_map>
#include <vector>

#include <MGDF/MGDF.hpp>
#include "../MGDFSoundManagerComponent.hpp"
#include "OpenALSoundManagerComponent.hpp"

namespace MGDF { namespace core { namespace audio { namespace openal_audio {

class OpenALSound: public DisposeImpl<ISound> {
friend class OpenALSoundManagerComponentImpl;
public:
	virtual ~OpenALSound();
	OpenALSound(IFile *source,OpenALSoundManagerComponentImpl *manager,int priority);

	virtual const wchar_t *GetName() const;	
	virtual Vector *GetPosition() const;
	virtual Vector *GetVelocity() const;
	virtual float GetInnerRange() const;
	virtual void SetInnerRange(float sourceRelative);
	virtual float GetOuterRange() const;
	virtual void SetOuterRange(float sourceRelative);
	virtual bool GetSourceRelative() const;
	virtual void SetSourceRelative(bool sourceRelative);
	virtual float GetVolume() const;
	virtual void SetVolume(float volume);
	virtual float GetPitch() const;
	virtual void SetPitch(float pitch);
	virtual void SetPriority(int priority);
	virtual int GetPriority() const;
	virtual bool GetLooping() const;
	virtual void SetLooping(bool looping);
	virtual void Stop();
	virtual void Pause();
	virtual void Play();
	virtual bool IsStopped() const;
	virtual bool IsPaused() const;
	virtual bool IsPlaying() const;
	virtual bool IsActive() const;

	virtual void Dispose();
private:
	float GetAttenuatedVolume();
	void Reactivate();
	void Deactivate();
	void SetGlobalVolume(float globalVolume);
	void Update(float attenuationFactor);

	std::wstring _name;
	OpenALSoundManagerComponentImpl *_soundManager;
	ALuint _sourceId,_bufferId;
	float _innerRange,_outerRange,_volume,_globalVolume,_attenuationFactor,_pitch;
	bool _isActive,_isSourceRelative,_isLooping,_wasPlaying,_startPlaying;
	int _priority;
	Vector *_position;
	Vector *_velocity;
};

}}}}