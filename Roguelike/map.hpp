#ifndef map_hpp
#define map_hpp

#define Attemps 2000



#include "npc.hpp"
#include <ctime>
#ifndef FUNC_HPP
#include "funcs.hpp"
#endif

#ifndef Global_HPP
#include "global.hpp"

#endif
#ifndef accessoryType
#include "accessoryType.hpp"
#endif

const unsigned short offset{5};
using globalValues::allPos;


class Map{
protected:
Map() = delete;
    void ReadFile(std::string const &path) noexcept;
public:
    unsigned short size{constValues::maxMapSize};
    unsigned short treeAmount{constValues::maxTreePos};
    unsigned short rockAmount{constValues::maxRock};
    unsigned short mobAmount{constValues::maxMobs};
    unsigned short itemAmount{constValues::maxItems};
    Map(std::string const &path) {ReadFile(path);}
};

void Map::ReadFile(std::string const &path) noexcept {
    std::ifstream file(path);
    std::string line;

    if (!file.is_open()) {
        logAction("Failed to open settings file");
        return;
    }

    logAction("file with settings has been opened");

    while (std::getline(file, line)) {
        size_t sep = line.find('=');
        if (sep == std::string::npos) continue;

        int val = std::stoi(line.substr(sep + 1));

        if (line.find("map size") != std::string::npos)
            size = std::min((int)constValues::maxMapSize, val);
        else if (line.find("tree amount") != std::string::npos)
            treeAmount = std::min((int)constValues::maxTreePos, val);
        else if (line.find("rock amount") != std::string::npos)
            rockAmount = std::min((int)constValues::maxRock, val);
        else if (line.find("mob amount") != std::string::npos)
            mobAmount = std::min((int)constValues::maxMobs, val);
        else if (line.find("item amount") != std::string::npos)
            itemAmount = std::min((int)constValues::maxItems, val);
    }
}

[[nodiscard]] std::vector<glm::vec3> generatePos( const unsigned short &size ,const unsigned short &amount , const float &radius , const float &Y) noexcept 
{//basicly this function is for responsible for generation of positions, radius is for certain distance, it stores all the information in a buffer that has all the pos
    std::vector<glm::vec3> positions;
    glm::vec3 currentVector{glm::vec3(0.0f)};
    const auto func = [&](const glm::vec3 &vector) -> bool {
        bool result_1 = (((vector.x + radius) < currentVector.x - radius) || ((vector.x - radius) > currentVector.x + radius)); 
        bool result_2 = ((vector.z + radius < currentVector.z - radius) || (vector.z - radius > currentVector.z + radius));
        return result_1 || result_2;
    };
    
    const unsigned short Coord = (unsigned short)(size * (unsigned short)(constValues::sizePlatform)) - offset;

    for(unsigned short i = 0 ; i < Attemps ; i++){
        const unsigned short firstPower = rand()%2 + 1;
        const unsigned short secondPower = rand()%2 + 1;
        currentVector.x = (float)(pow(-1 , firstPower) * (rand()%Coord));
        currentVector.y = Y;
        currentVector.z = (float)(pow(-1 , secondPower) * (rand()%Coord));
        if(allPos.empty()){
            const auto temp = currentVector;
            allPos.push_back(temp);
            positions.push_back(temp);
            continue;
        }else{
            bool result{true};
            for(auto& vector : allPos){
                result = func(vector);
                if(!result) break;
            }
            if(!result) continue;
            const auto temp = currentVector;
            allPos.push_back(temp);
            positions.push_back(temp);
        }
        if(positions.size() == amount) break;
    }
    return positions;
}

