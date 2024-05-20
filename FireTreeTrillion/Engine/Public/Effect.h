#pragma once
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CEffect : public CGameObject
{
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		//이펙트 재생을 시작하는 딜레이
		_float fStartDelay = { 0.f };
		

		//이펙트의 기본 시작 크자이
		_float3 vInitPos = { 0.f, 0.f, 0.f };
		_float3 vInitRot = { 0.f, 0.f, 0.f };
		_float3 vInitScale = { 1.f, 1.f, 1.f };

		//텍스쳐가 흑백 or RGB로 이루어져 있는 경우, 설정할 색상 값
		
		//흑백일 경우 흰색, RGB일 경우 R 색상에 해당
		_float4 vRColor = { 0.f, 0.f, 0.f, 1.f };
		_float4 vGColor = { 0.f, 0.f, 0.f, 1.f };
		_float4 vBColor = { 0.f, 0.f, 0.f, 1.f };

		string strFXName = { "NONE" };
		string strBufferTag = { "NONE" };
		string strTexTag = { "NONE" };
		string strMaskTexTag = { "NONE" };

		vector <FX_KEYFRAME> vecKeyframes[];

	}EFFECT_DESC;
};

END