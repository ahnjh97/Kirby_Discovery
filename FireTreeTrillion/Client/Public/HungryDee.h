#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"
#include "UI_PartTimeDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END


BEGIN(Client)

//enum class PARTTIME_ITEM;
class CHungryDee final : public CWaddleDee
{
public:
	struct HUNGRYDEE_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		_int iIdx;
	};

private:
	CHungryDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHungryDee(const CHungryDee& rhs);
	virtual ~CHungryDee() = default;

	enum WAITPOS { WAITPOS_FRONT = 2, WAITPOS_END };

public:
	static pair<_float3, vector<WAITING_INFO>> m_WaitingList;

	//이 두 변수는 싱글톤으로 뺄 거여요~
	static _int		m_iWatingNum;
	static _float	m_fWaitingTime;

	//현재 내가 기다려야 할 위치를 준다.
	_float3			Get_DestWaitingPos() { return m_WaitingList.first + m_WaitingList.second[m_iMyIdx].vPos; }
	_float			Get_WaitingTime() {return m_fWaitingTime;}
	_bool			IsFrontWaiting() { return m_iMyIdx == WAITPOS_FRONT; }
	_bool			IsSecondWaiting() { return m_iMyIdx == (WAITPOS_FRONT + 1); }

	void			Set_RenderPartObj(_bool bRender) { m_bRenderPartObj = bRender; }
	
	void			Erase_DialogUI(); //{ m_pDialogUI = nullptr; }

	void			Swap_WatingPosition();

	void			Ready_OrderUI(CUI_PartTimeDee::TYPE eType = CUI_PartTimeDee::ORDER);


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

	void			Change_Dialog(enum class PARTTIME_ITEM eItem);

	void			OnNotify();
	void			Bring_Food(enum class PARTTIME_ITEM eITEM);
	void			Set_MaskValueUI(_float _fMaskValue);

private:
	DEE_SHOPANIM	m_eMyState = { DEESHOPANIM_END };
	_int			m_iMyIdx = { -1 };
	_bool			m_bRenderPartObj = { false };

	class CUI_PartTimeDee* m_pDialogUI = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();
	_bool	Custom_Face(_uint iMeshIndex);

public:
	static CHungryDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
