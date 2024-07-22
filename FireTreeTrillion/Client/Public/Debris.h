#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
class CEffect;
END

BEGIN(Client)

class CDebris final : public CGameObject
{
public:
	struct DEBRIS_DESC : public GAMEOBJECT_DESC
	{
		CModel* pModel = { nullptr };
	};

private:
	CDebris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDebris(const CDebris& rhs);
	virtual ~CDebris() = default;

public:
	void Set_ParticleDebris(_fvector vPosition, _float fScale = 1.f, _float2 fRandY = _float2(10.f, 20.f), _float2 fRandXZ = _float2(5.f, 10.f), _float fTotalTime = 1.f);
	void Set_ParticleEffect(_fvector vPosition, _float fScale);
	void Set_DebrisInfo(_float4 vPos, _float fScale, _float fY, _float3 vDir, _float fTotalTime = 1.f, _float fFallSpeed = 16.f);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	//_bool m_bDirectionDetermined = { false };

	_float m_fSamplingFactor = { 1.f };
	_float m_fZero = { 0.f };
	_float m_fScale = { 0.f };
	_float m_fTimeDelta = { 0.f };

	_vector m_vRotationAxis = {};

	_float m_fFallSpeed = {};
	_float m_fTotalTime = { 0.f };
	_float m_fTime = { 0.f };
	_float m_fX{}, m_fY{}, m_fZ{};

	_bool m_bSwap = { false };
	_bool m_bDrain = { false };

	list<CEffect*>	m_FXList;

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CDebris* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END

