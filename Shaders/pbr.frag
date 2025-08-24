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

const float PI = 3.141592653589;



float DistributionGGX(vec3 _normal, vec3 _halfway, float _roughness)
{
    float a = _roughness*_roughness;
    float a2 = a*a;
    float cosWeighting = max(dot(_normal, _halfway), 0.0);
    float cosWeighting2 = cosWeighting * cosWeighting;

    float numerator = a2;
    float denominator = (cosWeighting2 * (a2 - 1.0) + 1.0);
    denominator = max(PI * denominator * denominator, 0.001);

    return numerator / denominator;
}



float GeometrySchlickGGX(float _cosWeighting, float _roughness)
{
    float r = _roughness + 1.0;
    float k = (r*r) / 8.0;

    float numerator = _cosWeighting;
    float denominator = _cosWeighting * (1.0 - k) + k;

    return numerator / denominator;
}



float GeometrySmith(vec3 _normal, vec3 _viewDirection, vec3 _incomingLightDirection, float _roughness)
{
    float viewCosWeighting = max(dot(_normal, _viewDirection), 0.0);
    float lightCosWeighting = max(dot(_normal, -_incomingLightDirection), 0.0);
    float ggx2 = GeometrySchlickGGX(viewCosWeighting, _roughness);
    float ggx1 = GeometrySchlickGGX(lightCosWeighting, _roughness);
    return ggx1 * ggx2;
}



vec3 FresnelSchlick(float _cosTheta, vec3 _F0)
{
    return _F0 + (1.0 - _F0) * pow(clamp(1.0 - _cosTheta, 0.0, 1.0), 5.0);
}



void main()
{
    vec3 albedo = texture(albedoSampler, vec3(TexCoord, 0)).rgb;
    vec3 normal = normalize(TBN * (texture(normalSampler, vec3(TexCoord, 0)).xyz * 2.0 - 1.0));
    float metallic = texture(metallicSampler, vec3(TexCoord, 0)).g;
    float roughness = texture(roughnessSampler, vec3(TexCoord, 0)).b;
    float ao = texture(aoSampler, vec3(TexCoord, 0)).r;
    vec3 emissive = texture(emissiveSampler, vec3(TexCoord, 0)).rgb;

    //Base reflectance
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);


    //Calculate incoming radiance
    vec3 lightPos = vec3(2,-2,2);
//    vec3 radiantFlux = vec3(23.47, 21.31, 20.79);
    vec3 radiantFlux = vec3(10.0, 2.0, 10.0);
    float distance2 = dot(FragPos - lightPos, FragPos - lightPos);
    float attenuation = 1.0 / distance2;
    vec3 incomingRadiance = radiantFlux * attenuation;


    //Calculate outgoing radiance
    vec3 incomingDirection = normalize(FragPos - lightPos);
    vec3 viewDir = normalize(CamPos - FragPos);
    vec3 halfwayDir = normalize(-incomingDirection + viewDir);

    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfwayDir, roughness);
    float G = GeometrySmith(normal, viewDir, incomingDirection, roughness);
    vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

    vec3 specularStrength = F;
    vec3 diffuseStrength = vec3(1.0) - specularStrength;
    diffuseStrength *= 1.0 - metallic; //Conductors (metals) don't have any diffuse reflectance

    vec3 numerator = NDF * G * F;
    float denominator = max(4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, -incomingDirection), 0.0), 0.001);
    vec3 specular = numerator / denominator;

    float cosWeighting = max(dot(normal, -incomingDirection), 0.0);
    vec3 outgoingRadiance = (diffuseStrength * albedo / PI + specular) * incomingRadiance * cosWeighting;


    //Ambient
    float ambientStrength = 0.005;
    vec3 ambient = ambientStrength * albedo * ao;


    vec3 colour = ambient + emissive + outgoingRadiance;
    outColour = vec4(colour, 1.0);
}




//#version 450
//
//layout(location = 0) in vec2 TexCoord;
//layout(location = 1) in vec3 FragPos;
//layout(location = 2) in vec3 WorldNormal;
//layout(location = 3) in vec3 CamPos;
//layout(location = 4) in mat3 TBN;
//
//layout(set = 1, binding = 0) uniform sampler2DArray albedoSampler;
//layout(set = 1, binding = 1) uniform sampler2DArray normalSampler;
//layout(set = 1, binding = 3) uniform sampler2DArray metallicSampler;
//layout(set = 1, binding = 4) uniform sampler2DArray roughnessSampler;
//layout(set = 1, binding = 5) uniform sampler2DArray aoSampler;
//layout(set = 1, binding = 6) uniform sampler2DArray emissiveSampler;
//
//layout(location = 0) out vec4 outColour;
//
//void main()
//{
//    //todo: implement cook-torrance pbr brdf with these - this will probably be done in VulkanPBR repo (https://github.com/Kahoneki/VulkanPBR)
//    float metallic = texture(metallicSampler, vec3(TexCoord, 0)).g;
//    float roughness = texture(roughnessSampler, vec3(TexCoord, 0)).b;
//
//    vec3 lightPos = vec3(1,-1,1);
//    vec3 lightColour = vec3(1.0,0.6,1.0);
//    float lightIntensity = 0.05;
//
//    vec3 albedo = texture(albedoSampler, vec3(TexCoord, 0)).rgb;
//    vec3 tangentNormal = texture(normalSampler, vec3(TexCoord, 0)).rgb * 2.0 - 1.0;
//    vec3 worldNormal = normalize(TBN * tangentNormal);
//
//    //Ambient
//    float ambientStrength = 0.005;
//    float ao = texture(aoSampler, vec3(TexCoord, 0)).r;
//    vec3 ambient = (ambientStrength * lightColour * lightIntensity) * ao;
//
//    //Diffuse
//    vec3 lightDir = normalize(lightPos - FragPos);
//    float diffusePow = max(dot(worldNormal, lightDir), 0.0);
//    vec3 diffuse = diffusePow * lightColour * lightIntensity;
//
//    //Blinn-Phong specular
//    vec3 viewDir = normalize(CamPos - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float specPow = pow(max(dot(halfwayDir, worldNormal), 0.0), 128);
//    vec3 specular = 16 * specPow * lightColour * lightIntensity;
//
//    //Emissive
//    vec3 emissive = 4 * texture(emissiveSampler, vec3(TexCoord, 0)).rgb;
//
//    vec3 finalColour = ((ambient + diffuse + specular) * albedo) + emissive;
//
//    outColour = vec4(finalColour, 1.0);
//}