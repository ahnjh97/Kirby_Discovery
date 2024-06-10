#pragma once
#include "GameInstance.h"

namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, 
                 LEVEL_LOGO = 2, 
                 LEVEL_GAMEPLAY, 

                //============= TOOL
                 LEVEL_TOOL_FX,
                 LEVEL_TOOL_UI,
                 LEVEL_TOOL_ANIM = 6, // ANIM툴은 6으로 고정 ! 부탁드립니댜
                 LEVEL_TOOL_MAP,
                //============ GAME
                // 여기에서 이어서 
                 LEVEL_INTRO,
                 LEVEL_STAGE1,

                 LEVEL_END };

    enum TRIGGER_TYPE { TRIGGER_CAMERA, TRIGGER_SHADER, TRIGGER_END };
    enum CAMERA_TYPE { CAM_FRONT, CAM_REAR, CAM_END };

    enum FILE_TYPE { FILE_UI, FILE_NONE };
    
    const unsigned int	g_iWinSizeX = 1600; //1280;
    const unsigned int	g_iWinSizeY = 900; //720;

    const wstring g_strLayerMonster = TEXT("Layer_Monster");
    const wstring g_strLayerItem = TEXT("Layer_Item");
    const wstring g_strLayerMapObject = TEXT("Layer_MapObject");
}

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

using namespace std;
using namespace Client;

#define ADD_GAMEOBJECT_PROTOTYPE(tag, className) \
    do { \
        wstring wstrResult = L"Prototype_GameObject_" + wstring(tag); \
        if (FAILED(m_pGameInstance->Add_Prototype(wstring(wstrResult), className::Create(m_pDevice, m_pContext)))) \
            return E_FAIL; \
    } while (0)

#define AXIS_X 1.f, 0.f, 0.f, 0.f
#define AXIS_Y 0.f, 1.f, 0.f, 0.f
#define AXIS_Z 0.f, 0.f, 1.f, 0.f

enum PASS_POSTEX {
    POSTEX_DEFAULT, POSTEX_ALPHABLEND, POSTEX_WHITE, POSTEX_BLOOM, POSTEX_END
};

enum PASS_MODEL {
    MODEL_NORMAL_O, MODEL_NORMAL_X, MODEL_SHADOW, MODEL_SKY, MODEL_BLOOM, MODEL_BLEND, MODEL_TRIGGER, 
    MODEL_DEFAULTFX, MODEL_BLENDFX, MODEL_DEFERREDINFO,
    MODEL_END
};

enum PASS_ANIMMODEL {
    ANIMMODEL_NORMAL_O, ANIMMODEL_NORMAL_X, ANIMMODEL_SHADOW, ANIMMODEL_KIRBYMOUTH, ANIMMODEL_KIRBYEYE, ANIMMODEL_BLOOM, ANIMMODEL_BLEND, ANIMMODEL_DEFERREDINFO,
    ANIMMODEL_KIRBY,
    ANIMMODEL_END
};

enum PASS_INSTANCEPOINT {
    INSTANCEPOINT_DEFAULT, INSTANCEPOINT_WHITE, INSTANCEPOINT_END
};

enum ABILITYTYPE {
    ABILITY_DEFAULT, ABILITY_SWORD, ABILITY_CUTTER, ABILITY_BOMB, ABILITY_END
};

enum VACUUMSIZE {
    SIZE_SMALL, SIZE_BIG, SIZE_END
};