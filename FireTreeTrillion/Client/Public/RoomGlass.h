#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CRoomGlass final : public CGameObject
{
private:
	CRoomGlass(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRoomGlass(const CRoomGlass& rhs);
	virtual ~CRoomGlass() = default;

public:
	void Activate(CGameObject* pObj);

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;

private:
	HRESULT				Add_Components(wstring& wstrModelName);
	HRESULT				Bind_ShaderResources();

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	_float m_fWhiteColorDiffuse = {};

public:
	static CRoomGlass* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END