#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBossClone final : public CGameObject
{
public:
	struct BOSSCLONE_DESC : public CGameObject::GAMEOBJECT_DESC {
		_vector	vPosition = {};
		_vector vTargetPos = {};
		_vector	vLook = {};
	};

private:
	CBossClone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossClone(const CBossClone& rhs);
	virtual ~CBossClone() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

	_float		m_fTimeDelta = { 0.f };
	_float		m_fDelayTime = { 0.f };
	_float		m_fSpeed = { 1.f };
	_float		m_fSecondSpeed = { 1.f };

	_vector		m_vPosition = {};
	_vector		m_vTargetPos = {};
	_vector		m_vLook = {};

	_bool		m_bMove = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CBossClone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END