[[nodiscard]] std::vector<std::shared_ptr<Item>> generateItems(std::vector<glm::vec3> const &pos , GLuint &adrenalineTexture , GLuint &crossTexture ,
                                                    GLuint &metal) noexcept { // generates all the items with their positions, since all the item has it own model
    // at least one position should be
    std::vector<std::shared_ptr<Item>> items;
    for(const auto& position: pos){
        const unsigned short index = rand()%5 + 1;
        const bool hasCurse = (rand()%5) == 0 ? true : false;
        std::cout << "next item  " << (hasCurse ? " [HAS CURSE]" : " [NO CURSE]") << std::endl;
        if(index == 1){
            std::shared_ptr<Item> adrenaline = std::make_shared<Item>("adrenaline.obj" , adrenalineTexture , "adrenaline");
            adrenaline->shiny = 32;
            adrenaline->trans = 1.0f;
            adrenaline->hasPersonalSettings = false;
            adrenaline->size = 0.02f;
            adrenaline->position = position;
            adrenaline->range = 10.0f;
            adrenaline->isCursed = hasCurse;
            items.push_back(adrenaline);
        }else if(index == 2){
            std::shared_ptr<Item> battery = std::make_shared<Item>(Item("battery.obj" , metal , "battery"));
            battery->shiny = 16;
            battery->trans = 1.0f;
            battery->size = 0.5f;
            battery->position = position;
            battery->isCursed = hasCurse;
            items.push_back(battery);
        }else if(index == 3){
            std::shared_ptr<Item> cross = std::make_shared<Item>("cross.obj" , crossTexture , "cross");
            cross->shiny = 8;
            cross->trans = 1.0f;
            cross->size = 1.5f;
            cross->position = position;
            cross->range = 10.0f;
            items.push_back(cross);
        }else if(index == 4){
            std::shared_ptr<Item> nightVision = std::make_shared<Item>("nightVision.obj" , metal, "nightVision");
            nightVision->shiny = 32;
            nightVision->trans = 1.0f;
            nightVision->size = 4.0f;
            nightVision->isCursed = hasCurse;
            nightVision->position = position;
            nightVision->range = 10.0f;
            items.push_back(nightVision);
        }else{
            std::shared_ptr<Item> stunner = std::make_shared<Item>("mobStunner.obj" ,metal, "mobStunner");
            stunner->size = 0.1f;
            stunner->trans = 1.0f;
            stunner->shiny = 32;
            stunner->range = 10.0f;
            stunner->position = position;
            stunner->isCursed = hasCurse;
            items.push_back(stunner);
        }
    }
    return items;
}

struct crystalLight{
    glm::vec3 diffuse;
    glm::vec3 ambient;
    glm::vec3 specular;


    float linear;
    float quadratic;
};

struct moonLight{
    glm::vec3 direction;
    glm::vec3 diffuse;
    glm::vec3 ambient;
    glm::vec3 specular;
};

struct Camera{
    glm::vec3 cameraPos{glm::vec3(0.0f , constValues::constY , 0.0f)};
    glm::vec3 cameraFront{glm::vec3(0.0f , 0.0f , -1.0f)};
    glm::vec3 cameraUp{glm::vec3(0.0f , 1.0f , 0.0f)};
};






class Accessory final : virtual public Model{
protected:
    std::string type;
    void setType(const std::string &type){
        this->type = type;
        if(this->type == Boots::name){
            this->type = Boots::name;
            validType = true;
            return;
        }
        if(this->type == Mask::name) {
            this->type = Mask::name;
            validType = true;
            return;
        }
        if(this->type == Bulb::name){
            this->type = Bulb::name;
            validType = true;
            return;
        }
        validType = false;
    }
    bool validType;
public:
    bool isUsed{false};
    float size;
    explicit Accessory(const std::string &path){
        loadModel(path);
        this->hasTexture = false;
        
    }

    explicit Accessory(const std::string &path , GLuint &texture) : Accessory(path){
        this->hasTexture = true;
        this->texture = texture;
    }

    explicit Accessory(const std::string &path , GLuint &texture , std::string const &Type) : Accessory(path , texture){
        setType(Type);
    }

    inline void playerNear(const glm::vec3 &pos) noexcept {
        const float value = (this->range ? this->range.value() : 20.0f);
        playerNearBy = checkRange(this->pos , value , pos);
    }


    inline void useOnPlayer(Player* player) noexcept {
        [[unlikely]] if(!player){ std::cout << "player is null\n"; return;}
        [[unlikely]] if(!this->validType) { std::cout << "cannot collect , since the item doesnt have a type\n"; return;}
        std::cout << "current item is used! " << this->type + "\n";
        if(this->type == Boots::name){
            std::cout << "player had " << player->getWalkingSpeed() << " walking speed and " << player->getRunningSpeed() << " running speed\n";
            player->setWalkingSpeed(player->getWalkingSpeed() + Boots::speedBuff);
            player->setRunningSpeed(player->getRunningSpeed() + Boots::speedBuff);
            this->isUsed = true;
            std::cout << "player NOW HAVE " << player->getWalkingSpeed() << " walking speed and " << player->getRunningSpeed() << " running speed\n";
            return;
        }else if(this->type == Mask::name){
            std::cout << constValues::maxRunningTime << " is the max running time ( TEMP )\n";
            constValues::maxRunningTime += Mask::durationBuff;
            this->isUsed = true;
            std::cout << constValues::maxRunningTime << " is the max running time ( NEW )\n";
            return;
        }else{
            std::cout << "LINEAR : " << player->flash.linear << " QUADRATIC : " << player->flash.quadratic << "\n";
            player->flash.linear -= percentage(Bulb::percentageBuff , player->flash.linear);
            player->flash.quadratic -= percentage(Bulb::percentageBuff , player->flash.quadratic);
            this->isUsed = true;
            std::cout << " NEW!!! LINEAR : " << player->flash.linear << " QUADRATIC : " << player->flash.quadratic << "\n";
            return;
        }
       
    }

