#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSimbaRock final : public CGameObject
{
private:
	CSimbaRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSimbaRock(const CSimbaRock& rhs);
	virtual ~CSimbaRock() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	//PxRigidDynamic* m_pDynamicActor = { nullptr };

	_float m_fSamplingFactor = { 1.f };
	_float m_fZero = {};

	_float	m_fTime = {};
	_float	m_fScale = {};

public:
	void SetUpSimbaRock(_fvector vPos);
	void MoveToOrigin();

private:
	HRESULT Add_Components(const wstring& _wstrModelName);
	HRESULT Bind_ShaderResources();

public:
	static CSimbaRock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
