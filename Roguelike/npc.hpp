#ifndef NPC_HPP
#define NPC_HPP


#define DefaultRadius 5.0f
#define MaxInventorySize 10
#define EOF -1
#define MaxIncreasePercentage 5
#define DefaultMobRange 30.0f


#include "objects.hpp"
#include "global.hpp"
#include <algorithm>


class NPC{
protected:
    float runningSpeed{constValues::runningSpeed};
    float walkingSpeed{constValues::walkingSpeed};
public:
    
    float yaw{-90.0f};
    float pitch{0.0f};
    float range{DefaultRadius};

    glm::vec3 position{0.0f , constValues::constY , 0.0f};
    glm::vec3 front{glm::vec3(0.0f , 0.0f , -1.0f)};
    const glm::vec3 Up{glm::vec3(0.0f , 1.0f , 0.0f)};

    
    float getRunningSpeed() const {return runningSpeed;}
    float getWalkingSpeed() const {return walkingSpeed;}
    NPC() {}
    virtual void setWalkingSpeed(const float &walkSpeed) noexcept {
        if(walkSpeed <= 3) return;
        [[unlikely]] if(walkSpeed > constValues::maxWalkingSpeed){
            walkingSpeed = constValues::maxWalkingSpeed;
        }else [[likely]] { 
            walkingSpeed = walkSpeed;
        }
    }
    virtual void setRunningSpeed(const float &runSpeed)noexcept {
        if(runSpeed <= 3) return;
        if(runSpeed > constValues::maxRunningSpeed){
            runningSpeed= constValues::maxRunningSpeed;
        }else{
            runningSpeed = runSpeed;
        }
    }
    virtual ~NPC() {}
};



class Mob : public NPC , public Model{
protected:

    Mob() = default;
    float mobSpeed = (runningSpeed + walkingSpeed) / 2.0f;
    
    const float ConstSpeed = mobSpeed;

    float tempSpeed = mobSpeed;
    const float defaultRange = 45.0f;
    float ratRange = defaultRange;

    const float ConstRatRange = ratRange;
public:
    
    Mob(std::string const &path){
        loadModel(path);
        this->hasTexture = false;
    }
    Mob(std::string const &path , const GLuint &texture) : Mob(path) {
        this->texture = texture;
        this->hasTexture = true;
    }
    bool playerInRange{false};
    std::optional<float> size;
    std::optional<glm::vec3> colorOutline;
    std::optional<bool> needsOutline;
    float currentAngle;
    bool rightAngle{false};
    std::optional<std::shared_ptr<Item>> itemOnMob;
    float range{20.0f};
    std::optional<float> attackRange;
    bool isDead{false};


    bool changedSpeed{false};

    inline friend void increaseMobsSpeed(std::vector<std::shared_ptr<Mob>> &mobs) noexcept;

    [[nodiscard]] virtual bool checkPlayerDamage(const glm::vec3 &pos) const noexcept {
        if(!attackRange) return false;
        return checkRange(this->position , this->attackRange.value() , pos);
    }

    void useItemOnMob(const std::shared_ptr<Item> &item) noexcept {
        if(!item) return;
        const std::string type = item->getType();
        if(type == "cross"){
            isDead = true;
            itemOnMob.reset();
            return;
        }
        if(type == "mobStunner"){
            if(item->isUsed){
                mobSpeed = tempSpeed;
                itemOnMob.reset();
                changedSpeed = false;
            }else{
                if(item->isCursed){
                    if(!changedSpeed) {mobSpeed += 1.5f; changedSpeed = true;}
                }else  {mobSpeed = 0.0f;}
            
            }
        }
    }

