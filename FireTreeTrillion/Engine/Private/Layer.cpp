#include "..\Public\Layer.h"
#include "GameObject.h"

CLayer::CLayer()
{
}

const CComponent * CLayer::Get_Component(const wstring & strComTag, _uint iIndex)
{
	auto	iter = m_GameObjects.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;
	

	return (*iter)->Get_Component(strComTag);
}

HRESULT CLayer::Initialize()
{
	return S_OK;
}

HRESULT CLayer::Add_GameObject(CGameObject * pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);	
	/*m_GameObjects.sort([](CGameObject* lhs, CGameObject* rhs)
		{
			return lhs->Get_RenderPriority() < rhs->Get_RenderPriority();
		});*/
	return S_OK;
}

CGameObject* CLayer::Get_GameObject(_uint iIndex)
{
	auto	iter = m_GameObjects.begin();
	advance(iter, iIndex);
	/*for (size_t i = 0; i < iIndex; i++)
		++iter;*/

	return *iter;
}

CGameObject* CLayer::Get_GameObject()
{
	for (auto& obj : m_GameObjects)
	{
		if (nullptr == obj)
			continue;;

		if (true == obj->Get_Dead())
			continue;

		return obj;
	}

	return nullptr;
}

CGameObject* CLayer::Get_GameObject_ByTag(const wstring& _tag)
{
	for (auto& obj : m_GameObjects)
	{
		if (obj->Get_PrototypeTag() == _tag)
			return obj;
	}
	return nullptr;
}

CGameObject* CLayer::Get_LastGameObject()
{
	return m_GameObjects.back();
}

void CLayer::Tick(_float fTimeDelta)
{
	for (auto iter = m_GameObjects.begin(); iter != m_GameObjects.end();)
	{
		_int iObjState = { OBJ_NOEVENT };
		if (nullptr != *iter)
			iObjState = (*iter)->Tick(fTimeDelta);

		if (OBJ_DEAD == iObjState)
		{
			Safe_Release(*iter);
			iter = m_GameObjects.erase(iter);
		}
		else
			iter++;
	}
}

void CLayer::Late_Tick(_float fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Late_Tick(fTimeDelta);
	}
}

CLayer * CLayer::Create()
{
	CLayer*		pInstance = new CLayer();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLayer::Free()
{
	for (auto& pGameObject : m_GameObjects)
		Safe_Release(pGameObject);

	m_GameObjects.clear();
}
