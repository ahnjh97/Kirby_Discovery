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
                 LEVEL_INTRO,
                 LEVEL_RACING,
                 LEVEL_DEEDEEDEE,
                 LEVEL_TOWN,
                 LEVEL_PARTTIME,
                 LEVEL_FINALBOSS, // 리소스명 LAB_DISCOVERA

                 LEVEL_END };

    enum TRIGGER_TYPE { TRIGGER_CAMERA, TRIGGER_SHADER, TRIGGER_STAR, TRIGGER_LEVELCHANGER, TRIGGER_END };
    enum CAMERA_TYPE  { CAM_FRONT, CAM_REAR, CAM_END };

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
    POSTEX_DEFAULT, POSTEX_SOLIDBLEND, POSTEX_BLENDFX, POSTEX_BLOOM, POSTEX_DEFAULTFX, POSTEX_SOLIDBLEND_NOZTEST,
    POSTEX_WHITEFX, POSTEX_UI_MASK, POSTEX_UI_MASK2, POSTEX_SOFTFX, POSTEX_SOFTALPHAFX, POSTEX_UIWHITEALPHA, POSTEX_ALPHABLEND_NOTEST, POSTEX_END
};

enum PASS_MODEL {
    MODEL_NORMAL_O, MODEL_NORMAL_X, MODEL_SHADOW, MODEL_SKY, MODEL_BLOOM, MODEL_BLEND, MODEL_TRIGGER, 
    MODEL_DEFAULTFX, MODEL_BLENDFX, MODEL_DEFERREDINFO, MODEL_WHITEFX, MODEL_KIRBYPART, MODEL_NEARCLIP,
    MODEL_ALPHABLEND, MODEL_MONSTERPARTOBJECT,
    MODEL_END
};

enum PASS_ANIMMODEL {
    ANIMMODEL_NORMAL_O, ANIMMODEL_NORMAL_X, ANIMMODEL_SHADOW, ANIMMODEL_MOUTH, ANIMMODEL_EYE, ANIMMODEL_BLOOM, ANIMMODEL_BLEND, ANIMMODEL_DEFERREDINFO,
    ANIMMODEL_KIRBY, ANIMMODEL_KIRBYMOUTH, ANIMMODEL_KIRBYEYE, ANIMMODEL_NEARCLIP, ANIMMODEL_LINEAR_NORMAL_O,
    ANIMMODEL_END
};

enum PASS_INSTANCEPOINT {
    INSTANCEPOINT_DEFAULT, INSTANCEPOINT_DEFAULTFX, INSTANCEPOINT_WHITEFX, INSTANCEPOINT_BLENDFX, INSTANCEPOINT_END
};

enum ABILITYTYPE {
    ABILITY_DEFAULT, ABILITY_SWORD, ABILITY_CUTTER, ABILITY_BOMB, ABILITY_END
};

enum VACUUMSIZE {
    SIZE_SMALL, SIZE_BIG, SIZE_END
};

enum PHYXOBJECT_CURSTATE {
    PO_NORMAL, PO_VACUUMING, PO_KIRBYMOUTH, PO_FLYAWAY, PO_FLYDEADAWAY, PO_PRESSED, PO_END
};

enum COLLISION_TYPE
{
    PLAYER, 
    PLAYERBULLET,
    MONSTER,
    MONSTERBULLET,
    BOSS_DEEDEEDEE,
    OBJECT, // 돌멩이 등 사물이다.
    ITEM, // 일반 먹는 아이템이다.
    ABILITYITEM, // 흡수해야 먹을 수 있는 능력 아이템이다.
    DEFORMOBJECT, // 머금기 오브젝트들
    NPC, // 마을에 있는 NPC
    DEE, // 마을에 있는 일반 디
    BATTLEDEE, //디디디랑싸우는디
    HITBOX_PLYAER,
    HITBOX_MONSTER,
    HITBOX_DEEDEEDEE,
    ANIMDECO, // 애니메이션 있는 풀, 상자 등등

    COLLISION_END
};
