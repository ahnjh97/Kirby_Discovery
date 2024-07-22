#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CSimbaLaser final : public CGameObject
{
private:
	CSimbaLaser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSimbaLaser(const CSimbaLaser& rhs);
	virtual ~CSimbaLaser() = default;

public:
	void HideLaser();

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

private:
	HRESULT			Add_Components(const wstring& _wstrModelName);
	HRESULT			Bind_ShaderResources();

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };
	CTexture*	m_pTextureCom = { nullptr };
	PxRigidDynamic* m_pDynamicActor = { nullptr };

	_float		m_fWhiteColorDiffuse = {};

public:
	static CSimbaLaser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