    void drawAutomaticDirection(Shader* shader , const glm::vec3 &pos , Shader* outlineShader)noexcept { // it pos should be the player pos
        bool outline = true;
        if(itemOnMob){
            if(!itemOnMob) return;
            auto tempItem = itemOnMob.value();
            useItemOnMob(tempItem);
        }
        if(!size.has_value()) size = 1.0f;
        if(isDead) return;
        if(needsOutline.has_value())  outline = needsOutline.value();
        playerNearBy = checkRange(position , range ,pos);
        if(!playerNearBy.value()){

            range = (float)DefaultMobRange;
            glm::mat4 modelTemp = glm::mat4(1.0f);
            modelTemp = glm::translate(modelTemp , position);
            modelTemp = glm::rotate(modelTemp , glm::radians(currentAngle + 69.0f) , glm::vec3(0.0f , 1.0f , 0.0f));
            modelTemp = glm::scale(modelTemp , glm::vec3(size.value()));
            model = modelTemp;
            draw(shader);
        }else{
            range = ratRange;
            const glm::vec3 currentPosition = glm::vec3(pos.x , constValues::constY , pos.z);
            if(outline && outlineShader != nullptr){
                glStencilFunc(GL_ALWAYS  , 1 , 0xFF);
                glStencilMask(0xFF);
            }
            const auto entityDirection = glm::normalize(pos - position);
            currentAngle = glm::acos(glm::dot(this->front , entityDirection)) + 69.0f;
            position += globalValues::DeltaTime * mobSpeed * entityDirection;
            glm::mat4 temp = glm::inverse(glm::lookAt(position , currentPosition , Up));
            temp = glm::rotate(temp , glm::radians(currentAngle) , glm::vec3(0.0f , 1.0f , 0.0f ));
            temp = glm::scale(temp , glm::vec3(size.value()));
            this->model = temp;
            draw(shader);
            if(outline && outlineShader != nullptr){
                shader->off();
                outlineShader->use();
                const float outlineSize = size.value() + 0.15f;
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glStencilFunc(GL_NOTEQUAL , 1 , 0xFF);
                glStencilMask(0x00);
                glm::mat4 temp_1 = glm::inverse(glm::lookAt(position , currentPosition , Up));
                temp_1 = glm::rotate(temp_1 , glm::radians(currentAngle) , glm::vec3(0.0f , 1.0f , 0.0f));
                temp_1 = glm::scale(temp_1 , glm::vec3(outlineSize));
                this->model = temp_1;
                outlineShader->set("color", colorOutline.value());
                draw(outlineShader);
                outlineShader->off();
                shader->use();
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                glStencilFunc(GL_ALWAYS , 1, 0xFF);
                glStencilMask(0x00);
                
            }
        }
    }
    virtual ~Mob() {}
};


class Player final: public NPC{

protected:

    std::array<std::shared_ptr<Item> , (std::size_t)MaxInventorySize> inventory{nullptr};
    std::array<bool , (std::size_t)MaxInventorySize> itemStatus{false};

    float tempRunSpeed{runningSpeed};
    float tempWalkingSpeed{walkingSpeed};
    const float constRange{5.0f};
public:
    Player() {inventory.fill(nullptr); itemStatus.fill(false);}
    float currentHeadShake{constValues::headShakeWhileWalking};
    FlashLight flash;
    float currentFov{constValues::defaultFov};
    int runningStamina = constValues::maxRunningTime;
    bool running{false};
    bool walking{false};
    unsigned int secondsExhaust{5};
    bool needToWaitSeconds_5{false}; 
    bool hideInventory{false};


    unsigned short currentSlot{0};

    std::optional<std::shared_ptr<Mob>> mobInRange;

    bool hasNightVision{false};
    float range{constRange};
    std::shared_ptr<Item> currentItem{nullptr};
    bool validItem{false}; // means that the slot is empty

    bool shouldDie{false};

    virtual void setWalkingSpeed(const float &walkSpeed) noexcept override final{
        [[unlikely]] if(walkSpeed == walkingSpeed) return;
        [[unlikely]] if(walkSpeed <= 0.0f) return;
        this->walkingSpeed = std::min(walkSpeed , (float)constValues::maxWalkingSpeed);
        this->tempWalkingSpeed = std::min(walkSpeed , (float)constValues::maxWalkingSpeed);
    } 

    virtual void setRunningSpeed(const float &runSpeed) noexcept  override final {
        [[unlikely]] if(runningSpeed = runSpeed) return;
        [[unlikely]] if(runSpeed <= 0.0f ) return;
        this->runningSpeed = runSpeed;
        this->tempRunSpeed = runSpeed;
    }

    void findNearestMob(std::vector<std::shared_ptr<Mob>> const &mobs) noexcept{
        if(mobs.empty()) return;
        for(const auto& mob : mobs){
            const bool result = checkRange(position , range , mob->position);
            if(result){
                mobInRange = mob;
                break;
            }
        }
    }

    void addItem(const std::shared_ptr<Item> &newItem) noexcept {
        if(!newItem) { logAction("couldnt add a item , since it null"); return; }
        if(!newItem->hasType) { logAction("cannot add item since it doesnt have a type"); return;}
        unsigned short location = 0;
        while(location != inventory.max_size()){
            if(!(itemStatus[location])){
                inventory[location] = newItem;
                itemStatus[location] = true;
                if(location == currentSlot) {currentItem = newItem; validItem = true;}
                std::cout << "adding item to the inventory POSITION [" + std::to_string(location) + "]\n";
                return;
            }
            location++;
        }
    }

