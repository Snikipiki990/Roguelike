#ifndef shader_text
#define shader_text

#include "shader.hpp"

namespace ShaderText{


    // sky box shader
    const char* skyBoxVertex = R"glsl(
#version 420 core
layout(location = 0) in vec3 aPos;    
layout(location = 1) in vec3 aNormal;
layout(std140 , binding = 0) uniform Matrix{
mat4 proj;
mat4 view;
};
out vec3 newTex;


void main(){
    newTex = aPos;
    gl_Position = proj * mat4(mat3(view)) * vec4(aPos , 1.0f);
}
    
)glsl";




    const char* skyBoxFragment = R"glsl(
#version 420 core    
in vec3 newTex;
layout(binding = 0)uniform samplerCube skyboxDay;
layout(binding = 1)uniform samplerCube skyboxNight;
uniform float factor;
uniform vec3 fogColor;
out vec4 fragColor;
void main(){
    vec4 result = vec4(mix(texture(skyboxNight , newTex) , texture(skyboxDay , newTex) , factor)); 
    fragColor = vec4(vec3(mix(vec3(result) , fogColor , 0.75f)) , 1.0f);
}    
    
)glsl";








    // screen shader first

    const char* vertexScreen = R"glsl(
#version 420  core
layout(location = 0) in vec3 aPos;    
layout(location = 1) in vec2 aTex;
out vec2 newTex;
void main(){
    newTex = aTex;
    gl_Position = vec4(aPos , 1.0f);
}
)glsl";


    
    const char* fragmentScreen = R"glsl(
#version 420 core    
in vec2 newTex;
layout(binding = 0) uniform sampler2D textureID;
out vec4 fragColor;
void main(){
    fragColor = texture(textureID , newTex);
}    
)glsl";




    // shader for nightVision  + screen
    const char* nightVisionFragment = R"glsl(
#version 420 core
in vec2 newTex;
layout(binding = 0)uniform sampler2D textureID;
out vec4 fragColor;

void main() {
    vec4 color = texture(textureID, newTex);
    float lum = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    lum = pow(lum, 0.8); 
    vec3 nightColor = vec3(lum * 0.2, lum * 0.4, lum);
    fragColor = vec4(nightColor, 1.0);
}

)glsl";







    // fragment for crystal
    const char* crystalFragment = R"glsl(
#version 420 core
layout(std140 , binding = 1) uniform Fog{
float far;
float near;
vec3 fogColor;
};
uniform vec3 color;
out vec4 fragColor;

float calcDepth(float depth){
    float z = 2.0f * depth - 1.0f;
    return (2.0f * near * far) / (far + near - z * (far - near));
}


    
void main(){
    float depth = calcDepth(gl_FragCoord.z);
    float factor = depth / far;
    vec3 result = mix(color , fogColor , factor);
    fragColor = vec4(result , 1.0f);
}
)glsl";









    // rain vertex
    const char* rainVertex = R"glsl(
#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;
layout(std140 , binding = 0) uniform Matrix{
mat4 proj;
mat4 view;
};

out vec4 ParticleColor;

void main() {
    ParticleColor = aColor;
    vec4 viewPos = view * vec4(aPos, 1.0);
    gl_Position = proj * viewPos;
    gl_PointSize = aSize * (400.0 / length(viewPos.xyz));
}
)glsl";


    // rain fragment
    const char* rainFragment = R"glsl(
    #version 420 core
in vec4 ParticleColor;
out vec4 FragColor;

void main() {

    float distance = length(gl_PointCoord - vec2(0.5));
    if (distance > 0.5) {
        discard;
    }
    float edgeSoftness = smoothstep(0.5, 0.3, distance);
    
    FragColor = vec4(ParticleColor.rgb, ParticleColor.a * edgeSoftness);
}
)glsl";







    // fragment without texture
    const char* fragmentSoloColor = R"glsl(
#version 420 core    
uniform vec3 color;
out vec4 fragColor;

void main(){
    fragColor = vec4(color , 1.0f);
}    
)glsl";






    // similar to vertex main but for 2d
    const char* vertexInventory = R"glsl(
#version 420 core    
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(std140 , binding = 3) uniform Matrix2D{
    mat4 proj;
    mat4 view;
};
uniform mat4 model;
out vec2 newTex;
void main(){
    gl_Position = proj * view * model * vec4(aPos , 1.0f);
    newTex = aTex;
}    
    
)glsl";

    const char* fragmentInventory = R"glsl(
