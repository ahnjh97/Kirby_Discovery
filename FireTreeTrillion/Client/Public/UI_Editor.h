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
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

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

	_bool			Create_UIObject();
	_bool			Save_FileData(string _strFilePath);
	_bool			Load_FileData(const string& _strFilePath);
	

public:
	static CUI_Editor*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};


END