#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CKirbyWeapons final : public CPartObject
{
public:
	typedef struct : public CPartObject::PARTOBJECT_DESC
	{
		_float4x4* pBoneMatrix = { nullptr };
		ABILITYTYPE* pAbilityType = { nullptr };
		_float* pWhite = { nullptr };
		_float* pOverPower = { nullptr };
	}KIRBYWEAPON_DESC;

	enum ANIM_TYPE {
		GIANTSWING = 3, HAMMERATTACKFINAL = 5, HAMMERATTACKHIT = 6,
		ONIGOROSIHAMMERCHARGE = 9, ONIGOROSIHAMMEREND = 10,
		WHEELHAMMER = 22, ANIM_END
	};
private:
	CKirbyWeapons(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirbyWeapons(const CKirbyWeapons& rhs);
	virtual ~CKirbyWeapons() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual HRESULT	Render_DeferredInfo() override;

	HRESULT Render_NonAnimWeapon();
	HRESULT Render_AnimWeapon();

	void	Change_Animation(class CKirby* pKirby);

private:
	// 랜더가 되지 않아야 하는 것들
	_bool Block_Render() {
		return *m_pAbilityType == ABILITY_END || *m_pAbilityType == ABILITY_DEFAULT || *m_pAbilityType == ABILITY_BOMB;
	}

	CModel* m_pModelCom[ABILITY_END] = {nullptr};
	CShader* m_pShaderCom = { nullptr };
	CShader* m_pAnimShaderCom = { nullptr };

	_bool	m_isAnim = { false };
	ANIM_TYPE m_eAnimType = { ANIM_END };
	ANIM_TYPE m_ePreAnimType = { ANIM_END };
	_float	m_fTimeDelta = { 0.f };


private:
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float4x4*		m_pBoneMatrix = { nullptr };
	ABILITYTYPE*	m_pAbilityType = { nullptr };

	_float*			m_pWhiteColorDiffuse = { nullptr };
	_float*			m_fOverPowerColor = { nullptr };


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShaderResources_For_Anim();
	void	Compute_MotionBlur();

public:
	static CKirbyWeapons* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
