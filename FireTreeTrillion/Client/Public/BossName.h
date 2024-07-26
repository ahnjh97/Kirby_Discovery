#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

class CBossName final : public CGameObject
{
public:
	struct BOSSNAME : public CGameObject::GAMEOBJECT_DESC {
		_float	fScale = { 1.f };
		_vector vPosition = {};
	};

private:
	CBossName(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossName(const CBossName& rhs);
	virtual ~CBossName() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

	_float		m_fScale = { 0.f };

	_vector		m_vPosition = {};

public:
	static CBossName* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

