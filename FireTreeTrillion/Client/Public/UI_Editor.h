#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CUI_Editor : public CUIObject
{
private:
	CUI_Editor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Editor(const CUI_Editor& rhs);
	virtual ~CUI_Editor() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual void	Render_IMGUI()								override;

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	// IMGUI SET
	_bool			Edit_Transform();
	_bool			Edit_RGBAColor();
	_bool			Set_OrthoProj();

	_bool			Set_GizmoSync();
	_bool			Set_GizmoGrid();
	_bool			Set_DockSpace();
	_bool			Set_FileDialog(); //IMGUI_MANAGER 클래스 내 함수와 동일한 기능. 추후 이동 필요

	_bool			Save_FileData();
	_bool			Load_FileData();
	

public:
	static CUI_Editor*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};


END