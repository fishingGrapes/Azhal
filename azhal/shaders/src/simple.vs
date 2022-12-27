struct VS_OUTPUT 
{
  float4 Position : SV_POSITION;
 
  [[vk::location(0)]]
  float3 Color : COLOR;
};

static const float2 K_POSITIONS[3] = 
{
  float2(0.0, -0.5), 
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
};

static const float3 K_COLORS[3] = 
{
  float3(1.0, 0.0, 0.0), 
  float3(0.0, 1.0, 0.0),
  float3(0.0, 0.0, 1.0)
};

VS_OUTPUT main( uint VertexIndex : SV_VERTEXID )
{
  VS_OUTPUT output = (VS_OUTPUT) 0;
 
  output.Position = float4( K_POSITIONS[VertexIndex], 0.0, 0.0 );
  output.Color = K_COLORS[VertexIndex];
  
  return output;
}