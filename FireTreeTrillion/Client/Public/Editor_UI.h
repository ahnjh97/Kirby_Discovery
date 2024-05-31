#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CEditor_UI : public CUIObject
{
private:
	CEditor_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditor_UI(const CEditor_UI& rhs);
	virtual ~CEditor_UI() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual void	Render_IMGUI()								override;

private:
	_bool			Window_Directories();
	_bool			Window_Properties();
	_bool			Window_Textures();

	_bool			Edit_Transform();
	_bool			Edit_RGBAColor();
	_bool			Set_OrthoProj();

	_bool			Set_GizmoSync();
	_bool			Set_GizmoGrid();
	_bool			Set_DockSpace();

	_bool			Create_UIObject();
	_bool			Delete_UIObject();
	_bool			Save_FileData(string _strFilePath);
	_bool			Load_FileData(const string& _strFilePath);
	

public:
	static CEditor_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};


END