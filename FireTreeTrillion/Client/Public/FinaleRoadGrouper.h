#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CFinaleRoadGrouper final : public CGameObject
{
public:
	enum MOVECMD { MOVECMD_STOP, MOVECMD_ROTATE, MOVECMD_END };
	struct ROADGROUP_DESC : public GAMEOBJECT_DESC
	{
		MOVECMD eMoveCommand = { MOVECMD_STOP };
		_bool	bIsAnimModel = { false };
	};

private:
	CFinaleRoadGrouper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleRoadGrouper(const CFinaleRoadGrouper& rhs);
	virtual ~CFinaleRoadGrouper() = default;

public:

	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	vector<class CFinaleRoad*> m_pRoads;

public:
	static CFinaleRoadGrouper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END