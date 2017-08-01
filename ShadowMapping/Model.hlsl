Texture2D txDiffuse : register( t0 );
Texture2D shadowMap : register(t1);
SamplerState samWrap : register(s0);
SamplerState samClamp : register(s1);

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix World;
    matrix View;
    matrix Proj;
    matrix lightSpaceMatrix;
    float4 viewPos;

    float NearZ;
    float FarZ;

    float2 padding;
};

cbuffer LightBuffer : register(b1)
{
    float4 LightPos;
    float4 LightColor;

    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float Constant;
    float Linear;
    float Quadratic;

    float Lpad2;
};
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float4 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float4 Normal : NORMAL;
    float4 PosLightSpace : POSITION0;
    float4 PosWorldSpace : POSITION1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.PosWorldSpace = mul(input.Pos, World);
    output.Pos = mul(output.PosWorldSpace, View);
    output.Pos = mul(output.Pos, Proj);
    output.Tex = input.Tex;
    output.Normal = mul(input.Normal,World);
    output.PosLightSpace = mul(output.PosWorldSpace, lightSpaceMatrix);

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float ShadowCalculation(float4 PosLightSpace, float3 Normal, float3 lightDir);

float4 PS( PS_INPUT input) : SV_Target
{
    float3 Pos = input.PosWorldSpace.xyz / input.PosWorldSpace.w;
    float3 lightDir = LightPos.xyz - Pos;
    float3 normal = normalize(input.Normal.xyz);
    if (normal.x == 0 && normal.y == 0 && normal.z == 0)
    {
        return txDiffuse.Sample(samWrap, input.Tex);
    }

    float3 viewDir = normalize(viewPos.xyz - Pos);

    float distance = length(lightDir);
    float attenuation = 1.0f / (Constant + Linear * distance + Quadratic * distance * distance);
    
    lightDir = normalize(LightPos.xyz - input.PosWorldSpace.xyz / input.PosWorldSpace.w);
    float shadow = ShadowCalculation(input.PosLightSpace, normal, lightDir);

    float diff = max(dot(normal, lightDir), 0.0);
    
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfDir, normal), 0.0), 30);
    
    return (Ambient + (1.0 - shadow) * (diff * Diffuse + spec * Specular)) * attenuation * LightColor * txDiffuse.Sample(samWrap, input.Tex);
}

float ShadowCalculation(float4 PosLightSpace,float3 normal,float3 lightDir)
{
    float3 projCoords = PosLightSpace.xyz / PosLightSpace.w;
    projCoords = (projCoords.x * 0.5 + 0.5, projCoords.y * 0.5 + 0.5, projCoords.z);
    float closestDepth = shadowMap.Sample(samClamp,projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.01);
    float shadow = 0.0;

    //shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    //PCF
    float w, h;
    shadowMap.GetDimensions(w, h);
    float2 texelSize = 1.0 / float2(w,h);
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = shadowMap.Sample(samClamp,projCoords.xy + float2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;
    
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}