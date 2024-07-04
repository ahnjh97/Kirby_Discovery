#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CDecoObject : public CGameObject
{
private:
	CDecoObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecoObject(const CDecoObject& rhs);
	virtual ~CDecoObject() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual _int			Tick(_float fTimeDelta) override;
	virtual void			Late_Tick(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual HRESULT			Render_LightDepth() override;

#ifdef _DEBUG
	virtual void			Render_IMGUI() override;
#endif

	void					Set_Animation(_int iAnimIndex);

private:
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

public:
	static CDecoObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
