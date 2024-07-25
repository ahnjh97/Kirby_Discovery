#pragma once
#include "Base.h"
#include "Engine_Defines.h"

BEGIN(Engine)

#define SOUND_MAX 1.0f
#define SOUND_MIN 0.0f
#define SOUND_DEFAULT 0.5f
#define SOUND_WEIGHT 0.1f

class ENGINE_DLL CSound_Manager final : public CBase
{
public:
private:
	CSound_Manager();
	~CSound_Manager() = default;

public:
	HRESULT Initialize();
	void  Tick(_float fTimeDelta);

	_float Get_Volume(CHANNELID eID);
	int  SetVolume(CHANNELID eID, _float _vol);

	int  VolumeUp(CHANNELID eID, _float _vol);
	int  VolumeDown(CHANNELID eID, _float _vol);
	int  VolumeMin(CHANNELID eID);
	int  VolumeRestore(CHANNELID eID);
	int  BGMVolumeUp(_float _vol);
	int  BGMVolumeDown(_float _vol);
	int  Pause(CHANNELID eID);
	void Pause(CHANNELID eID, _bool bStop);
	_bool IsChannelPaused(CHANNELID eID);

	void PlayMySound(TCHAR* pSoundKey, CHANNELID eID, _float _vol);
	void PlayBGM(TCHAR* pSoundKey, _float _vol = 0.5f);
	void PlayBGM(CHANNELID eID, TCHAR* pSoundKey, _float _vol = 0.5f);

	void LoopSound(CHANNELID eID, TCHAR* pSoundKey, _float _vol = 0.5f);

	void PlaySmoothUp(CHANNELID eID, _float targetVolume, _float fAddValue);
	void PlaySmoothDown(CHANNELID eID, _float targetVolume, _float fMinusValue);
	void PlaySmoothKill(CHANNELID eID, _float fMinusValue);

	void StopSound(CHANNELID eID);
	void StopAll();
	void PlaySound_Free(TCHAR* pSoundKey, _float fvolume);
	_int FindFreeChannel();
	FMOD_SOUND* FindSoundByKey(TCHAR* pSoundKey);

	void ApplyLowPass(_bool bSet);
	void AddLowPass();

	wstring Get_CurSound(CHANNELID eID);

private:
	float m_volume = SOUND_DEFAULT;
	_float m_fCurLowPass = { 10000.f };
	_float m_fDestLowPass = { 10000.f };
	float m_BGMvolume = SOUND_DEFAULT;
	_bool m_bSetLowPass = { false };
	FMOD_BOOL m_bool;

private:
	void LoadSoundFile();
	void LoadSoundFile(const char* szDir);
	void LoadSoundFile(const wchar_t* szDir);

private:
	// 사운드 리소스 정보를 갖는 객체 
	map<TCHAR*, FMOD_SOUND*> m_mapSound;
	// FMOD_CHANNEL : 재생하고 있는 사운드를 관리할 객체 
	FMOD_CHANNEL* m_pChannelArr[200];
	// 사운드 ,채널 객체 및 장치를 관리하는 객체 
	FMOD_SYSTEM* m_pSystem;
	_bool		m_bPause = false;

	FMOD_DSP* m_LowPassFilter;
public:
	static CSound_Manager* Create();
	virtual void Free() override;
};

END