    void changeCurrentItem(const unsigned int key) noexcept {
        if(currentItem){
            if(currentItem->isActive) return;
            currentItem->hasTaken = true;
        } 
        if(key > inventory.max_size()) return;
        if(!itemStatus[key]){
            validItem = false;
            currentItem = nullptr;
            currentSlot = key;
            return;
        }
        if(currentItem){
            currentItem->isActive = false;
        } // for it to not be draw
        currentItem = inventory[key];
        if(currentItem) validItem = true;
        else validItem = false;
        currentSlot = key;
    }

    [[nodiscard]] inline short getItemID(const std::shared_ptr<Item> &item) const noexcept{
        for(unsigned short i = 0 ; i < inventory.max_size() ; i++){
            if(inventory[i] == item) return i;
        }
        return (short)EOF;
    }


    inline void deleteItem(std::shared_ptr<Item> &item) noexcept {
        if(!item) return;
        const short location = getItemID(item);
        if(location == -1) return;
        if(item == currentItem){
            validItem = false;
        }
        item.reset();
        itemStatus[location] = false;
        inventory[location] = nullptr;
        
    }

    inline void checkItemActive() noexcept{
        if(!currentItem){
            validItem = false;
            return;
        } 
        if(currentItem->duration > 0 || currentItem->uses > 0){
            currentItem->isUsed = false;  // Item still has uses/duration left
        }else{
            currentItem->isUsed = true;   // Item is depleted, can be deleted
            validItem = false;
            
        }
    }

    inline void checkCurrentItemStatus() const noexcept{
        std::cout << "current location " + std::to_string(currentSlot) + "\n";
        if(!validItem){
            std::cout << "item is not valid\n";
            return;
        }else{
            std::cout << "item is valid\n";
        }

        if(!currentItem){
            std::cout << "currentItem is NULL\n";
        }else{
            std::cout << "currentItem is not null\n";
            if(currentItem->hasType){
                std::cout << "currentItem has type\n";
                std::cout << "currentItem is " << (currentItem->hasTaken ? "[ HAS BEEN TAKE ]" : "[ HASNT BEEN TAKEN ]") << std::endl;
            }else{
                std::cout << "current item does not have a type\n";
            }
        }

    }

