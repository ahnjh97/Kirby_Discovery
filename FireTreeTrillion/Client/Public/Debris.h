#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
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
	void Set_ParticleDebris(_fvector vPosition);

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
	_float m_fZero = {};

	_vector m_vRotationAxis = {};
	_float m_fTotalTime = {};
	_float m_fTime = {};
	_float m_fX{}, m_fY{}, m_fZ{};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CDebris* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END

