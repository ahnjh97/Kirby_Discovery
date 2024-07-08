#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

class CPopStar_StarSmall final : public CGameObject
{
private:
	CPopStar_StarSmall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPopStar_StarSmall(const CPopStar_StarSmall& rhs);
	virtual ~CPopStar_StarSmall() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CPopStar_StarSmall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

