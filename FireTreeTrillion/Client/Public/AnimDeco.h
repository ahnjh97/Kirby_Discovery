#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CAnimDeco : public CGameObject
{
public:
	struct ANIMDECO_DESC : public GAMEOBJECT_DESC
	{
		CModel* pAnimDecoModel = { nullptr };
	};

private:
	CAnimDeco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimDeco(const CAnimDeco& rhs);
	virtual ~CAnimDeco() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override { return OBJ_NOEVENT; }
	virtual void Late_Tick(_float fTimeDelta) override {};
	virtual HRESULT Render() override { return S_OK; }

public:
	_bool IsHidden() { if (nullptr != m_pAnimDecoModel) return m_pAnimDecoModel->IsHidden(); else return false; }
	void HideModel(); 

private:
	CModel* m_pAnimDecoModel = { nullptr };
	wstring m_wstrNonAnimDecoName;

public:
	static CAnimDeco* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END