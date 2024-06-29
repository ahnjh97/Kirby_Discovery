#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCarShopWallFrame final : public CPhysXObject
{
private:
	CCarShopWallFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCarShopWallFrame(const CCarShopWallFrame& rhs);
	virtual ~CCarShopWallFrame() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;

private:
	HRESULT			Add_Components(const wstring& _wstrModelName);
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

	_float			m_fWhiteColorDiffuse = {};

public:
	static CCarShopWallFrame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END