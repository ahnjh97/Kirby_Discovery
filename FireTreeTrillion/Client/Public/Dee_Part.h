#pragma once
#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
END

BEGIN(Client)

class CDee_Part final : public CPartObject
{
public:
	struct DEEPART_DESC : public PARTOBJECT_DESC
	{
		CBone* pSocket = { nullptr };
		wstring wstrModelName = TEXT("NONE");
	};

private:
	CDee_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDee_Part(const CDee_Part& rhs);
	virtual ~CDee_Part() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CBone* m_pSocket = { nullptr };

private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

private:
	HRESULT Add_Components(wstring wstrModelName);
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

public:
	static CDee_Part* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END