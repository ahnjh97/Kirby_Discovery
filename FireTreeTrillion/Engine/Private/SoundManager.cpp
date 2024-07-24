#include "SoundManager.h"
#include "tchar.h"
//#include "fmod_dsp_effects.h"
//#include "fmod_dsp.h"

#include "Utils.h"

CSound_Manager::CSound_Manager()
{
	m_pSystem = nullptr;
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_System_Create(&m_pSystem, FMOD_VERSION);

	// 1. 시스템 포인터, 2. 사용할 가상채널 수 , 초기화 방식) 
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

	//LoadSoundFile();
	LoadSoundFile(L"../../../Resources/Sounds");

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

//wstring CSound_Manager::Get_CurSound(CHANNELID eID)
//{
//	m_pChannelArr[eID]->
//
//}

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

void CSound_Manager::Pause(CHANNELID eID, _bool bStop)
{
	FMOD_Channel_SetPaused(m_pChannelArr[eID], bStop);
}

_bool CSound_Manager::IsChannelPaused(CHANNELID eID)
{
	FMOD_BOOL isPaused = FALSE;
	FMOD_Channel_GetPaused(m_pChannelArr[eID], &isPaused);
	return isPaused;
}

void CSound_Manager::PlayMySound(TCHAR* pSoundKey, CHANNELID eID, _float _vol)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	StopSound(eID);

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
    FMOD_Channel_SetVolume(m_pChannelArr[CHANNEL_BGM], 0.5f);

	FMOD_System_Update(m_pSystem);
}

// BGM을 재생하는 채널이 CHANNEL_BGM이 아닐 경우, 해당 함수를 사용할 수 있습니다.
void CSound_Manager::PlayBGM(CHANNELID eID, TCHAR* pSoundKey)
{
	if (eID != CHANNEL_BGM && eID != CHANNEL_BGM_SUB && eID != CHANNEL_BGM_STREAMING)
	{
		ALARM_FAIL("PlayBGM에서 채널을 넣어줄 대 BGM채널 enum값을 넣어주지 않았습니다.");
		return;
	}

	map<TCHAR*, FMOD_SOUND*>::iterator iter;
	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
		{
			return !lstrcmp(pSoundKey, iter.first);
		});

	if (iter == m_mapSound.end())
		return;

	FMOD_System_PlaySound(m_pSystem, iter->second, nullptr, FALSE, &m_pChannelArr[eID]);
	FMOD_Channel_SetMode(m_pChannelArr[eID], FMOD_LOOP_NORMAL);
	FMOD_Channel_SetVolume(m_pChannelArr[eID], 0.f);

	FMOD_System_Update(m_pSystem);
}

/// <summary> 특정 채널을 목표 수치까지 볼륨을 점진적으로 올린다. </summary>
/// <param name="eID"> 특정 채널 </param>
/// <param name="targetVolume"> 목표 수치 </param>
/// <param name="fAddValue"> 점진적 수치 </param>
void CSound_Manager::PlaySmoothUp(CHANNELID eID, _float targetVolume, _float fAddValue)
{
	_float volume = 0.f;
	FMOD_Channel_GetVolume(m_pChannelArr[eID], &volume);

	if(volume < targetVolume)
	{
		volume += fAddValue;
		if (volume > targetVolume)
			volume = targetVolume;

		FMOD_Channel_SetVolume(m_pChannelArr[eID], volume);
	}
}

// 특정 볼륨까지 점진적으로 내려갑니다.
void CSound_Manager::PlaySmoothDown(CHANNELID eID, _float targetVolume, _float fMinusValue)
{
	_float volume = 0.f;
	FMOD_Channel_GetVolume(m_pChannelArr[eID], &volume);

	if (volume > targetVolume)
	{
		volume -= fMinusValue;
		if (volume < targetVolume)
			volume = targetVolume;

		FMOD_Channel_SetVolume(m_pChannelArr[eID], volume);
	}
}

// 볼륨이 0을 향해 점진적으로 내려갑니다
void CSound_Manager::PlaySmoothKill(CHANNELID eID, _float fMinusValue)
{
	_float volume = 0.f;
	FMOD_Channel_GetVolume(m_pChannelArr[eID], &volume);

	if (volume > 0.f)
	{
		volume -= fMinusValue;
		if (volume < 0.f)
		{
			FMOD_Channel_Stop(m_pChannelArr[eID]);
			volume = 0.f;
			return;
		}
		FMOD_Channel_SetVolume(m_pChannelArr[eID], volume);
	}
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

	__int64 handle = _tfindfirst64(L"../../../Resources/Sounds/*.*", &fd);
	//__int64 handle = _tfindfirst64(L"../Bin/Sound/*.*", &fd);
	if (handle == -1 || handle == 0)
		return;

	int iResult = 0;

	char szCurPath[128] = "../../../Resources/Sounds/";
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

void CSound_Manager::LoadSoundFile(const wchar_t* szDir)
{
	_wfinddata64_t fd;

	wchar_t szSearchPath[MAX_PATH];
	swprintf_s(szSearchPath, L"%s/*.*", szDir);

	__int64 handle = _wfindfirst64(szSearchPath, &fd);
	if (handle == -1 || handle == 0)
		return;

	int iResult = 0;

	while (iResult != -1)
	{
		// 디렉토리인지 확인합니다.0
		if (fd.attrib & _A_SUBDIR)
		{
			// . 및 .. 디렉토리는 무시합니다.
			if (wcscmp(fd.name, L".") != 0 && wcscmp(fd.name, L"..") != 0)
			{
				// 새로운 디렉토리 경로를 조합합니다.
				wchar_t szNewDir[MAX_PATH];
				swprintf_s(szNewDir, L"%s/%s", szDir, fd.name);

				// 재귀적으로 함수를 호출하여 서브디렉토리를 탐색합니다.
				LoadSoundFile(szNewDir);
			}
		}
		else
		{
			wchar_t szFullPath[MAX_PATH];
			swprintf_s(szFullPath, L"%s/%s", szDir, fd.name);

			char szFullPathMB[MAX_PATH];
			WideCharToMultiByte(CP_UTF8, 0, szFullPath, -1, szFullPathMB, sizeof(szFullPathMB), NULL, NULL);

			FMOD_SOUND* pSound = nullptr;
			FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem, szFullPathMB, FMOD_DEFAULT, 0, &pSound);
			if (eRes == FMOD_OK)
			{
				int iLength = wcslen(fd.name) + 1;
				wchar_t* pSoundKey = new wchar_t[iLength];
				wcscpy_s(pSoundKey, iLength, fd.name);

				m_mapSound.emplace(pSoundKey, pSound);
			}
		}

		iResult = _wfindnext64(handle, &fd);
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


 