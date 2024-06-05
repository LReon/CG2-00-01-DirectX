#include "Object3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

//struct VertexShaderOutput
//{
//    float32_t4 position : SV_POSITION;
//};

struct VertxShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertxShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position,gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    return output;
}