    inline void drawCurrentItem(Shader* shader , Shader* outlineShader = nullptr) noexcept {
        if(!validItem || !currentItem || !currentItem->hasType) return;
        [[unlikely]] if(!shader) return;
        if(outlineShader && currentItem->isActive){
            glStencilFunc(GL_ALWAYS , 2 , 0xFF);
            glStencilMask(0xFF);
        }
        glm::mat4 mat = glm::mat4(1.0f);
        currentItem->hasTaken = false; // for it to be draw
        mat = glm::translate(mat , this->position);
        mat = glm::rotate(mat , glm::radians(-this->yaw - 90.0f) , glm::vec3(0.0f , 1.0f , 0.0f));
        mat = glm::rotate(mat , glm::radians(this->pitch) , glm::vec3(1.0f , 0.0f , 0.0f));
        mat = glm::translate(mat , glm::vec3(0.6f , -0.3f , -1.5f));
        mat = glm::scale(mat , glm::vec3(currentItem->size * 0.3f));
        currentItem->model = mat;
        currentItem->hasTaken = false;
        currentItem->draw(shader);
        if(currentItem->isActive){
            if(outlineShader){
                shader->off();
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glStencilFunc(GL_NOTEQUAL , 2 , 0xFF);
                glStencilMask(0x00);
                outlineShader->use();
                [[likely]] if(!currentItem->isCursed) {outlineShader->set("color" , glm::vec3(0.0f , 1.0f , 0.0f));
                }else [[unlikely]] {outlineShader->set("color" , glm::vec3(0.5f , 0.0f , 0.9f));}
                glm::mat4 temp = glm::mat4(1.0f);
                temp = glm::translate(temp , this->position);
                temp = glm::rotate(temp , glm::radians(-this->yaw - 90.0f) , glm::vec3(0.0f , 1.0f , 0.0f));
                temp = glm::rotate(temp , glm::radians(this->pitch) , glm::vec3(1.0f , 0.0f , 0.0f));
                temp = glm::translate(temp , glm::vec3(0.6f , -0.3f , -1.5f));
                temp = glm::scale(temp , glm::vec3(currentItem->size * 0.33f));
                currentItem->model = temp;
                currentItem->draw(outlineShader);
                outlineShader->off();
                shader->use();
                glStencilMask(0xFF);
                glStencilFunc(GL_ALWAYS , 0 , 0xFF);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);

            }
        }

    }

    [[nodiscard]] inline std::array<bool , 10> getItemStatus() const noexcept {return this->itemStatus;}

    inline void useItem(std::vector<std::shared_ptr<Mob>> const &mobs){
        static bool mobExists = mobs.size() == 0 ? false : true;
        if(!validItem) return;
        if(!currentItem) return;
        
        const std::string type = currentItem->getType();
        if(type == "adrenaline"){
            if(!currentItem->isUsed){
                if(!currentItem->isCursed){
                    this->runningSpeed = itemType::Adrenaline::runningSpeed;
                    this->walkingSpeed = itemType::Adrenaline::runningSpeed;
                    this->runningStamina = 30;
                    currentItem->isActive = true;
                    currentItem->uses = 0;
                }else{
                this->runningSpeed -= 3.0f;
                this->walkingSpeed -= 2.0f;
                this->runningStamina = 30;
                currentItem->isActive = true;
                currentItem->uses = 0;
                }
            }else{
                this->runningSpeed = tempRunSpeed; 
                this->runningStamina = constValues::maxRunningTime;
                this->walkingSpeed = tempWalkingSpeed;

                deleteItem(currentItem);
            }
        }else if(type == "battery"){
            if(!currentItem->isCursed) flash.duration += itemType::Battery::timePlus;
            else flash.duration -= 5;
            currentItem->isUsed = true ;
            deleteItem(currentItem);
        }else if(type == "nightVision"){
            if(!currentItem->isUsed){
                if(!currentItem->isCursed){
                    this->hasNightVision = true;
                    currentItem->isActive = true;
                    currentItem->uses = 0;
                }else{
                    this->hasNightVision = false;
                    currentItem->isActive = true;
                    currentItem->uses = 0;
                }
            }else{
                this->hasNightVision = false;
                deleteItem(currentItem);
            }
        }else if(type == "crystalExposer"){
            if(!currentItem->isUsed){
                globalValues::showCrystals = true;
                currentItem->isActive = true;
                currentItem->uses = 0;
            }else{
                globalValues::showCrystals = false;
                deleteItem(currentItem);
            }
            
        }else if(type == "cross"){
            if(!currentItem->isUsed){
                currentItem->isActive = true;
                this->range = itemType::Cross::radius;
                if(!mobInRange){
                    findNearestMob(mobs);
                }
                if(mobInRange){
                    mobInRange.value()->itemOnMob = currentItem;
                    currentItem->uses = 0;
                }else{
                    currentItem->isActive = false;
                }
            }else{
                if(mobInRange) mobInRange.reset();
                this->range = constRange;
                deleteItem(currentItem);
            }
        }else{ // the mobStunner
            if(!currentItem->isUsed){
                
                currentItem->isActive = true;
                this->range = itemType::mobStunner::radius;
                if(!mobInRange){
                    findNearestMob(mobs);
                }   
                if(mobInRange){
                    mobInRange.value()->itemOnMob = currentItem;
                    currentItem->uses = 0;
                }else{
                    currentItem->isActive = false;
                }
            }else{
                if(mobInRange) mobInRange.reset();
                this->range = constRange;
                deleteItem(currentItem);    
                
            }
        }
        checkItemActive();
    }

    inline void randomStat() noexcept{
        unsigned char situation = rand()%10 + 1;
        if(situation%3 == 0){
            std::cout << "[INCREASE] player speed by 5%\n";
            this->runningSpeed += percentage(5 , this->runningSpeed);
            this->tempRunSpeed= this->runningSpeed;
            this->walkingSpeed += percentage(5 , this->walkingSpeed);
            this->tempWalkingSpeed = this->walkingSpeed;
        }else if(situation% 10 == 0){
            std::cout << "[DECREASE] player spee by 5%\n";
            this->runningSpeed -= percentage(5 , this->runningSpeed);
            this->tempRunSpeed = this->runningSpeed;
            this->walkingSpeed -= percentage(5 , this->walkingSpeed);
            this->tempWalkingSpeed = this->walkingSpeed;
        }else if(situation % 2 == 0){
            std::cout << "[INCREASE] flash intensity\n";
            flash.linear -= percentage(5 , flash.linear);
            flash.quadratic -= percentage(5 , flash.quadratic);
        }else if(situation %4 == 0){
            std::cout << "[DECREASE] flash intensity\n";
            flash.linear += percentage(5 , flash.linear);
            flash.quadratic += percentage(5 , flash.quadratic);
        }
    }


    ~Player() {}
};

