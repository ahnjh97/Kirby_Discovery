#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)
class CSkySphereSub final : public CGameObject
{
public:
	enum MOD_TYPE { MOD_FRAME, MOD_PILLER, MOD_NONE };
	enum TEX_TYPE { TEX_DIFFUSE, TEX_NORMAL, TEX_EMISSIVE, TEX_HEIGHT, TEX_MRA, TEX_NONE };
	
private:
	CSkySphereSub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkySphereSub(const CSkySphereSub& rhs);
	virtual ~CSkySphereSub() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom[MOD_NONE] = { nullptr };

	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();

public:
	static CSkySphereSub*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

//private:
//	LEVEL					m_eCurLevel = { LEVEL_END };
//	string					m_strModelTag = { "" };
//	string					m_strTextureTag = { "" };
};

END