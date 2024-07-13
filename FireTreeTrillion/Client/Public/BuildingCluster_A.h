#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBuildingCluster_A final : public CGameObject
{
private:
	CBuildingCluster_A(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBuildingCluster_A(const CBuildingCluster_A& rhs);
	virtual ~CBuildingCluster_A() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

private:
	HRESULT			Add_Components();
	enum CUT		{ CUT1, CUT2, CUT_END};
	void			Set_Animation();

private:
	CShader* m_pShaderAnimModelCom = { nullptr };
	CModel* m_pBuildingCluster = { nullptr };

	// 배열로 들고 있는다.
	enum BUILDING { A1, A2, B1, C1, C2, C3, C4, D1, D2, Z};
	CShader* m_pShaderModelCom = { nullptr };
	CModel* m_pModelCom[Z];

	_float4x4		m_BuildingMatrix[Z];
	HRESULT			Rendering_Building(BUILDING eType);
	void			Update_BuildingMatrix();

public:
	static CBuildingCluster_A* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END