struct VertexInput {
    float2 position;
    float3 color;
    float2 texCoord;
};

struct VertexOutput {
    float4 sv_position : SV_Position;
    float3 color;
    float2 texCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
[[vk_binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
VertexOutput vertMain(VertexInput inp) {
    VertexOutput output;
    // output.sv_position = float4(inp.position, 0.0, 1.0);
    output.sv_position = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(inp.position, 0.0, 1.0))));
    output.color = inp.color;
    output.texCoord = inp.texCoord;
    return output;
}

[[vk_binding(1, 0)]]
Sampler2D sampler;

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    // float3 color = inVert.color;
    float4 color = sampler.Sample(inVert.texCoord);
    return color;
}


struct Particle {
    float2 position;
    float2 velocity;
    float4 color;
};

struct ParticleSSBO {
    Particle particles;
};
StructuredBuffer<ParticleSSBO> particlesIn;
RWStructuredBuffer<ParticleSSBO> particlesOut;

[shader("compute")]
[numthreads(256, 1, 1)]
void compMain(uint3 threadId: SV_DispatchThreadID)
{

}
