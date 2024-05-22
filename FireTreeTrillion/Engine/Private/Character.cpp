#include "Character.h"

CCharacter::CCharacter(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter & rhs)
	: CGameObject{ rhs }
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCharacter::Initialize(void * pArg)
{
	//if (nullptr != pArg)
	//{
	//	CHARACTER_DESC*		pCharacterDesc = (CHARACTER_DESC*)pArg;
	//}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

_int CCharacter::Tick(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CCharacter::Late_Tick(_float fTimeDelta)
{
}

HRESULT CCharacter::Render()
{
	return S_OK;
}

void CCharacter::Free()
{
	__super::Free();
}
