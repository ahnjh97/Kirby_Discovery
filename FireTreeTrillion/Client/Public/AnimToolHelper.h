#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CAnimToolHelper final : public CGameObject
{
private:
	CAnimToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimToolHelper(const CAnimToolHelper& rhs);
	virtual ~CAnimToolHelper() = default;

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg = nullptr)	override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
	//virtual void	Render_IMGUI()						override;

private:
	void			Ready_AnimObjects(const wstring& strLayerTag);

	// RENDER
	void			Render_ObjectList();
	void			Render_AnimationList(const wstring& wstrObjectTag);
	void			Render_FrameLine(class CAnimation** pModel, const string& strAnimationTag);
	//void			Render_EventList();

	void			Save();
	void			Load(const string& FileName);

private:
	vector<CGameObject*>	m_vecAnimObjects;
	CGameObject*			m_pGameObj = nullptr;

public:
	static	CAnimToolHelper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	 Clone(void* pArg) override;
	virtual void			 Free() override;

};

END

