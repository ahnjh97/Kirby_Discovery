#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)
class CSkySphere final : public CGameObject
{
private:
	CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkySphere(const CSkySphere& rhs);
	virtual ~CSkySphere() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	//class CCamera* m_pHifiCam = { nullptr };


	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CSkySphere* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END