#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLightng;
    float32_t shininess;
};

struct PixcelShaderOutput
{
    float32_t4 color : SV_Target0;
};

struct DirectrionaLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity;
};

struct Camera
{
    float32_t3 worldPosition;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectrionaLight> gDirectrionaLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

PixcelShaderOutput main(VertexShaderOutput input)
{
    PixcelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    if (gMaterial.enableLightng != 0)//Litingする場合
    {
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(gDirectrionaLight.direction, normalize(input.normal));
        
        float RdotE = dot(reflectLight, toEye);
        float speclarPow = pow(saturate(RdotE), gMaterial.shininess);
        
        float NdotL = dot(normalize(input.normal), -gDirectrionaLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
      // output.color = gMaterial.color * textureColor * gDirectrionaLight.color * cos * gDirectrionaLight.intensity;
   
        float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectrionaLight.color.rgb * cos * gDirectrionaLight.intensity;
        float32_t3 specular = gDirectrionaLight.color.rgb * gDirectrionaLight.intensity * speclarPow * float32_t3(1.0f, 1.0f, 1.0f);
        
        output.color.rgb = diffuse + specular;
        
        output.color.a = gMaterial.color.a * textureColor.a;
        
        
        }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}