#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class COrbitingCamera : public CPartObject
{
public:
	void Set_Radius(_float fRadius) { m_fRadius = fRadius; }
	
	_float3 Get_OrbitingCameraPos();
	void Set_OrbitingCameraPos(_fvector vPos) { if (nullptr != m_pTransformCom) 
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos); };

private:
	COrbitingCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	COrbitingCamera(const COrbitingCamera& rhs);
	virtual ~COrbitingCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float m_fRadius = { };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static COrbitingCamera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