inline void checkDeadMobs(std::vector<std::shared_ptr<Mob>> &mobs) noexcept{
    if(mobs.empty()) return;
    for(unsigned short i = 0 ; i < mobs.size() ; i++){
        if(!mobs[i]){
            mobs.erase(mobs.begin()+ i);
            continue;
        }
        if(mobs[i]->isDead){
            mobs[i].reset();
            mobs.erase(mobs.begin() + i );
        }
    }
}

inline void processSecondsItem(std::vector<std::shared_ptr<Item>> &items) noexcept{
    if(items.empty()) return;
    for(const auto& item : items){
        if(item->isActive){
            item->duration--;
            if(item->duration == 0){
                item->isUsed = true;
            }else{
                item->isUsed = false;
            }
        }
    }
}

inline void checkPitchValue(float &pitch) noexcept {
    if(pitch > constValues::maxPitch) pitch = constValues::maxPitch;
    if(pitch < constValues::minPitch) pitch = constValues::minPitch;
}

std::vector<std::shared_ptr<Mob>> TempMobs;

inline void increaseMobsSpeed(std::vector<std::shared_ptr<Mob>> &mobs) noexcept {
    [[unlikely]] if(mobs.empty()) return;
    for(auto& mob : mobs){
        [[unlikely]] if(!mob) continue;
        const float value = percentage( (unsigned int)MaxIncreasePercentage, mob->ConstSpeed);
        const float rangeInc = percentage( (unsigned int)MaxIncreasePercentage , mob->ConstRatRange);
        mob->mobSpeed += value;
        mob->tempSpeed += value;
        mob->ratRange += rangeInc;
    }
}

class StatueMob final :  virtual public Mob{
protected:
    const float daySpeed = percentage(30 , constValues::walkingSpeed);
    const float nightSpeed = constValues::walkingSpeed - percentage(20 , constValues::walkingSpeed);
    StatueMob() = default;
    const float StatueRange = (float)DefaultMobRange + 5.0f;
public:
    std::optional<bool> playerDetected{false};

    StatueMob(std::string const &path) {
        loadModel(path);
        hasTexture = false;
    }
    StatueMob(std::string const &path , GLuint &texture) : StatueMob(path){
        hasTexture = true;
        this->texture = texture;
        
    }
    [[nodiscard]] bool checkPlayerDamage(const glm::vec3 &pos) const noexcept override final{
        if(!attackRange) return false;
        return checkRange(this->pos , this->attackRange.value() , pos);
    }

    inline void drawStatue(Shader* shader , Player* player , const bool &DAY) noexcept{
        [[unlikely]]if(!shader) return;
        [[unlikely]]if(!player) return;
        if(!playerDetected) {
            playerInRange = checkRange(this->pos , StatueRange , player->position);
            if(playerInRange) playerDetected = true;
            else playerDetected = false;
        }else{
            if(!playerDetected.value()){
                playerInRange = checkRange(this->pos , StatueRange , player->position);
                if(playerInRange) playerDetected = true;
                else playerDetected = false;
            }
        }
        if(!playerDetected.value()){
            // it means that the player hasnt been detected
            glm::mat4 tempModel = glm::mat4(1.0f);
            tempModel = glm::translate(tempModel , this->pos);
            tempModel = glm::rotate(tempModel , glm::radians(currentAngle) , glm::vec3(0.0f , 1.0f ,0.0f));
            tempModel = glm::scale(tempModel , glm::vec3(   (size? size.value() : 1.0f)   ));
            this->model = tempModel;
            draw(shader);
        }else{
            float currentSpeed;
            
            const float distance = findDistance(this->pos , player->position);
            const auto vectorToUser = glm::normalize(player->position - this->pos);
            const float theta = glm::dot(vectorToUser , -player->front);
            if(theta > glm::cos(glm::radians(player->flash.outercutoff)) && player->flash.flashTurn && (distance <= 40.0f)){
                currentSpeed = 0.0f;
            }else{
                if(DAY) currentSpeed = daySpeed;
                else currentSpeed = nightSpeed;
            }
            const float currentAngle = glm::acos(theta) + 90.0f;
            this->pos += currentSpeed * globalValues::DeltaTime * vectorToUser;
            this->pos.y = -1.5f;
            glm::mat4 tempModel = glm::inverse(glm::lookAt(this->pos, glm::vec3(player->position.x , -1.5f , player->position.z) , glm::vec3(0.0f , 1.0f , 0.0f)));
            tempModel = glm::rotate(tempModel , glm::radians(currentAngle) , glm::vec3(0.0f , 1.0f , 0.0f));
            tempModel = glm::scale(tempModel , glm::vec3(   (size ? size.value() : 1.0f)  ));
            this->model = tempModel;
            draw(shader);
        }
    }

};






#endif