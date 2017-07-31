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
    XMFLOAT4X4 mWorldViewProj;
    XMFLOAT4X4 mWorld;
};

struct LightBuffer
{
	XMFLOAT3 LightPos;
	float Lpad1;
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
ID3D11Buffer*               g_pIndexBuffer = nullptr;
ID3D11Buffer*               g_pCBChangesEveryFrame = nullptr;
ID3D11ShaderResourceView*   g_pTextureRV = nullptr;
ID3D11ShaderResourceView*   g_pDepthTexture = nullptr;
ID3D11SamplerState*         g_pSamplerLinear = nullptr;
ID3D11Texture2D*			g_depthStencilBuffer = nullptr;
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
XMVECTOR					s_Eye = { 0.0f, 3.0f, -6.0f, 0.f };
XMVECTOR					s_At = { 0.0f, 1.0f, 0.0f, 0.f };
XMVECTOR					s_Up = { 0.0f, 1.0f, 0.0f, 0.f };
D3D11_VIEWPORT				g_viewport;

void UpdateLightView()
{
	auto pos = XMLoadFloat3(&lights[0].LightPos);
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
	vertices = new SimpleVertex[16*24]();
	indices = new DWORD[16 * 36]();

	lights = new LightBuffer[3]();
	lights[0].LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].LightPos = XMFLOAT3(0.0f, 0.0f, 1.5f);
	lights[0].Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	lights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0].Constant = 1.0f;
	lights[0].Linear = 0.00009f;
	lights[0].Quadratic = 0.000003f;

	float interval = 3.0f;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			for (int k = 0; k < 24; ++k)
			{
				vertices[(i * 4 + j) * 24 + k].Pos = XMFLOAT3(cube[k].Pos.x + (i - 2) * interval, cube[k].Pos.y + (j - 2) * interval, cube[k].Pos.z);
				vertices[(i * 4 + j) * 24 + k].Tex = cube[k].Tex;
				vertices[(i * 4 + j) * 24 + k].Normal = cube[k].Normal;
			}
			for (int k = 0; k < 36; ++k)
			{
				indices[(i * 4 + j) * 36 + k] = index[k] + (i * 4 + j) * 24;
			}
		}
	}
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
		pVSBlob->GetBufferSize(), &g_pLightVertexLayout);
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
	V_RETURN(DXUTCompileFromFile(L"ShadowMappingDepth.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pLightVertexShader);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSBlob);
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC LightLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = ARRAYSIZE(LightLayout);

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout(LightLayout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pLightVertexLayout);
	SAFE_RELEASE(pVSBlob);
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
	bd.ByteWidth = sizeof(SimpleVertex) * 24 * 16;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * 36 * 16;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = indices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer));

	// Create the constant buffers
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame));

	// Initialize the world matrices
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	static const XMVECTORF32 s_Eye = { 0.0f, 3.0f, -6.0f, 0.f };
	static const XMVECTORF32 s_At = { 0.0f, 1.0f, 0.0f, 0.f };
	static const XMVECTORF32 s_Up = { 0.0f, 1.0f, 0.0f, 0.f };
	g_View = XMMatrixLookAtLH(s_Eye, s_At, s_Up);

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
	V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear));

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
	g_LightProjection = XMMatrixPerspectiveFovLH(XM_PI * 0.25f, fAspect, 0.1f, 100.0f);

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
 //   //
 //   // Clear the back buffer
 //   //
 //   auto pRTV = DXUTGetD3D11RenderTargetView();
 //   pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::MidnightBlue );

 //   //
 //   // Clear the depth stencil
 //   //
 //   auto pDSV = DXUTGetD3D11DepthStencilView();
 //   pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

 //   XMMATRIX mWorldViewProjection = g_World * g_LightView * g_LightProjection;

 //   // Update constant buffer that changes once per frame
 //   HRESULT hr;
 //   D3D11_MAPPED_SUBRESOURCE MappedResource;
 //   V( pd3dImmediateContext->Map( g_pCBChangesEveryFrame , 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
 //   auto pCB = reinterpret_cast<CBChangesEveryFrame*>( MappedResource.pData );
 //   XMStoreFloat4x4( &pCB->mWorldViewProj, XMMatrixTranspose( mWorldViewProjection ) );
 //   XMStoreFloat4x4( &pCB->mWorld, XMMatrixTranspose( g_World ) );
 //   pd3dImmediateContext->Unmap( g_pCBChangesEveryFrame , 0 );

 //   //
 //   // Render DepthTexture
 //   //
	//RenderBuffers(pd3dImmediateContext, &g_pVertexBuffer, g_pIndexBuffer);
	//pd3dImmediateContext->IASetInputLayout(g_pLightVertexLayout);
 //   pd3dImmediateContext->VSSetShader( g_pLightVertexShader, nullptr, 0 );
 //   pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBChangesEveryFrame );
 //   pd3dImmediateContext->PSSetShader( g_pLightPixelShader, nullptr, 0 );
	//pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
 //   pd3dImmediateContext->DrawIndexed( 36*16, 0, 0 );
 //
 // Clear the back buffer
 //
	auto pRTV = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->ClearRenderTargetView(pRTV, Colors::Black);

	//
	// Clear the depth stencil
	//
	auto pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	XMMATRIX mWorldViewProjection = g_World * g_View * g_Projection;

	// Update constant buffer that changes once per frame
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V(pd3dImmediateContext->Map(g_pCBChangesEveryFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pCB = reinterpret_cast<CBChangesEveryFrame*>(MappedResource.pData);
	XMStoreFloat4x4(&pCB->mWorldViewProj, XMMatrixTranspose(mWorldViewProjection));
	XMStoreFloat4x4(&pCB->mWorld, XMMatrixTranspose(g_World));
	pd3dImmediateContext->Unmap(g_pCBChangesEveryFrame, 0);

	//
	// Render the cube
	//
	RenderBuffers(pd3dImmediateContext, &g_pVertexBuffer, g_pIndexBuffer);
	pd3dImmediateContext->IASetInputLayout(g_pLightVertexLayout);
	pd3dImmediateContext->VSSetShader(g_pLightVertexShader, nullptr, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShader(g_pLightPixelShader, nullptr, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangesEveryFrame);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	pd3dImmediateContext->DrawIndexed(36*16, 0, 0);
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
    SAFE_RELEASE( g_pIndexBuffer );
    SAFE_RELEASE( g_pVertexLayout );
    SAFE_RELEASE( g_pTextureRV );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pCBChangesEveryFrame );
    SAFE_RELEASE( g_pSamplerLinear );
	SAFE_RELEASE(g_pDepthTexture);
	SAFE_RELEASE(g_pLightVertexShader);
	SAFE_RELEASE(g_pLightPixelShader);
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
    DXUTCreateWindow( L"Tutorial08" );

    // Only require 10-level hardware or later
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 800, 600 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}
