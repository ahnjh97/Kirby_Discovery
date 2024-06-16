#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END

BEGIN(Client)

// 미리 15개를 생성하여, 폭탄일때만 활성화한다.
// 모든 연산은 커비가 관리하고, Orbit은 경로 위치에서 
class CBombOrbitGlow final :
    public CGameObject
{
private:
	CBombOrbitGlow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBombOrbitGlow(const CBombOrbitGlow& rhs);
	virtual ~CBombOrbitGlow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	// 궤적의 루트를 그려줌. 포지션 업데이트
	void	Update_GlowPosition(_float4 vPos, _float4 vOriginPos);
	// Render를 할지말지 결정한다.

	_bool	RayCast_Terrain(const _float3 vMoveDir, _Inout_ _float4& vRayCastPos, _Inout_ _float4& vTerrainNormal);
	_float4		m_vOriginPos = { 0.f, 0.f, 0.f, 0.f };

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void Billboard(_float fTimeDelta);

public:
	static CBombOrbitGlow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END