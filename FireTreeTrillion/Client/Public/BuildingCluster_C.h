#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBuildingCluster_C final : public CGameObject
{
private:
	CBuildingCluster_C
(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBuildingCluster_C
(const CBuildingCluster_C
& rhs);
	virtual ~CBuildingCluster_C
() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;


private:
	HRESULT			Add_Components();
	enum CUT { CUT1, CUT11, CUT12, CUT13, CUT14, CUT15, CUT16, CUT17, CUT18, CUT19, 
		CUT2, CUT20, CUT3, CUT5, CUT6, CUT7, CUT8, CUT9, CUT_END };
	CUT				m_eCurCut = { CUT_END };
	CUT				m_ePreCut = { CUT_END };
	_bool			m_bRender = { false };
	void			Set_Animation();

private:
	CModel* m_pBuildingCluster = { nullptr };

	// 배열로 들고 있는다.
	enum BUILDING { A1, A2, A3, A4, A5, B1, C1, C2, C3, C4, D1, D2, E1, E2, E3, F1, F2, G1, G2, G3, H1, H2, H3, Z };
	CShader* m_pShaderModelCom = { nullptr };
	CModel* m_pModelCom[Z];

	_float4x4		m_BuildingMatrix[Z];
	HRESULT			Rendering_Building(BUILDING eType);
	void			Update_BuildingMatrix();

public:
	static CBuildingCluster_C
* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END