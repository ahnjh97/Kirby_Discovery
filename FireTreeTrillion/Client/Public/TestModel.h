#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CRigidBody;
END

BEGIN(Client)

class CTestModel final : public CGameObject
{
private:
	CTestModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestModel(const CTestModel& rhs);
	virtual ~CTestModel() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void	Add_RigidBody(const wstring& KeyName, void* pArg);

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	CRigidBody*		m_pRigidBodyCom = { nullptr };
	//map<string, CRigidBody*> m_mapRigidBodies;

	class CLight*	m_pLight = { nullptr };

public:
	static CTestModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END