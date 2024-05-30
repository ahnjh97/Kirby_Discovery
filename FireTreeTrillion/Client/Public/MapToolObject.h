#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CMapToolObject : public CGameObject
{
public:
	struct MAPTOOLOBJECT_DESC : public GAMEOBJECT_DESC
	{
		_int iTriggerType = -1;
		_int iTriggerIndex = -1;
	};
public:
	_int Get_TriggerType() { return m_iTriggerType; }
	_int Get_TriggerIndex() { return m_iTriggerIndex; }

	void Set_TriggerType(_int iTriggerType) { m_iTriggerType = iTriggerType; }
	void Set_TriggerIndex(_int iTriggerIndex) { m_iTriggerIndex = iTriggerIndex; }

private:
	CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolObject(const CMapToolObject& rhs);
	virtual ~CMapToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_int m_iTriggerType = { -1 };
	_int m_iTriggerIndex = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CMapToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