#version 420 core
in vec2 newTex;
uniform vec3 color;
out vec4 fragColor;

void main(){
    float distant = length(newTex - vec2(0.5));
    if(distant > 0.5){
        discard;
    }
    float factor = smoothstep(0.5 , 0.3 , distant);
    fragColor = vec4(color, 1.0f * factor);
}
)glsl";

    const char* fragmentSquareInventory = R"glsl(
#version 420 core
uniform vec3 color;    
out vec4 fragColor;
void main(){
    fragColor = vec4(color , 1.0f);
}
    
    
)glsl";


    const char* inventoryFragmentTexture = R"glsl(
#version 420 core    
in vec2 newTex;
layout(binding = 0)uniform sampler2D textureID;
out vec4 fragColor;
void main(){    
    vec3 Texture = vec3(texture(textureID , newTex));
    vec3 result = mix(Texture , vec3(0.0f) , 0.7f);
    fragColor = vec4(result , 1.0f);
}    
)glsl";







    // main vertex
    const char* vertexSource = R"glsl(
#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;    

layout(std140 , binding = 0) uniform Matrix{
    mat4 proj;
    mat4 view;
};

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};
uniform mat4 model;

out vec4 fragSpace;

out VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
} vs_out;

void main(){
    gl_Position = proj * view * model * vec4(aPos , 1.0f);
    vs_out.normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.newTex = aTex;
    vs_out.fragPos = vec3(model * vec4(aPos , 1.0f));
    fragSpace = vec4(lightSpaceMatrix * vec4(vs_out.fragPos , 1.0f));
}
)glsl";

    // mainFragment
    

    const char* fragmentSource = R"glsl(
#version 420 core

// will work as a crystal source of light
struct PointLight{

    vec3 position;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    float linear;
    float quadratic;
};    

// will work as a sun

struct DirLight{
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
}; 

// obvious

struct FlashLight{
    vec3 direction;
    vec3 position;
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    
    float linear;
    float quadratic;

    float cutoff;
    float outercutoff;
};

struct Material{
    int shiny;
    float trans;
    sampler2D diffuse;
};

#define MAX 10

in VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
} vs_in;

in vec4 fragSpace;

uniform vec3 cameraPos;
layout(std140 , binding = 1) uniform Fog{
    float far;
    float near;
    vec3 fogColor;
};

layout(binding = 5) uniform sampler2D shadowMap;

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};
uniform Material material;
uniform DirLight sun;
uniform PointLight crystal[MAX];
uniform FlashLight flash;


out vec4 fragColor;

float calcDepth(float depth){
    float z = 2.0f * depth - 1.0f;
    return (2.0f * near * far) / (far + near - z * (far - near));
}


// shadow calculation
float calcShadow(vec4 lightSpace , vec3 normal , vec3 lightDir){
    vec3 projCoord = lightSpace.xyz / lightSpace.w;
    projCoord = projCoord * 0.5f + 0.5f;
    float currentDepth = projCoord.z;
    if(currentDepth > 1.0f) return 0.0f;
    float bios = max(0.005 * (1.0f- dot(normal , lightDir)), 0.001f);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) { 
            float pcfDepth = texture(shadowMap, projCoord.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bios) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0f;
    return shadow;
}

// functions
vec3 calcDirLight(DirLight light , vec3 ViewPos , vec3 normal , float shadow){
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient = vec3(texture(material.diffuse , vs_in.newTex))  * light.ambient;

    float diff = max(dot(lightDir , normal) , 0.0f);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse , vs_in.newTex));

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f) , material.shiny);
    vec3 specular = spec * vec3(texture(material.diffuse , vs_in.newTex)) * light.specular;


    float shadowAmbient = 0.7f;
    shadow *= shadowAmbient;
    return ambient + (1.0f - shadow) * (diffuse + specular);
    
}

vec3 calcPointLight(PointLight light, vec3 ViewPos , vec3 normal){
    
    vec3 lightDir = normalize(light.position - vs_in.fragPos);
    float distant = length(light.position - vs_in.fragPos);

    float att = 1.0f / (1.0f + light.linear * distant + light.quadratic * (distant * distant));

    vec3 ambient = light.ambient * vec3(texture(material.diffuse , vs_in.newTex));
    ambient *= att;

    float diff = max(dot(lightDir , normal) , 0.0f);
    vec3 diffuse = diff * vec3(texture(material.diffuse , vs_in.newTex)) * light.diffuse;
    diffuse *= att;

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f ) , material.shiny);
    vec3 specular = spec * light.specular * vec3(texture(material.diffuse , vs_in.newTex));
    specular *= att;
    return ambient + specular + diffuse;
}

