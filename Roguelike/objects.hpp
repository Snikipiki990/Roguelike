#ifndef OBJECTS
#define OBJECTS




#include "model.hpp"
#include <array>
#include "ItemTypes.hpp"
#include "../globalValues/global.hpp"
#include "funcs.hpp"


class Item : virtual public Model{
protected:
    std::string type{"none"};
    float radius{0.0f};
    void setType(std::string const &type){
        if(type.empty()) return;
        
        if(itemType::checkType(type)){
            this->type = type;
            hasType = true;
            delcType(type);
        }else{
            hasType = false;
        }
    }

    // needs to be used only if the check has been completed
    void delcType(std::string const &type){
        if(type.empty()) return;
        if(itemType::checkType(type)){
            if(type == "adrenaline"){
                duration = itemType::Adrenaline::durationTime;
                uses = 1;
                maxDuration = duration;
            }else if(type == "cross"){
                radius = itemType::Cross::radius;
                duration = itemType::Cross::durationTime;
                uses = 1;
                maxDuration = duration;
            }else if(type == "nightVision"){
                duration = itemType::NightVision::durationTime;
                maxDuration = duration;
            }else if(type == "crystalExposer"){
                uses = itemType::CrystalExposer::uses;
                radius = itemType::CrystalExposer::radius;
                duration = itemType::CrystalExposer::durationTime;
                maxDuration = duration;
            }else if(type == "teleporter"){
                uses = itemType::Teleporter::uses;
                radius = itemType::Teleporter::radius;
            }else if(type == "battery"){
                uses = 1;
            }else{
                radius = itemType::mobStunner::radius;
                uses = itemType::mobStunner::uses;
                duration = itemType::mobStunner::durationTime;
                maxDuration = duration;
            } // dont need a battery since it will just increase the amount of time for flashlight
        }
    }
    
public:
    Item(std::string const &path){
        loadModel(path);
        hasTexture = false;
        hasType = false;
    }
    Item(std::string const &path , GLuint &texture) : Item(path){
        this->texture = texture;
        hasTexture = true;
        
    }
    Item(std::string const &path , GLuint &texture , std::string const &type) : Item(path , texture){
        setType(type);
    }
    Item(std::string const &path , std::string const &type): Item(path) {
        setType(type);
    }
    bool hasTaken{false};
    bool isUsed{false};
    bool isActive{false};
    bool isCursed{false};
    unsigned int duration{0};
    bool hasType{false};
    unsigned int uses{1};

    short maxDuration;
    // if it has the light
    float size{1.0f};

    std::optional<glm::vec3> color;
    glm::vec3 position;

    

    inline std::string getType() const {return type;}
    inline float getRadius() const {return radius;}

    virtual void checkPlayerNear(const glm::vec3 pos){
        playerNearBy = checkRange(position , 10.0f , pos);

    }

    virtual void draw(Shader* shader) override final {
        if(!hasType) return;
        if(hasTaken) return;
        bool result = false;
        if(hasPersonalSettings.has_value()) result = hasPersonalSettings.value();
        if(!result){
            for(auto& mesh : meshes){
                mesh->model = model;
                mesh->texture = texture;
                mesh->hasTexture = hasTexture;
                mesh->shiny = shiny;
                mesh->trans = trans;
                
                mesh->draw(shader);
            }
        }else{
            for(auto& mesh : meshes){
                mesh->hasTexture = true;
                mesh->model = model;
                mesh->draw(shader);
            }
                //  if i want to set my own specular for each mesh 
                // need to set SPECULAR , SHINY , TRANS , TEXTURE 
        }

    }
    virtual ~Item() {}
};

class FlashLight{
public:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 specular;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    
    float linear;
    float quadratic;

    float cutoff = constValues::min_cutoff;
    float outercutoff = constValues::min_outercutoff;

    int duration = constValues::maxSecondsFlashLight;
    unsigned int maxDuration = constValues::maxSecondsFlashLight;
    bool flashTurn{false};
    bool batteryEmpty{false};
};



#endif