    virtual void draw(Shader* shader) override final {
        [[unlikely]] if(!shader) return;
        if(this->isUsed) return;
        bool access{false};
        if(hasPersonalSettings) access = hasPersonalSettings.value();
        [[likely]] if(!access){
            for(auto& part : meshes){
                part->hasTexture = this->hasTexture;
                if(part->hasTexture) part->texture = this->texture;
                part->shiny = shiny;
                part->trans = trans;
                part->model = model;
                part->draw(shader);
            }
        }else [[unlikely]] {
            for(auto& part : meshes){
                part->model = model;
                part->draw(shader);
            }
        }
    }

    virtual ~Accessory() {}
};





inline void checkUsedAccessories(std::vector<std::shared_ptr<Accessory>> &accessories) noexcept {
    [[unlikely]] if(accessories.empty()) return;

    for(auto& item : accessories){
        if(!item) continue;
        if(item->isUsed){
            const auto it = std::find(accessories.begin() , accessories.end() , item);
            [[likely]] if(it != accessories.end()) accessories.erase(it);
        }
    }
}





[[nodiscard]] inline std::vector<glm::vec3> generatePosAcc ( const unsigned short &size ,const unsigned short &amount){
    std::vector<glm::vec3> positions;
    glm::vec3 currentVector;
    const float radius = 10.0f;
    const auto func = [&](const glm::vec3 &vector) -> bool {
        bool result_1 = (((vector.x + radius) < currentVector.x - radius) || ((vector.x - radius) > currentVector.x + radius)); 
        bool result_2 = ((vector.z + radius < currentVector.z - radius) || (vector.z - radius > currentVector.z + radius));
        return result_1 || result_2;
    };
    const unsigned short Coord = (unsigned short)(size * (unsigned short)(constValues::sizePlatform)) - offset;
    for(unsigned short i = 0 ; i < amount ; i++){
        const unsigned char powerFirst = rand()%2 + 1;
        const unsigned char powerSecond = rand()%2 + 1;
        currentVector.x = (float)(rand()%Coord) * (float)(pow(-1 , powerFirst));
        currentVector.y = 2.0f;
        currentVector.z = (float)(rand()%Coord) * (float)(pow(-1 , powerSecond));
        {
            bool result{false};
            for(const auto& vector : allPos){
                result = func(vector);
                if(!result) break;
            }
            if(result){
                const auto temp = currentVector;
                allPos.push_back(temp);
                positions.push_back(temp);
            }else{
                continue;
            }
        }
    }
    return positions;
}

[[nodiscard]] inline std::vector<std::shared_ptr<Accessory>> generateAcc(std::vector<glm::vec3> const &positions , GLuint &bootsTexture , GLuint &bulbTexture , GLuint maskTexture){
    std::vector<std::shared_ptr<Accessory>> items;
    for(const auto& pos : positions){
        const unsigned char index = rand()%3 + 1;
        const float radius = 20.0f;
        if(index == 1){
            std::shared_ptr<Accessory> item = std::make_shared<Accessory>("boots.obj" , bootsTexture , Boots::name);
            item->hasPersonalSettings = false;
            item->pos = pos;
            item->size = 0.5f;
            item->shiny = 16;
            item->trans = 1.0f;
            item->range = radius;
            items.push_back(item);
        }else if(index ==2){
            std::shared_ptr<Accessory> item = std::make_shared<Accessory>("mask.obj" , maskTexture , Mask::name);
            item->pos = pos;
            item->size = 0.5f;
            item->shiny = 16;
            item->trans = 1.0f;
            item->range = radius;
            items.push_back(item);
        }else{
            std::shared_ptr<Accessory> item = std::make_shared<Accessory>("bulb.obj" , bulbTexture , Bulb::name);
            item->pos = pos;
            item->size = 0.4f;
            item->shiny = 32;
            item->trans = 1.0f;
            item->range = radius;
            items.push_back(item);
        }
    }
    return items;
}

