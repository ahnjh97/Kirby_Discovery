#include "SoundManager.h"
#include "tchar.h"
//#include "fmod_dsp_effects.h"
//#include "fmod_dsp.h"

CSound_Manager::CSound_Manager()
{
	m_pSystem = nullptr;
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_System_Create(&m_pSystem, FMOD_VERSION);

	// 1. 시스템 포인터, 2. 사용할 가상채널 수 , 초기화 방식) 
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

	LoadSoundFile();

	FMOD_System_CreateDSPByType(m_pSystem, FMOD_DSP_TYPE_LOWPASS, &m_LowPassFilter);
	FMOD_DSP_SetParameterFloat(m_LowPassFilter, FMOD_DSP_LOWPASS_CUTOFF, m_fCurLowPass);
	//FMOD_Channel_AddDSP(*m_pChannelArr, BGM, m_LowPassFilter);
	return S_OK;
}

void CSound_Manager::Tick(_float fTimeDelta)
{
	m_fCurLowPass += (m_fDestLowPass - m_fCurLowPass) * fTimeDelta * 4.f;
	FMOD_DSP_SetParameterFloat(m_LowPassFilter, FMOD_DSP_LOWPASS_CUTOFF, m_fCurLowPass);
}

void CSound_Manager::ApplyLowPass(_bool bSet)
{

	//FMOD_Channel_AddDSP(*m_pChannelArr, BGM, m_LowPassFilter);

	if (m_bSetLowPass == bSet)
		return;

	if (bSet)
	{
		//FMOD_Channel_AddDSP(*m_pChannelArr, BGM, m_LowPassFilter);
		m_fDestLowPass = 800.f;
		//FMOD_DSP_SetParameterFloat(m_LowPassFilter, FMOD_DSP_LOWPASS_CUTOFF, m_fDestLowPass);
		m_bSetLowPass = true;
	}
	else
	{
		//FMOD_Channel_RemoveDSP(*m_pChannelArr, m_LowPassFilter);
		m_fDestLowPass = 10000.f;
		//FMOD_DSP_SetParameterFloat(m_LowPassFilter, FMOD_DSP_LOWPASS_CUTOFF, m_fDestLowPass);
		m_bSetLowPass = false;
	}

}

void CSound_Manager::AddLowPass()
{
	FMOD_Channel_AddDSP(*m_pChannelArr, CHANNEL_BGM, m_LowPassFilter);

}

int CSound_Manager::SetVolume(CHANNELID eID, _float _vol)
{
	FMOD_Channel_SetVolume(m_pChannelArr[eID], _vol);

	return 0;

}

