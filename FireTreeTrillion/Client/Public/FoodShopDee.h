//커비와의 상호작용 여부 체크#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END


BEGIN(Client)

class CFoodShopDee final : public CWaddleDee
{
private:
	CFoodShopDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFoodShopDee(const CFoodShopDee& rhs);
	virtual ~CFoodShopDee() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;


	virtual void	Add_AnimEvent()	override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif


private:
	class CUI_Interactable* m_pUI_Interactable = nullptr;
	DEE_ANIM		m_eMyState = { DEEANIM_END };

	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();
	_bool	Custom_Face(_uint iMeshIndex);

	_bool	bOpenEffect = false;
	_bool	m_bCheckCollision = false;
	_bool	m_bIsInteractKirby = { FALSE }; //커비와의 상호작용 여부 체크

public:
	static CFoodShopDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
