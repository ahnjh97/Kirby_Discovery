#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CTrigger final : public CGameObject
{
	enum TRIGGER { TRIGGER_CAM, TRIGGER_SHADER, TRIGGER_STAR, TRIGGER_END };

public:
	struct TRIGGER_DESC : GAMEOBJECT_DESC
	{
		_uint iTriggerType;
		_int iTriggerIndex = -1;
	};

private:
	CTrigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrigger(const CTrigger& rhs);
	virtual ~CTrigger() = default;

public:
	virtual HRESULT Initialize_Prototype()								override;
	virtual HRESULT Initialize(void* pArg)								override;
	virtual _int	Tick(_float fTimeDelta)								override;
	virtual void	Late_Tick(_float fTimeDelta)						override;
	virtual HRESULT Render()											override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()										override;
#endif

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };	
	CRigidBody*		m_pRigidBodyCom = { nullptr };

private:
	TRIGGER			m_eTriggerType = { TRIGGER_END };
	_int			m_iTriggerIndex = { -1 };


public:
	static CTrigger*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END