int CSound_Manager::VolumeUp(CHANNELID eID, _float _vol)
{
	if (m_volume < SOUND_MAX) {
		m_volume += _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

int CSound_Manager::VolumeDown(CHANNELID eID, _float _vol)
{
	if (m_volume > SOUND_MIN) {
		m_volume -= _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

int CSound_Manager::VolumeMin(CHANNELID eID)
{
	if (m_volume > SOUND_MIN) {
		m_volume = SOUND_MIN;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

int CSound_Manager::VolumeRestore(CHANNELID eID)
{
	FMOD_Channel_SetVolume(m_pChannelArr[eID], 0.5f);

	return 0;
}

int CSound_Manager::BGMVolumeUp(_float _vol)
{
	if (m_BGMvolume < SOUND_MAX) {
		m_BGMvolume += _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[CHANNEL_BGM], m_BGMvolume);

	return 0;
}

int CSound_Manager::BGMVolumeDown(_float _vol)
{
	if (m_BGMvolume > SOUND_MIN) {
		m_BGMvolume -= _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[CHANNEL_BGM], m_BGMvolume);

	return 0;
}

int CSound_Manager::Pause(CHANNELID eID)
{
	m_bPause = !m_bPause;
	FMOD_Channel_SetPaused(m_pChannelArr[eID], m_bPause);

	return 0;
}


void CSound_Manager::PlayMySound(TCHAR* pSoundKey, CHANNELID eID, _float _vol)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
		{
			return !lstrcmp(pSoundKey, iter.first);
		});

	if (iter == m_mapSound.end())
		return;

	FMOD_BOOL bPlay = FALSE;
	if (FMOD_Channel_IsPlaying(m_pChannelArr[eID], &bPlay))
	{
		FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, FALSE, &m_pChannelArr[eID]);
		if (_vol >= SOUND_MAX)
			_vol = 1.f;
		else if (_vol <= SOUND_MIN)
			_vol = 0.f;
		FMOD_Channel_SetVolume(m_pChannelArr[eID], _vol);
	}
	FMOD_System_Update(m_pSystem);
}

void CSound_Manager::PlaySound_Free(TCHAR* pSoundKey, _float fvolume)
{
	_int freeChannel = FindFreeChannel();

	if (freeChannel == 200)
		return;

	FMOD_SOUND* sound = FindSoundByKey(pSoundKey);
	if (sound != nullptr)
	{
		FMOD_BOOL bPlay = FALSE;
		FMOD_System_PlaySound(m_pSystem, sound, nullptr, FALSE, &m_pChannelArr[freeChannel]);
		if (fvolume >= SOUND_MAX)
			fvolume = 1.f;
		else if (fvolume <= SOUND_MIN)
			fvolume = 0.f;
		FMOD_Channel_SetVolume(m_pChannelArr[freeChannel], fvolume);
	}

	FMOD_System_Update(m_pSystem);
}

_int CSound_Manager::FindFreeChannel()
{
	for (int i = 0; i < 200; ++i)
	{
		FMOD_BOOL bPlay = FALSE;
		FMOD_Channel_IsPlaying(m_pChannelArr[i], &bPlay);
		if (!bPlay)
		{
			return i;
		}
	}
	return 200;
}

FMOD_SOUND* CSound_Manager::FindSoundByKey(TCHAR* pSoundKey)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
		{
			return !lstrcmp(pSoundKey, iter.first);
		});

	if (iter == m_mapSound.end())
		return nullptr;

	return iter->second;
}


void CSound_Manager::PlayBGM(TCHAR* pSoundKey)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
		{
			return !lstrcmp(pSoundKey, iter.first);
		});

	if (iter == m_mapSound.end())
		return;

	FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, FALSE, &m_pChannelArr[CHANNEL_BGM]);
	FMOD_Channel_SetMode(m_pChannelArr[CHANNEL_BGM], FMOD_LOOP_NORMAL);
	FMOD_System_Update(m_pSystem);
}

void CSound_Manager::StopSound(CHANNELID eID)
{
	FMOD_Channel_Stop(m_pChannelArr[eID]);
}

void CSound_Manager::StopAll()
{
	for (int i = 0; i < 200; ++i)
		FMOD_Channel_Stop(m_pChannelArr[i]);
}

void CSound_Manager::LoadSoundFile()
{
	_tfinddata64_t fd;

	__int64 handle = _tfindfirst64(L"../Bin/Sound/*.*", &fd);
	if (handle == -1 || handle == 0)
		return;

	int iResult = 0;

	char szCurPath[128] = "../Bin/Sound/";
	char szFullPath[128] = "";
	char szFilename[MAX_PATH];

	while (iResult != -1)
	{
		WideCharToMultiByte(CP_UTF8, 0, fd.name, -1, szFilename, sizeof(szFilename), NULL, NULL);
		strcpy_s(szFullPath, szCurPath);
		strcat_s(szFullPath, szFilename);
		FMOD_SOUND* pSound = nullptr;

		FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem, szFullPath, FMOD_DEFAULT, 0, &pSound);
		if (eRes == FMOD_OK)
		{
			int iLength = strlen(szFilename) + 1;

			TCHAR* pSoundKey = new TCHAR[iLength];
			ZeroMemory(pSoundKey, sizeof(TCHAR) * iLength);
			MultiByteToWideChar(CP_ACP, 0, szFilename, iLength, pSoundKey, iLength);

			m_mapSound.emplace(pSoundKey, pSound);
		}
		iResult = _tfindnext64(handle, &fd);
	}

	FMOD_System_Update(m_pSystem);
	_findclose(handle);
}

CSound_Manager* CSound_Manager::Create()
{
	CSound_Manager* pInstance = new CSound_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSound_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSound_Manager::Free()
{
	FMOD_DSP_Release(m_LowPassFilter);

	for (auto& Mypair : m_mapSound)
	{
		delete[] Mypair.first;
		FMOD_Sound_Release(Mypair.second);
	}
	m_mapSound.clear();



	FMOD_System_Release(m_pSystem);
	FMOD_System_Close(m_pSystem);

}


