#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END


BEGIN(Client)

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

public:
	static pair<_float3, vector<WAITING_INFO>> m_WaitingList;

	//현재 내가 기다려야 할 위치를 준다.
	_float3		Get_DestWaitingPos() { return m_WaitingList.first + m_WaitingList.second[m_iMyIdx].vPos; }


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
	DEE_SHOPANIM	m_eMyState = { DEESHOPANIM_END };
	_int			m_iMyIdx = { -1 };

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