vec3 calcFlashLight(FlashLight light , vec3 ViewPos , vec3 normal){

    vec3 lightDir = normalize(light.position - vs_in.fragPos);
    float distant = length(light.position - vs_in.fragPos);
    float att = 1.0f / (1.0f + light.linear * distant + light.quadratic * (distant * distant));

    float theta = dot(-lightDir , normalize(light.direction));
    float epsilon = light.cutoff - light.outercutoff;
    float intensity = clamp( (theta - light.outercutoff) / epsilon , 0.0f , 1.0f);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse , vs_in.newTex));
    ambient *= att;
    ambient *= intensity;

    float diff = max(dot(lightDir , normal) , 0.0f);
    vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse , vs_in.newTex));
    diffuse *= att;
    diffuse *= intensity;

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f) , material.shiny);
    vec3 specular = spec * light.specular * vec3(texture(material.diffuse , vs_in.newTex));
    specular *= att;
    specular *= intensity;
    return specular + diffuse + ambient;
}
    
void main(){
    float depth = calcDepth(gl_FragCoord.z);
    float factor = depth/far;
    vec3 ViewPos = normalize(cameraPos - vs_in.fragPos);
    float shadow = calcShadow(fragSpace , vs_in.normal , -normalize(sun.direction));
    vec3 result = calcDirLight(sun, ViewPos , vs_in.normal , shadow);
    for(int i = 0  ; i < MAX ; i++){
        result += calcPointLight(crystal[i] , ViewPos , vs_in.normal);
    }
    result += calcFlashLight(flash , ViewPos , vs_in.normal);
    vec3 final = mix(result , fogColor , factor);
    fragColor = vec4(final , material.trans);
}
)glsl";






    const char* instanceVertex = R"glsl(
#version 420 core    
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in mat4 model;

layout(std140 , binding = 0 ) uniform Matrix{
mat4 proj;
mat4 view;
};

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};
out vec4 fragSpace;

out VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
} vs_out;

void main(){
    vs_out.normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.newTex = aTex;
    vs_out.fragPos = vec3(model * vec4(aPos , 1.0f));
    gl_Position = proj * view * model * vec4(aPos , 1.0f);
    fragSpace = vec4(lightSpaceMatrix * vec4(vs_out.fragPos , 1.0f));
}
)glsl";
}





const char* shadowVertex = R"glsl(
#version 420 core
layout(location = 0 ) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};

uniform mat4 model;

void main() {   gl_Position = lightSpaceMatrix * model * vec4(aPos , 1.0f);     }
)glsl";


    const char* shadowInstanceVertex = R"glsl(
#version 420 core    
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in mat4 instanceModel;
layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};
void main(){    gl_Position = lightSpaceMatrix * instanceModel * vec4(aPos , 1.0f);     }
)glsl";


    const char* shadowFragment = R"glsl(
#version 420 core    
void main() {}
)glsl";






    const char* normalVertexSource  = R"glsl(
#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;    

layout(std140 , binding = 0) uniform Matrix{
    mat4 proj;
    mat4 view;
};

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};
uniform mat4 model;

out VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
    vec4 fragSpace;
} vs_out;

void main(){
    gl_Position = proj * view * model * vec4(aPos , 1.0f);
    vs_out.normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.newTex = aTex;
    vs_out.fragPos = vec3(model * vec4(aPos , 1.0f));
    vs_out.fragSpace = vec4(lightSpaceMatrix * vec4(vs_out.fragPos , 1.0f));
}
)glsl";





    const char* geometryNormalSource = R"glsl(
#version 420 core
layout(triangles) in;
layout(triangle_strip , max_vertices = 3) out;

in VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
    vec4 fragSpace;
} vs_in[];

out GS_OUT{
    mat3 TBN;
    vec2 newTex;
    vec3 fragPos;
    vec4 fragSpace;
} gs_out;

