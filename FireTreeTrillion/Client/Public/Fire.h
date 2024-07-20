#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CTexture;
class CShader;
END

class CFire final : public CGameObject
{
public:
	struct FIREDESC : public GAMEOBJECT_DESC
	{
		_float4 vFirePos = { 0.f, 0.f, 0.f, 0.f };
		_float4 vFirstColor;
		_float4 vTargetColor;
		_float fUpRange;
		_float fScale;
		_float fTimeRatio = { 1.f };
	};
private:
	CFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFire(const CFire& rhs);
	virtual ~CFire() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float4 m_vTargetColor = { 0.f, 0.f, 0.f, 0.f };
	_float4	m_vFirstColor = { 0.f, 0.f, 0.f, 0.f };

	_float4 m_vColor = { 1.f, 1.f, 1.f, 1.f };

	_float	m_fMaxRange = { 0.f };

	_float	m_fMaxScale = { 0.f };
	_float	m_fScale = { 0.f };

	_float m_fDissolve = { 0.f };
	_float	m_fSpeed = { 0.f };
	_float4 m_vOriginPos = { 0.f, 0.f, 0.f, 0.f };

	_float m_fTimeDelta = { 0.f };
	_float m_fTimeRatio = { 0.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CFire* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

