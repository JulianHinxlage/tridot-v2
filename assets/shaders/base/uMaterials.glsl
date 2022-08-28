struct Material{
    vec4 color;
    int mapping;
    float roughness;
    float metallic;
    float normalMapFactor;
    float emissive;
    int texture;
    int normalMap;
    int roughnessMap;
    int metallicMap;
    int ambientOcclusionMap;
    int displacementMap;
    int align1;
    vec2 textureOffset;
    vec2 textureScale;
    vec2 normalMapOffset;
    vec2 normalMapScale;
    vec2 roughnessMapOffset;
    vec2 roughnessMapScale;
    vec2 metallicMapOffset;
    vec2 metallicMapScale;
    vec2 ambientOcclusionMapOffset;
    vec2 ambientOcclusionMapScale;
    vec2 displacementMapOffset;
    vec2 displacementMapScale;
};
layout(std140) uniform uMaterials {
    Material materials[32];
};