void main(){
    vec3 edge1 = vs_in[1].fragPos - vs_in[0].fragPos;
    vec3 edge2 = vs_in[2].fragPos - vs_in[0].fragPos;
    vec2 UV1 = vs_in[1].newTex - vs_in[0].newTex;
    vec2 UV2 = vs_in[2].newTex - vs_in[0].newTex;
    float f = 1.0f/(UV1.x * UV2.y - UV2.x * UV1.y);
    vec3 T = normalize(f * (edge1 * UV2.y - edge2 * UV1.y));
    for(int i = 0 ; i < 3 ; i++){
        gs_out.fragPos = vs_in[i].fragPos;
        gs_out.newTex  = vs_in[i].newTex;
        gs_out.fragSpace = vs_in[i].fragSpace;
        vec3 N = normalize(vs_in[i].normal);
        vec3 T_temp = normalize(T - dot(N , T) * N);
        vec3 B = cross(N, T_temp);
        gs_out.TBN = mat3(T_temp , B , N);
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
)glsl";




    const char* normalFragmentSource = R"glsl(
#version 420 core

// will work as a crystal source of light
struct PointLight{

    vec3 position;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    float linear;
    float quadratic;
};    

// will work as a sun

struct DirLight{
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
}; 

// obvious

struct FlashLight{
    vec3 direction;
    vec3 position;
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    
    float linear;
    float quadratic;

    float cutoff;
    float outercutoff;
};

struct Material{
    int shiny;
    float trans;
    sampler2D diffuse;
};

#define MAX 10

in GS_OUT{
    mat3 TBN;
    vec2 newTex;
    vec3 fragPos;
    vec4 fragSpace;
} gs_in;

layout(std140 , binding = 1) uniform Fog{
    float far;
    float near;
    vec3 fogColor;
};

layout(binding = 5) uniform sampler2D shadowMap;
layout(binding = 10) uniform sampler2D normalMap;
layout(binding = 11) uniform sampler2D depthMap;
uniform Material material;
uniform DirLight sun;
uniform PointLight crystal[MAX];
uniform FlashLight flash;


out vec4 fragColor;    
    
vec2 texCoord;

float calcDepth(float depth){
    float z = depth * 2.0f - 1.0f;
    return (far * 2.0f * near)/(far + near - z * (far - near));
}
    
float calcShadow(vec4 vector , vec3 normal , vec3 lightDir){
    vec3 projCoord = vector.xyz / vector.w;
    projCoord = projCoord * 0.5f + 0.5f;
    float currentDepth = projCoord.z;
    float shadow = 0.0f;
    float bias = max(0.05f * (1.0f - dot(normal , lightDir)) , 0.005f);
    vec2 textSize = 1.0f/textureSize(shadowMap , 0);
    for(int x = -2 ; x <= 2 ; x++){
        for(int y = -2 ; y <= 2 ; y++){
            float pcfDepth = texture(shadowMap , projCoord.xy + vec2(x , y) * textSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0f : 0.0f;
        }
    }
    return shadow / 25.0f;
}

vec3 calcDirLight(DirLight light , vec3 normal , vec3 ViewPos , float shadow){
    vec3 lightDir = normalize(-light.direction);
    vec3 color = vec3(texture(material.diffuse , texCoord));

    vec3 ambient = light.ambient * color;

    float diff = max(dot(normal , lightDir), 0.0f);
    vec3 diffuse = diff * color * light.diffuse;

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f) , material.shiny);
    vec3 specular = spec * color * light.specular;

    float shadowAmbient = 0.7f;
    shadow *= shadowAmbient;

    return ambient + (1.0f - shadow) * (specular + diffuse);
}

vec3 calcPointLight(PointLight light , vec3 normal , vec3 ViewPos){
    vec3 lightDir = normalize(light.position - gs_in.fragPos);
    float distant = length(light.position - gs_in.fragPos);
    float att = 1.0f/(1.0f + light.linear * distant + distant * distant * light.quadratic);

    vec3 color = texture(material.diffuse , texCoord).xyz;
    
    vec3 ambient = light.ambient * color * att;

    float diff = max(dot(normal , lightDir) , 0.0f);
    vec3 diffuse = diff * att * color * light.diffuse;

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f) , material.shiny);
    vec3 specular = spec * att * color * light.specular;
    return ambient + diffuse + specular;
}

