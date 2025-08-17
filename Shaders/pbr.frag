#version 450

layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 FragPos;
layout(location = 2) in vec3 WorldNormal;
layout(location = 3) in vec3 CamPos;
layout(location = 4) in mat3 TBN;

layout(set = 1, binding = 0) uniform sampler2DArray albedoSampler;
layout(set = 1, binding = 1) uniform sampler2DArray normalSampler;
layout(set = 1, binding = 3) uniform sampler2DArray metallicSampler;
layout(set = 1, binding = 4) uniform sampler2DArray roughnessSampler;
layout(set = 1, binding = 5) uniform sampler2DArray aoSampler;
layout(set = 1, binding = 6) uniform sampler2DArray emissiveSampler;

layout(location = 0) out vec4 outColour;

void main()
{
    //todo: implement cook-torrance pbr brdf with these - this will probably be done in VulkanPBR repo (https://github.com/Kahoneki/VulkanPBR)
    float metallic = texture(metallicSampler, vec3(TexCoord, 0)).g;
    float roughness = texture(roughnessSampler, vec3(TexCoord, 0)).b;
    
    vec3 lightPos = vec3(1,-1,1);
    vec3 lightColour = vec3(1.0,0.6,1.0);
    float lightIntensity = 0.05;
    
    vec3 albedo = texture(albedoSampler, vec3(TexCoord, 0)).rgb;
    vec3 tangentNormal = texture(normalSampler, vec3(TexCoord, 0)).rgb * 2.0 - 1.0;
    vec3 worldNormal = normalize(TBN * tangentNormal);
    
    //Ambient
    float ambientStrength = 0.005;
    float ao = texture(aoSampler, vec3(TexCoord, 0)).r;
    vec3 ambient = (ambientStrength * lightColour * lightIntensity) * ao;
    
    //Diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diffusePow = max(dot(worldNormal, lightDir), 0.0);
    vec3 diffuse = diffusePow * lightColour * lightIntensity;
    
    //Blinn-Phong specular
    vec3 viewDir = normalize(CamPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specPow = pow(max(dot(halfwayDir, worldNormal), 0.0), 128);
    vec3 specular = 16 * specPow * lightColour * lightIntensity;
    
    //Emissive
    vec3 emissive = 4 * texture(emissiveSampler, vec3(TexCoord, 0)).rgb;
    
    vec3 finalColour = ((ambient + diffuse + specular) * albedo) + emissive;
    
    outColour = vec4(finalColour, 1.0);
}