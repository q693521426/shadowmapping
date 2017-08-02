//--------------------------------------------------------------------------------------
// File: Tutorial08.cpp
//
// Basic introduction to DXUT
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "SDKmisc.h"

#pragma warning( disable : 4100 )

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};

struct CBChangesEveryFrame
{
    XMFLOAT4X4 mWorld;
    XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT4X4 lightSpaceMatrix;
	XMFLOAT4 viewPos;
	float NearZ;
	float FarZ;
	XMFLOAT2 padding;
};

struct LightBuffer
{
	XMFLOAT4 LightPos;
	XMFLOAT4 LightColor;

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	float Constant;
	float Linear;
	float Quadratic;

	float Lpad2;
};

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
const int					screen_width = 800;
const int					screen_height = 600;
ID3D11VertexShader*         g_pVertexShader = nullptr;
ID3D11VertexShader*         g_pLightVertexShader = nullptr;
ID3D11PixelShader*          g_pPixelShader = nullptr;
ID3D11PixelShader*          g_pLightPixelShader = nullptr;
ID3D11InputLayout*          g_pVertexLayout = nullptr;
ID3D11InputLayout*			g_pLightVertexLayout = nullptr;
ID3D11Buffer*               g_pVertexBuffer = nullptr;
ID3D11Buffer*               g_pFloorVertexBuffer = nullptr;
ID3D11Buffer*               g_pIndexBuffer = nullptr;
ID3D11Buffer*               g_pFloorIndexBuffer = nullptr;
ID3D11Buffer*               g_pCBChangesEveryFrame = nullptr;
ID3D11Buffer*               g_pCBLight = nullptr;
ID3D11ShaderResourceView*   g_pTextureRV = nullptr;
ID3D11ShaderResourceView*   g_pDepthTextureRV = nullptr;
ID3D11SamplerState*         g_pSamplerWrap = nullptr;
ID3D11SamplerState*         g_pSamplerClamp = nullptr;
ID3D11Texture2D*			g_depthMapping = nullptr;
ID3D11Texture2D*			g_depthStencilBuffer = nullptr;
ID3D11RenderTargetView*		g_depthRenderTargetView = nullptr;
ID3D11DepthStencilView*		g_depthStencilView = nullptr;
ID3D11DepthStencilState*	g_depthStencilState = nullptr;
ID3D11DepthStencilState*	g_depthDisabledStencilState = nullptr;
ID3D11RasterizerState*		g_rasterState = nullptr;
ID3D11RasterizerState*		g_rasterStateNoCulling = nullptr;
ID3D11BlendState*			g_alphaEnableBlendingState = nullptr;
ID3D11BlendState*			g_alphaDisableBlendingState = nullptr;
XMMATRIX                    g_World;
XMMATRIX                    g_View;
XMMATRIX                    g_Projection;
XMMATRIX                    g_LightView;
XMMATRIX                    g_LightProjection;
SimpleVertex*				vertices;
DWORD*						indices;
LightBuffer*				lights;
XMVECTOR					s_Eye = { 0.0f, -15.0f, -20.0f, 0.f };
XMVECTOR					s_At = { 0.0f, 1.0f, 0.0f, 0.f };
XMVECTOR					s_Up = { 0.0f, 1.0f, 0.0f, 0.f };
D3D11_VIEWPORT				g_viewport;

SimpleVertex floorVextices[]=
{
	{ XMFLOAT3(-10.0f, -10.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f,0.0f,-1.0f)},
	{ XMFLOAT3(10.0f, -10.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f,0.0f,-1.0f)},
	{ XMFLOAT3(10.0f, 10.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,0.0f,-1.0f)},
	{ XMFLOAT3(-10.0f, 10.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,0.0f,-1.0f)},
};

DWORD floorIndex[] =
{
	2,0,3,
	1,0,2
};

void UpdateLightView()
{
	auto pos = XMLoadFloat4(&lights[0].LightPos);
	g_LightView = XMMatrixLookAtLH(pos, XMVectorSet(0.0f, 0.0f, 0.0f, 0.f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.f));
}