inline void setupFlash(Shader* shader , Player* user , std::string const &name) noexcept{ // shader needs to be turn on
    [[unlikely]] if(!shader) return;
    [[unlikely]] if(!user) return;
    [[unlikely]] if (name.empty()) return;
    const std::string location = name + ".";
    /*
    diffuse
    specular
    direction
    position
    glm::cos(glm::radians(cutoff / outercutoff))
    ambient
    linear
    quadratic
    */
    shader->set(location + "direction" , user->front);
    shader->set(location + "position" , user->position);
    shader->set(location + "linear" , user->flash.linear);
    shader->set(location + "quadratic" , user->flash.quadratic);

    shader->set(location + "cutoff" , glm::cos(glm::radians(user->flash.cutoff)));
    shader->set(location + "outercutoff" , glm::cos(glm::radians(user->flash.outercutoff)));

    if(user->flash.flashTurn){
        shader->set(location + "diffuse" , constValues::flashlight_diffuse);
        shader->set(location + "specular" , constValues::flashlight_specular);
        shader->set(location + "ambient" , constValues::flashlight_ambient);
    }else{
        shader->set(location + "diffuse" , constValues::off);
        shader->set(location + "specular" , constValues::off);
        shader->set(location + "ambient" , constValues::off);
    }
}

inline void setupSun(Shader* shader ,std::string const &name ,const glm::vec3 &diffuse , const glm::vec3 &specular , const glm::vec3 &ambient) noexcept {
    [[unlikely]] if(!shader) return;
    const std::string location = name + ".";
    shader->set(location + "diffuse" , diffuse);
    shader->set(location + "specular" , specular);
    shader->set(location + "ambient" , ambient);
}


[[nodiscard]] glm::mat4 calcLightSpaceMatrix(const glm::vec3 &direction , const glm::vec3 &playerPos, float radius) noexcept {
    
    glm::vec3 const center = glm::vec3(playerPos.x , 0.0f , playerPos.z);

    glm::vec3 lightPos = center - direction * radius; 
    
    glm::mat4 lightView = glm::mat4(1.0f);
    lightView = glm::lookAt(lightPos, center, glm::vec3(0.0f , 1.0f , 0.0f));
    
    glm::mat4 lightProj = glm::mat4(1.0f);
    radius *= 1.5f;
    lightProj = glm::ortho(-radius, radius, -radius, radius, 0.1f, radius + radius);
    
    return lightProj * lightView;
}


inline void setupFlower(InstanceModel* flowers1 , GLuint flowerTexture , GLuint flowerNormal , GLuint flowerDepth , GLuint centerTexture , GLuint centerNormal , GLuint centerDepth){
    if(!flowers1) throw std::runtime_error("FLOWER IS [NULL]\n");

        flowers1->personalSetting = true;
        flowers1->meshes[0]->shiny = 8;
        flowers1->meshes[0]->trans = 1.0f;
        flowers1->meshes[0]->hasTexture = true;
        flowers1->meshes[0]->texture = flowerTexture;
        flowers1->meshes[0]->normalTexture = flowerNormal;
        flowers1->meshes[0]->depthTexture = flowerDepth;

        flowers1->meshes[1]->shiny = 8;
        flowers1->meshes[1]->trans = 1.0f;
        flowers1->meshes[1]->hasTexture = true;
        flowers1->meshes[1]->texture = centerTexture;
        flowers1->meshes[1]->normalTexture = centerNormal;
        flowers1->meshes[1]->depthTexture = centerDepth;
    
}

inline void setupCrystals(Shader* shader , const std::string &name , const std::vector<glm::vec3> &positions , crystalLight &crystal){
    [[unlikely]] if(!shader) throw std::runtime_error("SHADER IS [NULL]\n");
    [[unlikely]] if(positions.empty()) throw std::runtime_error("CRYSTAL POSITION IS NULL\n");
    shader->use();
    for(unsigned short i = 0 ; i < positions.size() ; i++){
        const std::string location = name + "[" + std::to_string(i) + "].";
        shader->set(location + "diffuse" , crystal.diffuse);
        shader->set(location + "specular" , crystal.ambient);
        shader->set(location + "ambient" , crystal.ambient);
        shader->set(location + "linear" , crystal.linear);
        shader->set(location + "quadratic" , crystal.quadratic);
        shader->set(location + "position" , positions[i]);
    }
    shader->off();
}

#endif