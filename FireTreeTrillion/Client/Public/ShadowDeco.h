#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CShadowDeco : public CGameObject
{
public:
	struct SHADOWDECO_DESC : public GAMEOBJECT_DESC
	{
		CModel* pDecoModel = { nullptr };
	};
private:
	CShadowDeco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShadowDeco(const CShadowDeco& rhs);
	virtual ~CShadowDeco() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;

	virtual HRESULT Render_LightDepth() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float m_fWhiteColorDiffuse = {};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CShadowDeco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
