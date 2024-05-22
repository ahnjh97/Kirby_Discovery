#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CRigidBody;
class CCharacterController;
END

BEGIN(Client)

class CTestModel final : public CGameObject
{
private:
	CTestModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestModel(const CTestModel& rhs);
	virtual ~CTestModel() = default;

public:
	void IsJump(_bool _isJump) {
		m_isJump = _isJump;
	}

public:
	virtual HRESULT Initialize_Prototype()		 override;
	virtual HRESULT Initialize(void* pArg)		 override;
	virtual _int	Tick(_float fTimeDelta)		 override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()					 override;
	virtual HRESULT Render_LightDepth()			 override;
	virtual void	Render_IMGUI()				 override;

public:
	void SetOn_Slope();

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void	Add_RigidBody(const wstring& KeyName, void* pArg);

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	class CLight*	m_pLight = { nullptr };

	CRigidBody*		m_pRigidBodyCom = { nullptr };
	//map<string, CRigidBody*> m_mapRigidBodies;
	CCharacterController* m_pControllerCom = { nullptr };

	_int		m_iTestAnim = { 0 };
	_bool		m_isJump = { false };

	_float		m_fSpeed = { 0.f };
	_float		m_fFallVelocity = { 0.f };
	_vector		m_vPos = {};

public:
	static CTestModel*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END