vec3 calcFlashLight(FlashLight light , vec3 normal , vec3 ViewPos){
    vec3 lightDir = normalize(light.position - gs_in.fragPos);
    float distant = length(light.position - gs_in.fragPos);
    float att = 1.0f/(1.0f + distant * light.linear + distant * light.quadratic * distant );
    float theta = dot(lightDir , normalize(-light.direction));
    float epsilon = light.cutoff - light.outercutoff;
    float intensity = clamp((theta - light.outercutoff) / epsilon , 0.0f , 1.0f);

    vec3 color = texture(material.diffuse , texCoord).xyz;

    vec3 ambient = color * light.ambient * att * intensity;
    
    float diff = max(dot(normal , lightDir), 0.0f);
    vec3 diffuse = diff * color * light.diffuse * att * intensity;

    vec3 highway = normalize(ViewPos + lightDir);
    float spec = pow(max(dot(normal , highway) , 0.0f ) , material.shiny);
    vec3 specular = spec * light.specular * color * att * intensity;

    return specular + diffuse + ambient;
}

vec2 calcTexCoord(vec2 texCoord , vec3 ViewDir){
    float height = texture(depthMap , texCoord).r;
    float depth = 1.0f - height;
    float heightScale = 0.01f;
    vec2 p = ViewDir.xy/ViewDir.z * (depth * heightScale);
    return texCoord - p;
}

void main(){
    vec3 cameraPos = flash.position;

    vec3 ViewPos = normalize(cameraPos - gs_in.fragPos);
    vec3 viewDirTBN = transpose(gs_in.TBN) * ViewPos;
    viewDirTBN = normalize(viewDirTBN);
    texCoord = calcTexCoord(gs_in.newTex , viewDirTBN);

    vec3 normalTexture = texture(normalMap , texCoord).xyz;
    normalTexture = normalTexture * 2.0f - 1.0f;
    vec3 normal = normalize(gs_in.TBN * normalTexture);

    

    float shadow = calcShadow(gs_in.fragSpace , normal , normalize(-sun.direction));


    vec3 result = calcDirLight(sun , normal , ViewPos , shadow); 
    for(int i = 0 ; i < MAX ; i++){
        result += calcPointLight(crystal[i] , normal , ViewPos);
    }
    result += calcFlashLight(flash , normal , ViewPos);
    float depth = calcDepth(gl_FragCoord.z);
    float factor = depth/far;
    vec3 finalColor = mix(result , fogColor , factor);
    fragColor = vec4(finalColor , material.trans);
}
    
)glsl";



    const char* instanceNormalVertexSource = R"glsl(
#version 420 core    
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in mat4 model;

layout(std140 , binding = 0 ) uniform Matrix{
mat4 proj;
mat4 view;
};

layout(std140 , binding = 2) uniform shadowMatrix{
    mat4 lightSpaceMatrix;
};


out VS_OUT{
    vec3 normal;
    vec2 newTex;
    vec3 fragPos;
    vec4 fragSpace;
} vs_out;

void main(){
    vs_out.normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.newTex = aTex;
    vs_out.fragPos = vec3(model * vec4(aPos , 1.0f));
    gl_Position = proj * view * model * vec4(aPos , 1.0f);
    vs_out.fragSpace = vec4(lightSpaceMatrix * vec4(vs_out.fragPos , 1.0f));
}
)glsl";


/*
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_in[]; // Массив из 3-х вершин

out GS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} gs_out;

void main() {
    // 1. Вычисляем тангент для всего треугольника
    vec3 edge1 = vs_in[1].FragPos - vs_in[0].FragPos;
    vec3 edge2 = vs_in[2].FragPos - vs_in[0].FragPos;
    vec2 deltaUV1 = vs_in[1].TexCoords - vs_in[0].TexCoords;
    vec2 deltaUV2 = vs_in[2].TexCoords - vs_in[0].TexCoords;

    float f = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    vec3 T = normalize(f * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
    
    // 2. Генерируем 3 вершины для фрагментного шейдера
    for(int i = 0; i < 3; i++) {
        gs_out.FragPos = vs_in[i].FragPos;
        gs_out.TexCoords = vs_in[i].TexCoords;
        
        // Ортогонализация Грама-Шмидта для каждой вершины
        vec3 N = normalize(vs_in[i].Normal);
        vec3 T_ortho = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T_ortho);
        
        gs_out.TBN = mat3(T_ortho, B, N);
        
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}

*/
#endif