void Initialize()
{
	// Create vertex buffer
	SimpleVertex cube[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f,1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f,1.0f,0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f,-1.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f,-1.0f,0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT2(1.0f, 1.0f),XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f),XMFLOAT3(-1.0f,0.0f,0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),XMFLOAT3(-1.0f,0.0f,0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f),	XMFLOAT2(1.0f, 1.0f),XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f),XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f),XMFLOAT3(1.0f,0.0f,0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),XMFLOAT3(1.0f,0.0f,0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),	XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,0.0f,-1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,0.0f,-1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f),XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),	XMFLOAT2(0.0f, 1.0f),XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),	XMFLOAT2(0.0f, 0.0f),XMFLOAT3(0.0f,0.0f,1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),	XMFLOAT2(1.0f, 0.0f),XMFLOAT3(0.0f,0.0f,1.0f) },
	};

	// Create index buffer
	DWORD index[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};
	vertices = new SimpleVertex[16*24 + 4]();
	indices = new DWORD[16 * 36 + 6]();

	lights = new LightBuffer[3]();
	lights[0].LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].LightPos = XMFLOAT4(0.0f, -2.0f, -10.0f, 1.0f);
	lights[0].Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	lights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].Constant = 1.0f;
	lights[0].Linear = 0.009f;
	lights[0].Quadratic = 0.0003f;

	float interval = 3.0f;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			for (int k = 0; k < 24; ++k)
			{
				vertices[(i * 4 + j) * 24 + k].Pos = XMFLOAT3(cube[k].Pos.x + (i - 1.5) * interval, cube[k].Pos.y + (j - 1.5) * interval, cube[k].Pos.z);
				vertices[(i * 4 + j) * 24 + k].Tex = cube[k].Tex;
				vertices[(i * 4 + j) * 24 + k].Normal = cube[k].Normal;
			}
			for (int k = 0; k < 36; ++k)
			{
				indices[(i * 4 + j) * 36 + k] = index[k] + (i * 4 + j) * 24;
			}
		}
	}
	for (int i = 0; i < 4; ++i)
		vertices[16 * 24 + i] = floorVextices[i];
	for (int i = 0; i<6; ++i)
		indices[16 * 36 + i] = floorIndex[i] + 16 * 24;

}

