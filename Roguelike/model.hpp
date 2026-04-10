#ifndef MODEL_HPP
#define MODEL_HPP

#include "mesh.hpp"
#include <optional>

template<typename T>
[[nodiscard]] inline auto percentage(const unsigned int &&perc , const T data) noexcept {
    return (data/100) * perc;
}

class Model{
protected:
    virtual void loadModel(std::string const &path){
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path , aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            std::cout << path << " "<< "ASSIMP_ERROR:" << import.GetErrorString() << std::endl;
            return;
        }else{
            std::cout << path << " model [SUCCESS] \n";
        }
        proccessNode(scene->mRootNode , scene);
    }
    virtual void proccessNode(aiNode* node, const aiScene* scene){
        for (unsigned int i = 0 ; i < node->mNumMeshes ; i++){
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh , scene));
        }
        for(unsigned short i = 0  ; i < node->mNumChildren ; i++){
            proccessNode(node->mChildren[i] , scene);
        }
    }
    [[nodiscard]] virtual std::shared_ptr<Mesh> processMesh(aiMesh* mesh , const aiScene* scene){
        std::vector<float> vertices;
        std::vector<unsigned short> indices;
        for(unsigned int i = 0 ; i < mesh->mNumVertices ; i++){
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            if(mesh->HasNormals()){
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }else{
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            if(mesh->mTextureCoords[0]){
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }else{
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
        for(unsigned short i = 0 ; i < mesh->mNumFaces ; i++){
            aiFace face =  mesh->mFaces[i];
            for(unsigned short j = 0 ; j < face.mNumIndices ; j++){
                indices.push_back(face.mIndices[j]);
            }
        }
        return std::make_shared<Mesh>(vertices , indices);
    }
    Model() = default;
public:

    std::vector<std::shared_ptr<Mesh>> meshes;
    bool hasTexture{true};
    GLint shiny{2};
    float trans{1.0f};

    glm::mat4 model{glm::mat4(1.0f)};
    GLuint texture;
    std::optional<GLuint> normalTexture;
    
    unsigned int ID;
    // bool hasPersonalSettings{false};
    std::optional<bool> hasPersonalSettings;
    // radius is for objects between each other
    float radius{(float)DefaultRadius};
    //  range is for player
    std::optional<float> range;
    std::optional<bool> playerNearBy;
    std::optional<GLuint> depthTexture;
    glm::vec3 pos;

    Model(std::string const &path){
        loadModel(path);
        hasTexture = false;
    }
    Model(std::string const &path , GLuint &texture) : Model(path)  { hasTexture = true; this->texture = texture;}

    virtual void draw(Shader* shader)  {
        bool value = false;
        if(hasPersonalSettings.has_value()) value = hasPersonalSettings.value();

        if(!value){
            for(unsigned short i = 0 ; i < meshes.size() ; i++){
                meshes[i]->model = model;

                if(hasTexture){
                    meshes[i]->hasTexture = true;
                    meshes[i]->texture = texture;
                }
                if(normalTexture) meshes[i]->normalTexture = this->normalTexture.value();
                meshes[i]->shiny = shiny;
                meshes[i]->trans = trans;
                if(depthTexture) meshes[i]->depthTexture = depthTexture;
                meshes[i]->draw(shader);
            }
        } else {
            // keep per-mesh custom settings (no override)
            for(unsigned short i = 0 ; i < meshes.size() ; i++){
                meshes[i]->model = model;
                meshes[i]->draw(shader);
            }
        }

    }
    virtual ~Model() {}
};


class InstanceModel{
protected:
    InstanceModel() = default;
    void loadModel(std::string const &path) noexcept {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path , aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
        [[unlikely]]if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            std::cout << path << " "<< "ASSIMP_ERROR:" << import.GetErrorString() << std::endl;
            return;
        }else[[likely]]{
            std::cout << path << " INSTANCE model [SUCCESS] \n";
        }
        proccessNode(scene->mRootNode , scene);
    }
    virtual void proccessNode(aiNode* node, const aiScene* scene) noexcept {
        for (unsigned int i = 0 ; i < node->mNumMeshes ; i++){
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh , scene));
        }
        for(unsigned short i = 0  ; i < node->mNumChildren ; i++){
            proccessNode(node->mChildren[i] , scene);
        }
    }

    [[nodiscard]] virtual std::shared_ptr<InstanceMesh> processMesh(aiMesh* mesh , const aiScene* scene) noexcept{
        std::vector<float> vertices;
        std::vector<unsigned short> indices;
        for(unsigned int i = 0 ; i < mesh->mNumVertices ; i++){
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            if(mesh->HasNormals()){
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }else{
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            if(mesh->mTextureCoords[0]){
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }else{
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
        for(unsigned short i = 0 ; i < mesh->mNumFaces ; i++){
            aiFace face =  mesh->mFaces[i];
            for(unsigned short j = 0 ; j < face.mNumIndices ; j++){
                indices.push_back(face.mIndices[j]);
            }
        }
        return std::make_shared<InstanceMesh>(vertices , indices , this->matrix);
    } 

public:
    std::vector<std::shared_ptr<InstanceMesh>> meshes;
    std::optional<bool> personalSetting;
    bool hasTexture{false};
    GLuint texture;
    std::vector<glm::mat4> matrix;
    std::optional<GLuint> normalTexture;
    std::optional<GLuint> depthTexture;
    GLint shiny{8};
    float trans{1.0f};
    explicit InstanceModel(const std::string &path , std::vector<glm::mat4> const &matrix){
        this->matrix = matrix;
        loadModel(path);
        hasTexture = false;
    }
    explicit InstanceModel(const std::string &path , std::vector<glm::mat4> const &matrix , GLuint &texture){
        this->matrix = matrix;
        loadModel(path);
        hasTexture = true;
        this->texture = texture;
    }

    virtual inline void draw(Shader* shader) noexcept{
        [[unlikely]] if(!shader) return;
        bool checkUp = false;
        if(personalSetting) checkUp = personalSetting.value();
        if(!checkUp){
            for(auto& mesh : meshes){
                mesh->trans = trans;
                mesh->texture = texture;
                mesh->hasTexture = hasTexture;
                mesh->shiny = shiny;
                if(normalTexture) mesh->normalTexture = normalTexture;
                if(depthTexture) mesh->depthTexture = depthTexture;
                mesh->draw(shader);
            }
        }else{
            for(auto& mesh: meshes){
                mesh->draw(shader);
            }
        }
    }

    virtual ~InstanceModel() {}
};


#endif