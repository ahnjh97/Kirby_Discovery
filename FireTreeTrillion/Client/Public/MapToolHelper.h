#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CMapToolHelper final : public CGameObject
{
private:
	CMapToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolHelper(const CMapToolHelper& rhs);
	virtual ~CMapToolHelper() = default;

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg = nullptr)	override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
	virtual void	Render_IMGUI()						override;

private:
	void SetUpTxtVectors(TYPE _eType);

	void Menu_NonAnimModels();

private:
	vector<string> m_vecAnimTxts;
	vector<string> m_vecNonAnimTxts;
	string m_strCurModel;

public:
	static	CMapToolHelper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void			Free() override;

};

END