void RenderBuffers(ID3D11DeviceContext* deviceContext, ID3D11Buffer** m_vertexBuffer, ID3D11Buffer* m_indexBuffer)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(SimpleVertex);
	offset = 0;
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
	HRESULT hr = S_OK;

	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"Model.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	SAFE_RELEASE(pVSBlob);
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"Model.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	ID3DBlob* pLightVSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"ShadowMappingDepth.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pLightVSBlob));

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pLightVSBlob->GetBufferPointer(), pLightVSBlob->GetBufferSize(), nullptr, &g_pLightVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pLightVSBlob);
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC LightLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	numElements = ARRAYSIZE(LightLayout);

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout(LightLayout, numElements, pLightVSBlob->GetBufferPointer(),
		pLightVSBlob->GetBufferSize(), &g_pLightVertexLayout);
	SAFE_RELEASE(pLightVSBlob);
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	ID3DBlob* pLightPSBlob = nullptr;
	V_RETURN(DXUTCompileFromFile(L"ShadowMappingDepth.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pLightPSBlob));

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader(pLightPSBlob->GetBufferPointer(), pLightPSBlob->GetBufferSize(), nullptr, &g_pLightPixelShader);
	SAFE_RELEASE(pLightPSBlob);
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * (24 * 16 + 4);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer));

	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	InitData.pSysMem = floorVextices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pFloorVertexBuffer));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * (36 * 16 + 6);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = indices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer));

	bd.ByteWidth = sizeof(DWORD) * 6;
	InitData.pSysMem = floorIndex;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pFloorIndexBuffer));

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame));

	bd.ByteWidth = sizeof(LightBuffer);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBLight));

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	g_View = XMMatrixLookAtLH(s_Eye, s_At, s_Up);
	UpdateLightView();
	// Load the Texture
	V_RETURN(DXUTCreateShaderResourceViewFromFile(pd3dDevice, L"misc\\seafloor.dds", &g_pTextureRV));

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerWrap));

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerClamp));

	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	// Setup the render target texture description.
	depthDesc.Width = screen_width;
	depthDesc.Height = screen_height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	V_RETURN(pd3dDevice->CreateTexture2D(&depthDesc, NULL, &g_depthMapping));

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = depthDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	V_RETURN(pd3dDevice->CreateRenderTargetView(g_depthMapping, &renderTargetViewDesc, &g_depthRenderTargetView));


	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = depthDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(g_depthMapping, &shaderResourceViewDesc, &g_pDepthTextureRV));

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    // Setup the projection parameters
    float fAspect = static_cast<float>( pBackBufferSurfaceDesc->Width ) / static_cast<float>( pBackBufferSurfaceDesc->Height );
    g_Projection = XMMatrixPerspectiveFovLH( XM_PI * 0.25f, fAspect, 0.1f, 100.0f );
	g_LightProjection = XMMatrixPerspectiveFovLH(XM_PI * 2/3, fAspect, 0.1f, 100.0f);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Rotate cube around the origin
    //g_World = XMMatrixRotationY( 60.0f * XMConvertToRadians((float)fTime) );

    //// Modify the color
    //g_vMeshColor.x = ( sinf( ( float )fTime * 1.0f ) + 1.0f ) * 0.5f;
    //g_vMeshColor.y = ( cosf( ( float )fTime * 3.0f ) + 1.0f ) * 0.5f;
    //g_vMeshColor.z = ( sinf( ( float )fTime * 5.0f ) + 1.0f ) * 0.5f;

}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
	auto pRTV = DXUTGetD3D11RenderTargetView();
	auto pDSV = DXUTGetD3D11DepthStencilView();
	//
	// Shadow Mapping
	//
	pd3dImmediateContext->OMSetRenderTargets(1, &g_depthRenderTargetView, pDSV);
	pd3dImmediateContext->ClearRenderTargetView(g_depthRenderTargetView, Colors::Black);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V(pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
	XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(g_World));
	XMStoreFloat4x4(&pCB->mView, XMMatrixTranspose(g_LightView));
	XMStoreFloat4x4(&pCB->mProj, XMMatrixTranspose(g_LightProjection));
	pCB->viewPos = lights[0].LightPos;
	pCB->NearZ = 0.1f;
	pCB->FarZ = 100.0f;
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

	pd3dImmediateContext->IASetInputLayout(g_pLightVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pLightVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pLightPixelShader, nullptr, 0);

	////floor
	//RenderBuffers(pd3dImmediateContext, &g_pFloorVertexBuffer, g_pFloorIndexBuffer);
	//pd3dImmediateContext->DrawIndexed(6, 0, 0);

	//cubes
	RenderBuffers(pd3dImmediateContext, &g_pVertexBuffer, g_pIndexBuffer);
	pd3dImmediateContext->DrawIndexed(36 * 16 + 6, 0, 0);

	//
	// Render
	//
	pd3dImmediateContext->OMSetRenderTargets(1, &pRTV, pDSV);
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::MidnightBlue);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	XMMATRIX lightSpaceMatrix = XMMatrixMultiply(g_LightView, g_LightProjection);
	V(pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
	XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(g_World));
	XMStoreFloat4x4(&pCB->mView, XMMatrixTranspose(g_View));
	XMStoreFloat4x4(&pCB->mProj, XMMatrixTranspose(g_Projection));
	XMStoreFloat4x4(&pCB->lightSpaceMatrix, XMMatrixTranspose(lightSpaceMatrix));
	XMStoreFloat4(&pCB->viewPos,s_Eye);
	pCB->NearZ = 0.1f;
	pCB->FarZ = 100.0f;
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

	V(pd3dImmediateContext->Map(g_pCBLight, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pLB = reinterpret_cast<LightBuffer*>(MappedResource.pData);
	*pLB = lights[0];
	pd3dImmediateContext->Unmap(g_pCBLight, 0);

	pd3dImmediateContext->IASetInputLayout(g_pVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetConstantBuffers(1, 1, &g_pCBLight);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	pd3dImmediateContext->PSSetShaderResources(1, 1, &g_pDepthTextureRV);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamplerWrap);
	pd3dImmediateContext->PSSetSamplers(1, 1, &g_pSamplerClamp);

	//floor
	RenderBuffers(pd3dImmediateContext, &g_pFloorVertexBuffer, g_pFloorIndexBuffer);
	pd3dImmediateContext->DrawIndexed(6, 0, 0);
	
	//cubes
	RenderBuffers(pd3dImmediateContext, &g_pVertexBuffer, g_pIndexBuffer);
	pd3dImmediateContext->DrawIndexed(36 * 16, 0, 0);

}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    SAFE_RELEASE( g_pVertexBuffer );
	SAFE_RELEASE(g_pFloorVertexBuffer);
    SAFE_RELEASE( g_pIndexBuffer );
	SAFE_RELEASE(g_pFloorIndexBuffer);
    SAFE_RELEASE( g_pVertexLayout );
    SAFE_RELEASE( g_pTextureRV );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pCBChangesEveryFrame );
    SAFE_RELEASE( g_pSamplerWrap );
	SAFE_RELEASE(g_pSamplerClamp);
	SAFE_RELEASE( g_pDepthTextureRV);
	SAFE_RELEASE( g_pLightVertexShader);
	SAFE_RELEASE( g_pLightPixelShader);

	SAFE_RELEASE(g_pLightVertexLayout);
	SAFE_RELEASE(g_depthMapping);
	SAFE_RELEASE(g_depthStencilBuffer);
	SAFE_RELEASE(g_depthRenderTargetView);
	SAFE_RELEASE(g_depthStencilView);
	SAFE_RELEASE(g_depthStencilState);
	SAFE_RELEASE(g_depthDisabledStencilState);
	SAFE_RELEASE(g_rasterState);
	SAFE_RELEASE(g_rasterStateNoCulling);
	SAFE_RELEASE(g_alphaEnableBlendingState);
	SAFE_RELEASE(g_alphaDisableBlendingState);
	SAFE_RELEASE(g_pCBLight);
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: // Change as needed                
                break;
			case 'W':
			{
				XMVECTOR s = XMVectorReplicate(0.05);
				XMVECTOR Pos = XMLoadFloat4(&lights[0].LightPos);
				XMVECTOR Look =  XMVectorSet(0.0f,0.0f,0.0f,1.0f) - Pos;
				XMStoreFloat4(&lights[0].LightPos, XMVectorMultiplyAdd(s, Look, Pos));

				UpdateLightView();
			}break;
			case 'S':
			{
				XMVECTOR s = XMVectorReplicate(-0.05);
				XMVECTOR Pos = XMLoadFloat4(&lights[0].LightPos);
				XMVECTOR Look = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f) - Pos;
				XMStoreFloat4(&lights[0].LightPos, XMVectorMultiplyAdd(s, Look, Pos));

				UpdateLightView();
			}break;
			case 'A':
			{
				XMMATRIX Rotate = XMMatrixRotationZ(0.05f);
				XMVECTOR Pos = XMLoadFloat4(&lights[0].LightPos);
				Pos = XMVector3TransformNormal(Pos, Rotate);
				XMStoreFloat4(&lights[0].LightPos, Pos);

				UpdateLightView();
			}break;
			case 'D':
			{
				XMMATRIX Rotate = XMMatrixRotationZ(-0.05f);
				XMVECTOR Pos = XMLoadFloat4(&lights[0].LightPos);
				Pos = XMVector3TransformNormal(Pos, Rotate);
				XMStoreFloat4(&lights[0].LightPos, Pos);

				UpdateLightView();
			}break;
			case 'Q':
			{
				XMVECTOR s = XMVectorReplicate(0.5);

				s_Eye = XMVectorMultiplyAdd(s, s_Up, s_Eye);

				UpdateLightView();
			}break;
			case 'E':
			{
				XMVECTOR s = XMVectorReplicate(-0.5);

				s_Eye = XMVectorMultiplyAdd(s, s_Up, s_Eye);

				UpdateLightView();
			}break;
        }

    }
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below

	Initialize();
    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here

    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"ShadowMapping" );

    // Only require 10-level hardware or later
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 800, 600 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}
