#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CToolHelper final : public CGameObject
{
private:
    CToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CToolHelper(const CToolHelper& rhs);
    virtual ~CToolHelper() = default;

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg = nullptr)	override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()						override;
#endif

public:
	void			Save();
	void			Load(const string& FileName);
	
private:
	vector<CGameObject*>	m_vecUIObjects;

public:
	static	CToolHelper*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END

