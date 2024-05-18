

sampler ClampSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = clamp;
    AddressV = clamp;
};

// 선형 샘플러. UV좌표의 주소모드를 wrap으로 설정하여 텍스처가 경계를 넘어갈 경우 반복되어 나타난다.
sampler LinearSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = wrap;
    AddressV = wrap;
};

// 점 샘플러. 텍셀이 직접 샘플링 되어 보여진다. 선형샘플러와 유사하다.
sampler PointSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = wrap;
    AddressV = wrap;
};

// 기본 상태이다. 솔리드 형태로 채워지며 후면 추려내기가 된다. 앞면 다각형의 위상 순서를 시계 방향으로 설정한다.
RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

// 와이어 프레임 설정이다. 위와 같으나 와이어프레임으로 출력한다.
RasterizerState RS_Wireframe
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockwise = false;
};

// 컬링 안한다!
RasterizerState RS_NonCull
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false; // 필요에 따라 설정하세요
};

// 하늘 랜더링 래스터라이저 상태 설정. 다 채우며, 앞면 추려내기를 사용한다. false 를 사용한다면 시계 반대방향이 아니여도 래스터라이저가 발동한다.
RasterizerState RS_Sky
{
    FillMode = Solid;
    CullMode = Front;
    FrontCounterClockwise = false;
};

// 깊이 버퍼를 사용하여 깊이 테스트를 활성화한다. 모든 픽셀에 대해 깊이 값을 쓰도록 지정한다. (작거나 같은 경우에만 픽셀이 랜더링된다.)
DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less_equal;
};

// 스카이 박스 설정이며, 깊이 테스트를 비 활성화 하여 그린다. 카메라 뒤에있는 것들을 랜더링 할 때 사용하면 된다.
DepthStencilState DSS_Sky
{
    DepthEnable = false;
    DepthWriteMask = zero;
};

DepthStencilState DSS_NO_TEST_WRITE
{
    DepthEnable = false;
    DepthWriteMask = zero;
};

// 블랜딩을 비 활성화 하고 단일 랜더 대상에 대해 블랜딩을 사용하지 않음을 지정한다.
BlendState BS_Default
{
    BlendEnable[0] = false;
};

// 알파 블랜딩 상태이다. 이 설정은 블랜딩을 활성화 하며 알파블랜딩을 수행한다.
BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;

    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = Add;
};


BlendState BS_Blend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = one;
    DestBlend = one;
    BlendOp = Add;
};



