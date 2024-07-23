#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Fire.h"

BEGIN(Engine)

END

BEGIN(Client)

class CUnKnownFireMan final : public CGameObject
{
public:
	struct UNKNOWNFIREDESC : public GAMEOBJECT_DESC
	{
		_uint iFireCount = { 0 };
		_float4 vFirePos = { 0.f, 0.f, 0.f, 0.f };
		_float4 vFireColor;
		_float4 vTargetColor;
		_float fUpRange;
		_float fScale;
		_float fTimeRatio = { 1.f };

		_float4 vMoveDir = { 0.f, 0.f, 0.f, 0.f };
	};

private:
	CUnKnownFireMan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUnKnownFireMan(const CUnKnownFireMan& rhs);
	virtual ~CUnKnownFireMan() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	vector<CFire*> m_vecMyFires;

public:
	static CUnKnownFireMan* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
