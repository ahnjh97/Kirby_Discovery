#pragma once
#include "Client_Defines.h"
#include "WaddleDee.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class COriginalDee final : public CWaddleDee
{
private:
	COriginalDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	COriginalDee(const COriginalDee& rhs);
	virtual ~COriginalDee() = default;

public:
	static pair<_float3, vector<TOWN_POINT_INFO>> m_TownPoints;

	//목적지의 위치를 가져온다
	virtual _float3					Make_DestPos() override;
	//목적지에 도착하면 뭐 할 지 만들어준다~
	virtual pair<DEE_ANIM, _bool>	Make_WhatToDo() override;

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
	DEE_ANIM		m_eMyState = { DEEANIM_END };

	//이 포인트는 처음에 걷는 와들디로 시작한 놈들만 갱신됩니다.
	//그렇지 않을 시 움직이지 않습니다.
	TOWN_POINT		m_eDestPoint = { TOWNPOINT_END };
	TOWN_POINT		m_ePrePoint = { TOWNPOINT_END };

	HRESULT Add_Components();
	HRESULT Add_PartObjects(DEE_CHARACTER eCharacter);
	HRESULT Bind_ShaderResources();

	void	SetUp_FSM();
	void	Make_InitialState(DEE_CHARACTER iDeeCharacter);
	_bool	Custom_Face(_uint iMeshIndex);

	void Draw_TownPoints();
public:
	static COriginalDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

