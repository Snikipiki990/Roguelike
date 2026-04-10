#include "map.hpp"
#include <ctime>
#include "rain.hpp"
#include "shadersFile.hpp"
#include "uniformBuffer.hpp"



#define Width 1920
#define Height 1080
using namespace ShaderText;
using namespace globalValues;
using namespace constValues;
using namespace itemType;

unsigned short Timer = 900;
const float aspect = (float)Width/(float)Height;

std::unique_ptr<Player> user;


#ifdef _WIN32
#include <windows.h>
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

/*
RANGE IS FOR PLAYER IN THE RANGE OF SMTH
RADIUS IS FOR GENERATION OF OBJECTS

except for items

UPDATE: 01.04{

я сделал нормальную оптимизацию с отрисовкой через instance и создание дополнительно матриц
по итогу оно стало лучше, но теперь надо дополнительно сделать отдельный шейдер для instance

дополнительно добавил uniform buffer для нормального использования глобальных переменных среди всех возможных
шейдеров. думаю что пока что это мой максимум из-за того что я больше не знаю как это можно разбавить
}
update: 03.04 
я добавил тени, было сложно, но так же и весело 

}


*/



void framebuffer_size_callback(GLFWwindow* window , int width, int height){
    glViewport(0 , 0 , width , height);
}


void scrollCallBack(GLFWwindow* window , double xpos , double ypos){
    [[unlikely]] if(!window) return;
    if(!(user->flash.flashTurn)) return;
    user->flash.cutoff -= ypos;
    user->flash.outercutoff -=ypos;
    if(user->flash.cutoff > max_cutoff) user->flash.cutoff = max_cutoff;
    if(user->flash.cutoff < min_cutoff) user->flash.cutoff = min_cutoff;
    if(user->flash.outercutoff > max_outercutoff) user->flash.outercutoff = max_outercutoff;
    if(user->flash.outercutoff < min_outercutoff) user->flash.outercutoff = min_outercutoff;
}

void cursorCallBack(GLFWwindow* window , [[maybe_unused]] double xpos , double ypos){
    [[unlikely]] if(!window) return;
    if(firstMouse) [[unlikely]] {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }else [[likely]] {
        float xoffset = (float)(-(lastX - xpos));
        float yoffset = (float)(-(ypos - lastY));
        xoffset *= sens;
        yoffset *= sens;
        lastX = xpos;
        lastY = ypos;
        user->pitch += yoffset;
        user->yaw += xoffset;
        checkPitchValue(user->pitch);
        user->front = calcFront(user->pitch , user->yaw);
    }

}

void keyCallBack(GLFWwindow* window , int key , [[maybe_unused]] int scancode , int action ,[[maybe_unused]] int mode){
    [[unlikely]] if(!window) return;
    if((key == GLFW_KEY_F) && (action == GLFW_PRESS)){
        if(!user->flash.flashTurn){
            if(!user->flash.batteryEmpty){
                user->flash.duration--;
                user->flash.flashTurn = true;    
            }else{
                user->flash.flashTurn = false;
            }
        }else{
            user->flash.flashTurn = false;
        }
    }
    if(!user->needToWaitSeconds_5){
        if((key == GLFW_KEY_LEFT_SHIFT) && (action == GLFW_PRESS)) {
            user->running = true;
            user->currentFov = runningFov;
            user->currentHeadShake = headShakeWhileRunning;
        }        
    }

    if((key == GLFW_KEY_LEFT_SHIFT) && (action == GLFW_RELEASE)){
        user->running = false;
        user->currentFov = defaultFov;
        user->currentHeadShake = headShakeWhileWalking;
    }
    
    if((key == GLFW_KEY_H) && (action == GLFW_PRESS)){
        linear = !linear;
        std::cout << "changing linear" << std::endl;
    }
    if((key == GLFW_KEY_R) && (action == GLFW_PRESS)) COLLECT_BUTTON = true;
    if((key == GLFW_KEY_R) && (action == GLFW_RELEASE)) COLLECT_BUTTON = false;
    if((key == GLFW_KEY_J) && (action == GLFW_PRESS)) megaSpeed = !megaSpeed;
    if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(window, true);
    if((key == GLFW_KEY_1) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_1);
    if((key == GLFW_KEY_2) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_2);
    if((key == GLFW_KEY_3) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_3);
    if((key == GLFW_KEY_4) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_4);
    if((key == GLFW_KEY_5) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_5);
    if((key == GLFW_KEY_6) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_6);
    if((key == GLFW_KEY_7) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_7);
    if((key == GLFW_KEY_8) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_8);
    if((key == GLFW_KEY_9) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_9);
    if((key == GLFW_KEY_0) && (action == GLFW_PRESS)) user->changeCurrentItem(KEY_10);
    if((key == GLFW_KEY_E) && (action == GLFW_PRESS)) user->useItem(TempMobs);

    if((key == GLFW_KEY_Q) && (action == GLFW_PRESS)) user->hideInventory = !user->hideInventory;
}

void assetInput(GLFWwindow* window){
    [[unlikely]] if(!window) return;

    float currentSpeed;
    float currentHeadShake;
    static float timer = 0.0f;
    const float amplitude = 0.0f;

    user->walking  = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    
    if(user->flash.duration <= 0){
        user->flash.batteryEmpty = true;
    }else{
        user->flash.batteryEmpty = false;
    }
    if(user->needToWaitSeconds_5){
        user->running = false;
        user->currentFov = exhaustFov;
    }else{
        user->currentFov = constValues::defaultFov;
    }
    if(user->running){
        currentSpeed = DeltaTime * user->getRunningSpeed();
        currentHeadShake = DeltaTime * user->currentHeadShake;
        user->currentFov = constValues::runningFov;
    }else{
        currentSpeed = DeltaTime * user->getWalkingSpeed();
        currentHeadShake = DeltaTime * user->currentHeadShake;
        user->currentFov = constValues::defaultFov;
    }
    if(megaSpeed){
        currentSpeed = DeltaTime * (maxRunningSpeed + 5.0f);
    }
    user->front = calcFront(user->pitch , user->yaw);
    const glm::vec3 walkFront = glm::normalize(glm::vec3(user->front.x , 0.0f , user->front.z));
    const glm::vec3 right = glm::normalize(glm::cross(user->front , user->Up));

    if(glfwGetKey(window , GLFW_KEY_W) == GLFW_PRESS) user->position += walkFront * currentSpeed;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) user->position -= walkFront * currentSpeed;
    if(glfwGetKey(window , GLFW_KEY_D) == GLFW_PRESS) user->position += right * currentSpeed;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) user->position -= right * currentSpeed;

    if(user->walking){
        timer += currentHeadShake;
        user->position.y = constY + sin(timer)/2.0f;
    }else{
        user->position.y = constY;
        timer = 0.0f;
    }
}

int main(){
    std::srand(std::time(nullptr));
    std::cout << "starting the program\n";

    [[unlikely]] if(!glfwInit()){
        std::cout << "glfwInit fail"  << std::endl;
        return -1;
    }
    std::cout << "glfw init success" << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR , 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR , 2);
    glfwWindowHint(GLFW_MAXIMIZED , GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS , 24);
    glfwWindowHint(GLFW_STENCIL_BITS , 8);
    glfwWindowHint(GLFW_REFRESH_RATE , 60);
    glfwWindowHint(GLFW_SAMPLES , 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE , GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow* window = glfwCreateWindow(Width , Height , WINDOW_NAME , NULL , NULL);
    [[unlikely]] if(!window){
        std::cout  << "window is NULL" << std::endl;
        return -1;
    }
    std::cout << "window has been created" << std::endl;

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window , framebuffer_size_callback);
    glfwSetInputMode(window , GLFW_CURSOR , GLFW_CURSOR_DISABLED); 
    glfwSetCursorPosCallback(window , cursorCallBack);
    glfwSetScrollCallback(window , scrollCallBack);
    glfwSetKeyCallback(window , keyCallBack);
    // glad init 
    [[unlikely]] if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "glad init fail" << std::endl;
        glfwTerminate();
        return -1;
    }
    std::cout << "glad init success" << std::endl;

    {
        const GLubyte* renderer = glGetString(GL_RENDERER); // name of GPU 
        const GLubyte* vendor = glGetString(GL_VENDOR);     // who
        const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        std::cout << "GPU: " << renderer << std::endl;
        std::cout << "Vendor: " << vendor << std::endl;
        std::cout << "GLAD version : " << glslVersion << std::endl;
    }

    glViewport(0, 0 ,Width , Height);

    user = std::make_unique<Player>();
    std::unique_ptr<Map> game = std::make_unique<Map>(Map("settings.txt"));

    stbi_set_flip_vertically_on_load(true);



    // setup for shadows

    GLint shadowWidth = 4096 , shadowHeight = 4096;

    GLuint shadowFrame , shadowTexture;
    glGenTextures(1 , &shadowTexture);
    glBindTexture(GL_TEXTURE_2D , shadowTexture);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT , shadowWidth , shadowHeight , 0 , GL_DEPTH_COMPONENT , GL_FLOAT , NULL);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST);
    const float borderColor[] = {1.0f , 1.0f , 1.0f , 1.0f};
    glTexParameterfv(GL_TEXTURE_2D , GL_TEXTURE_BORDER_COLOR , borderColor);
    glBindTexture(GL_TEXTURE_2D , 0);

    glGenFramebuffers(1 , &shadowFrame);
    glBindFramebuffer(GL_FRAMEBUFFER , shadowFrame);
    glFramebufferTexture2D(GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_TEXTURE_2D , shadowTexture , 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "SHADOW FRAMEBUFFER [FAILED]\n";
    }else{
        std::cout << "SHADOW FRAMEBUFFER [SUCCESS]\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER , 0);

    


    // setup for multisamples
    GLuint msaaFrame , msaaRender , msaaTexture;
    glGenTextures(1 , &msaaTexture);
    glGenFramebuffers(1 , &msaaFrame);
    glGenRenderbuffers(1 , &msaaRender);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE , msaaTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE , 4 , GL_RGB , Width ,Height , GL_TRUE);
    

    glBindFramebuffer(GL_FRAMEBUFFER , msaaFrame);
    glBindRenderbuffer(GL_RENDERBUFFER , msaaRender);

    glFramebufferTexture2D(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D_MULTISAMPLE , msaaTexture , 0);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER , 4 , GL_DEPTH24_STENCIL8 , Width , Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_STENCIL_ATTACHMENT , GL_RENDERBUFFER , msaaRender);

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
    glBindRenderbuffer(GL_RENDERBUFFER , 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE , msaaTexture);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "FRAMEBUFFER [FAILED]\n";
    }else{
        std::cout << "FRAMEBUFFER [SUCCESS]\n";
    }
    

    
    
    GLuint frameBuffer , renderBuffer , texture;
    glGenTextures(1 , &texture);
    glBindTexture(GL_TEXTURE_2D , texture);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D , 0 , GL_RGB , Width , Height , 0 , GL_RGB , GL_UNSIGNED_BYTE , NULL);

    glGenFramebuffers(1 , &frameBuffer);
    glGenRenderbuffers(1 ,&renderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER , frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , texture , 0);
    glBindRenderbuffer(GL_RENDERBUFFER , renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER , GL_DEPTH24_STENCIL8 , Width ,Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER , GL_DEPTH_STENCIL_ATTACHMENT , GL_RENDERBUFFER , renderBuffer);
    [[unlikely]]if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "frame buffer [FAILED]" << std::endl;
    }else[[likely]]{
        std::cout << "frame buffer [SUCCESS]" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER , 0);
    glBindRenderbuffer(GL_RENDERBUFFER , 0);
    glBindTexture(GL_TEXTURE_2D , 0);

    GLuint dirt, log, leaves, stone, metal, grass, crystal , paper , mushroomLog , mushroomHead , mushroomSmall , logHuge , leavesHuge , UI,
    maskTexture , bulbTexture , grassNormal , stoneNormal , logNormal , leavesNormal , logDepth, stoneDepth, grassDepth , leavesDepth;
    GLuint flowerTexture , flowerDepth , flowerNormal;
    GLuint centerTexture , centerDepth , centerNormal;

    setTexture(flowerTexture , "textures/petal.png");
    setTexture(flowerNormal , "textures/petalNormal.png");
    setTexture(flowerDepth , "textures/petalDepth.png");

    setTexture(centerTexture , "textures/center.png");
    setTexture(centerNormal , "textures/centerNormal.png");
    setTexture(centerDepth , "textures/centerDepth.png");

    setTexture(leavesDepth, "textures/leavesDepth.png");
    setTexture(stoneDepth , "textures/stoneDepth.png");
    setTexture(logDepth , "textures/logDepth.png");
    setTexture(grassDepth , "textures/grassDepth.png");
    setTexture(leavesNormal , "textures/leavesNormal.png");
    setTexture(logNormal , "textures/logNormal.png");
    setTexture(stoneNormal , "textures/stoneNormal.png");
    setTexture(grassNormal , "textures/grassNormal.png");
    setTexture(dirt,    "textures/dirt.png");
    setTexture(log,     "textures/log.png");
    setTexture(leaves,  "textures/leaves.png");
    setTexture(stone,   "textures/stone.png");
    setTexture(metal,   "textures/metal.png");
    setTexture(grass,   "textures/grass.png");
    setTexture(crystal, "textures/crystal.png");
    setTexture(paper , "textures/paper.png");
    setTexture(UI , "textures/UI.png");
    setTexture(bulbTexture , "textures/glass.png");
    setTexture(maskTexture , "textures/floor.png");

    setTexture(mushroomLog , "textures/mushroom.png");
    setTexture(mushroomHead , "textures/mushroomHead.png");
    setTexture(mushroomSmall , "textures/mushroomSmall.png");

    setTexture(logHuge , "textures/logHuge.png");
    setTexture(leavesHuge , "textures/leavesHuge.png");

    stbi_set_flip_vertically_on_load(false);
    // texture set for skybox 
    const std::vector<std::string> skyBoxDay = {
        "skybox/right.jpg", "skybox/left.jpg" , "skybox/top.jpg",
        "skybox/bottom.jpg", "skybox/front.jpg" , "skybox/back.jpg"
    };
    const std::vector<std::string> skyBoxNight = {
        "skyboxNight/right.jpg", 
        "skyboxNight/left.jpg", 
        "skyboxNight/top.jpg",
        "skyboxNight/bottom.jpg", 
        "skyboxNight/front.jpg", 
        "skyboxNight/back.jpg"
    };
    GLuint skyboxDay, skyboxNight;
    setTexture(skyBoxDay , skyboxDay);
    setTexture(skyBoxNight , skyboxNight);

    std::vector<float> cube_vertices= {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        // Передняя грань
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        // Левая грань
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Правая грань
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Нижняя грань
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        // Верхняя грань
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    std::vector<unsigned short> cube_indices = {
        0, 1, 2, 2, 3, 0,       
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    std::unique_ptr<Mesh> skybox = std::make_unique<Mesh>(cube_vertices , cube_indices);

    std::vector<std::shared_ptr<Mesh>> floor;
    const glm::vec3 sizeFloor = glm::vec3((float)sizePlatform , 0.7f , (float)sizePlatform);


    for(short i = game->size * (-1) ; i < game->size ; i++){
        for(short j = game->size * (-1) ; j < game->size ; j++){
            std::shared_ptr<Mesh> temp = std::make_shared<Mesh>(cube_vertices , cube_indices , dirt);
            temp->shiny = 4.0f;
            temp->trans = 1.0f;
            glm::vec3 position;
            position.x = sizePlatform * j;
            position.y = 0.0f;
            position.z = sizePlatform * i;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , position);
            model = glm::scale(model , sizeFloor);
            temp->model = model;
            temp->normalTexture = grassNormal;
            temp->depthTexture = grassDepth;
            floor.push_back(temp);
        }
    }
    // crystals
    std::vector<glm::vec3> crystalPositions = generatePos(game->size , crystalAmount , 40.0f , 3.0f);
    std::vector<std::shared_ptr<Model>> crystals;
    for(unsigned short i = 0; i < crystalAmount ; i++){
        std::shared_ptr<Model> Crystal = std::make_shared<Model>("crystal.obj" , crystal);
        Crystal->shiny = 64;
        Crystal->trans = 1.0f;
        Crystal->radius = 40.0f;
        Crystal->range = 10.0f;
        Crystal->ID = i;
        Crystal->pos = crystalPositions[i];
        crystals.push_back(Crystal);
    }
    



    // items
    std::vector<glm::vec3> itemPos = generatePos(game->size , game->itemAmount , 40.0f , 2.0f);
    std::vector<std::shared_ptr<Item>> items = generateItems(itemPos , crystal , log , metal);





    // statues
    std::vector<glm::vec3> mobStatuePos = generatePos(game->size , 10 , 20.0f , -1.5f);
    std::vector<std::shared_ptr<StatueMob>> statues;
    for(const auto& position : mobStatuePos){
        std::shared_ptr<StatueMob> statue = std::make_shared<StatueMob>("mobStatue.obj" , stone);
        statue->size = 1.4f;
        statue->shiny = 32;
        statue->trans = 1.0f;
        statue->attackRange = 8.0f;
        statue->pos = position;
        statue->position = position;
        statues.push_back(statue);
    }


    // structures
    std::vector<std::shared_ptr<Model>> structures;
    for(unsigned short i = 0 ; i < 10 ; i++){
        unsigned short index = rand()%2 + 1;
        std::shared_ptr<Model> temp;
        if(index == 1){
            temp = std::make_shared<Model>("crossStructure.obj" , stone);
        }else{
            temp = std::make_shared<Model>("statue.obj" , stone);
        }
        temp->radius = 40.0f;
        temp->shiny = 32;
        temp->trans = 1.0f;
        temp->hasPersonalSettings = false;
        temp->ID = i;
        structures.push_back(temp);
    }
    std::vector<glm::vec3> structuresPos = generatePos(game->size , structures.size() , structures[0]->radius , 0.7f);





    std::vector<glm::vec3> AccessoriesPos = generatePosAcc(game->size , 10);
    std::vector<std::shared_ptr<Accessory>> accessories = generateAcc(AccessoriesPos , paper , bulbTexture , maskTexture);






    // mobs
    const std::vector<glm::vec3> mobsPos = generatePos(game->size , game->mobAmount , 10.0f , constY);
    std::vector<std::shared_ptr<Mob>> mobs;

    for(unsigned short i = 0 ; i < game->mobAmount ; i++){
        std::shared_ptr<Mob> mob = std::make_shared<Mob>("ratt.obj" , paper);
        mob->hasPersonalSettings = false;
        mob->size = 1.5f;
        mob->trans = 1.0f;
        mob->colorOutline = glm::vec3(0.6f , 0.0f , 0.0f);
        mob->needsOutline = true;
        mob->ID = i;
        mob->range = 30.0f;
        mob->attackRange = 7.0f;
        mob->radius = 10.0f;
        mob->position = mobsPos[i];
        mobs.push_back(mob);
    }

    TempMobs = mobs;

    





    // tree huge
    const std::vector<glm::vec3> treeHugePos  = generatePos(game->size , 50 , 20.0f , -1.0f);
    std::vector<glm::mat4> Martrixs_Htree;
    // instance
    {   
        unsigned short i = 0;
        for(const auto& pos : treeHugePos){
            glm::mat4 model= glm::mat4(1.0f);
            const float angle = (float)(i++) * 20.0f;
            model = glm::translate(model , pos);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f, 0.0f));
            model = glm::scale(model , glm::vec3(5.0f , 8.0f, 5.0f ));
            Martrixs_Htree.push_back(model);
        }
    }
    std::shared_ptr<InstanceModel> treeHuge = std::make_shared<InstanceModel>("tree_2.obj" , Martrixs_Htree);
    {
        treeHuge->personalSetting = true;
        treeHuge->hasTexture = true;

        treeHuge->meshes[0]->hasTexture = true;
        treeHuge->meshes[0]->texture = logHuge;
        treeHuge->meshes[0]->trans = 1.0f;
        treeHuge->meshes[0]->shiny = 4;

        treeHuge->meshes[1]->hasTexture == true;
        treeHuge->meshes[1]->texture = leaves;
        treeHuge->meshes[1]->trans = 1.0f;
        treeHuge->meshes[1]->shiny = 16;
    }





    // small mushroom
    std::vector<glm::mat4> Matrix_Smushroom;
    const std::vector<glm::vec3> smallMushroomPos = generatePos(game->size , game->rockAmount , 20.0f , -1.0f);
    // instance 
    {
        for(unsigned short i = 0 ; i < smallMushroomPos.size() ; i++){
            const float angle = (float)i * 20.0f;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , smallMushroomPos[i]);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
            model = glm::scale(model , glm::vec3(1.2f));
            Matrix_Smushroom.push_back(model);
        }
    }
    std::shared_ptr<InstanceModel> smallMushroom = std::make_shared<InstanceModel>("smallMushroom.obj" , Matrix_Smushroom);
    {
        smallMushroom->personalSetting = true;
        smallMushroom->hasTexture = true;
        smallMushroom->meshes[0]->hasTexture = true;
        smallMushroom->meshes[0]->texture = mushroomLog;
        smallMushroom->meshes[0]->trans = 1.0f;
        smallMushroom->meshes[0]->shiny = 16;

        smallMushroom->meshes[1]->hasTexture = true;
        smallMushroom->meshes[1]->texture = mushroomSmall;
        smallMushroom->meshes[1]->trans = 1.0f;
        smallMushroom->meshes[1]->shiny = 16;
    }







    // trees
    const std::vector<glm::vec3> treePositions = generatePos(game->size , game->treeAmount , 5.0f , 1.0f);
    // instance
    std::vector<glm::mat4> treeMatrix;
    {
        for(unsigned short i = 0 ; i < treePositions.size() ; i++){
            const float angle = (float)i * 20.0f;
            const float size = 1.0f + pow(sin(angle) , 2);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , treePositions[i]);
                model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
                
                model = glm::scale(model , glm::vec3(size));
                treeMatrix.push_back(model);
        }
    }
    
    
    std::shared_ptr<InstanceModel> tree = std::make_shared<InstanceModel>("tree.obj" , treeMatrix);
    {
        tree->personalSetting = true;
        tree->meshes[0]->hasTexture = true;
        tree->meshes[0]->shiny = 4;
        tree->meshes[0]->trans = 1.0f;
        tree->meshes[0]->texture = log;
        tree->meshes[0]->normalTexture = logNormal;
        tree->meshes[0]->depthTexture = logDepth;
        
        tree->meshes[1]->hasTexture = true;
        tree->meshes[1]->shiny = 16;
        tree->meshes[1]->trans = 1.0f;
        tree->meshes[1]->texture = leaves;
        tree->meshes[1]->normalTexture = leavesNormal;
        tree->meshes[1]->depthTexture = leavesDepth;
    
    }
    



    // ---- stone -----
    const std::vector<glm::vec3> rocksPos = generatePos(game->size , game->rockAmount , 4.0f , 0.5f);
    std::vector<glm::mat4> stoneMatrix;
    // instance
    {
        for(unsigned short i = 0 ; i < rocksPos.size() ; i++){
            const float angle = (float)i * 20.0f;
            const float size{(float)pow(cos(angle) ,2)/2.0f + 0.4f};
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , rocksPos[i]);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.3f));
            model = glm::scale(model , glm::vec3(size));
            stoneMatrix.push_back(model);
        }
    }
    
    // rocks
    std::shared_ptr<InstanceModel> Stone = std::make_shared<InstanceModel>("stone.obj" , stoneMatrix ,stone);
    {
        Stone->trans = 1.0f;
        Stone->shiny = 16;
        Stone->normalTexture = stoneNormal;
        Stone->depthTexture = stoneDepth;
    }


    // flowers setup
    const std::vector<glm::vec3> flowersPos = generatePos(game->size , 200 , 10.0f , -1.0f);
    std::vector<glm::mat4> flowerMatrix;
    for(unsigned short i = 0 ; i < flowersPos.size() ; i++){
        const float angle = (float)i * 35.0f;
        const float size = (float)pow(sin(angle) , 2)*2.0 + 4.0f; 
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model , flowersPos[i]);
        model = glm::rotate(model  , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0));
        model = glm::scale(model , glm::vec3(size));
        flowerMatrix.push_back(model);
    }
    std::shared_ptr<InstanceModel> flowers = std::make_shared<InstanceModel>("flowers.obj" , flowerMatrix);
    try{
        setupFlower(flowers.get() , flowerTexture , flowerNormal , flowerDepth , centerTexture , centerNormal , centerDepth);
    }
    catch(std::exception &e){
        std::cout << "FLOWER ERROR:" << e.what() <<"\n";
    }
    // 2nd flower

    std::vector<glm::mat4> flower2Matrix;
    for(unsigned short i = 0 ; i < flowersPos.size() ; i++){
        const float x = (float)(rand()%6) + 2.0f;
        const float z = (float)(rand()%6) + 2.0f;
        const float angle = (float)i * 2.0f * 20.0f;
        const float size = (float)pow(cos(angle) , 2) * 1.5f + 4.0f;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model , glm::vec3(flowersPos[i] + glm::vec3(x, 0.0f , z)));
        model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f,  1.0f , 0.0f));
        model = glm::scale(model , glm::vec3(size));
        flower2Matrix.push_back(model);
    }
    std::shared_ptr<InstanceModel> flowers1 = std::make_shared<InstanceModel>("flowers1.obj" , flower2Matrix);
    try{
        setupFlower(flowers1.get() , flowerTexture , flowerNormal , flowerDepth , centerTexture , centerNormal , centerDepth);
    }
    catch(std::exception &e){
        std::cout << "FLOWER ERROR:" << e.what() <<"\n";
    }
    //3rd flower
    std::vector<glm::mat4> flower3Matrix;
    for(unsigned short i = 0 ; i < flower2Matrix.size() ; i++){
        const float x = pow(-1.0f , i)*(float)(rand()%6) + 2.0f;
        const float z = (float)(rand()%6) + 2.0f;
        const glm::vec3 position = glm::vec3(x , 0.0f , z) + flowersPos[i]; 
        const float angle = (float)i * 2.0f * 20.0f;
        const float size = (float)pow(cos(angle) , 2) * 1.5f + 5.0f;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model , position);
        model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
        model = glm::scale(model , glm::vec3(size));
        flower3Matrix.push_back(model);
    }
    std::shared_ptr<InstanceModel> flowers2 = std::make_shared<InstanceModel>("flowers2.obj" , flower3Matrix);
    try{
        setupFlower(flowers2.get() , flowerTexture , flowerNormal , flowerDepth , centerTexture , centerNormal , centerDepth);
    }
    catch(std::exception &e){
        std::cout << "FLOWER ERROR:" << e.what() <<"\n";
    }


    // grass
    const std::vector<glm::vec3> grassPos = generatePos(game->size , 350 , 4.0f , 0.3f);
    std::vector<glm::mat4> grassMatrix;
    // instance
    {
        for(unsigned short i = 0 ; i < grassPos.size() ; i++){
            const float angle = (float)i * 20.0f;
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , grassPos[i]);
                model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
                model = glm::scale(model , glm::vec3(6.0f));
                grassMatrix.push_back(model);
        }
    }
    std::shared_ptr<InstanceModel> Grass = std::make_shared<InstanceModel>("grass.obj" , grassMatrix ,grass);
    {
        Grass->shiny = 4;
        Grass->trans = 1.0f;
    }
    





    //  ---- mushroom looking like a tree
    const std::vector<glm::vec3> mushroomTreePos = generatePos(game->size , 200 , 10.0f , -3.0f);
    
    // instance 
    std::vector<glm::mat4> Matrix_Tmushroom;
    {
        for(unsigned short i = 0 ; i < mushroomTreePos.size() ; i++){
            const float angle = (float)i * 20.0f;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , mushroomTreePos[i]);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
            model = glm::scale(model , glm::vec3(5.0f , 5.0f + pow(cos(glm::radians((float)i * 20.0f)) , 2) * 4.0f , 5.0f ));
            Matrix_Tmushroom.push_back(model);
        }
    }
    std::shared_ptr<InstanceModel> mushroomTree = std::make_shared<InstanceModel>("mushroomTree.obj" , Matrix_Tmushroom);
    // set for mushroom looking like tree
    {
        mushroomTree->personalSetting = true;
        mushroomTree->hasTexture = true;
        mushroomTree->meshes[0]->texture = paper;
        mushroomTree->meshes[0]->shiny = 16;
        mushroomTree->meshes[0]->trans = 1.0f;
        mushroomTree->meshes[0]->hasTexture = true;

        mushroomTree->meshes[1]->texture = mushroomHead;
        mushroomTree->meshes[1]->shiny = 16;
        mushroomTree->meshes[1]->trans = 1.0f;
        mushroomTree->meshes[1]->hasTexture = true;
    }


    allPos.clear();


    
    // setup for framebuffer 
    float quadVertices[] = {
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  
        1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f   
    };

    unsigned short quadIndices[] = {
        0, 1, 2,  
        2, 3, 0   
    };
    GLuint VBO , EBO , VAO;
    
    glGenBuffers(1 , &VBO);
    glGenBuffers( 1, &EBO);
    glGenVertexArrays(1 , &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER , VBO);    
    glBufferData(GL_ARRAY_BUFFER , sizeof(quadVertices) , quadVertices , GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(quadIndices) , quadIndices , GL_STATIC_DRAW);
    
    glVertexAttribPointer( 0 ,3 , GL_FLOAT , GL_FALSE , 5 * sizeof(float) , (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1 , 2 , GL_FLOAT , GL_FALSE , 5 * sizeof(float) , (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER , 0);


    std::unique_ptr<Rain> rainSystem = std::make_unique<Rain>(200);
    rainSystem->raining = true;
    rainSystem->duration = constValues::rainDuration;

    std::vector<std::shared_ptr<Model>> crystalsToRemove;


    // ------ SHADERS -------

    // shaders for 3D models all types / main one / outline + depth test / outline without depth test
    std::shared_ptr<Shader> mainShader = std::make_shared<Shader>(vertexSource , fragmentSource);
    std::shared_ptr<Shader> singleColorShader = std::make_shared<Shader>(vertexSource , crystalFragment);
    std::shared_ptr<Shader> outlineShader = std::make_shared<Shader>(vertexSource , fragmentSoloColor);
    std::shared_ptr<Shader> instanceShader = std::make_shared<Shader>(instanceVertex , fragmentSource);
    std::unique_ptr<Shader> normalShader = std::make_unique<Shader>(normalVertexSource , geometryNormalSource , normalFragmentSource);
    std::unique_ptr<Shader> instanceNormalShader = std::make_unique<Shader>(instanceNormalVertexSource , geometryNormalSource , normalFragmentSource);


    //  common screen shader , cannot use inventory, since it lacks the needed sampler2D
    std::shared_ptr<Shader> screenShader = std::make_shared<Shader>(vertexScreen , fragmentScreen);
    std::shared_ptr<Shader> screenNightVision = std::make_shared<Shader>(vertexScreen , nightVisionFragment);


    
    // both of these are used for inventory, which is 2D
    std::shared_ptr<Shader> inventoryShader = std::make_shared<Shader>(vertexInventory ,fragmentInventory);
    std::unique_ptr<Shader> inventorySquare = std::make_unique<Shader>(vertexInventory , fragmentSquareInventory);
    std::unique_ptr<Shader> inventoryTexture = std::make_unique<Shader>(vertexInventory , inventoryFragmentTexture);


    // obvious for rain
    std::unique_ptr<Shader> rainShader = std::make_unique<Shader>(rainVertex , rainFragment);

    // skybox shader
    std::unique_ptr<Shader> skyboxShader = std::make_unique<Shader>(skyBoxVertex , skyBoxFragment);
    

    // shader for 
    std::unique_ptr<Shader> shadowShader = std::make_unique<Shader>(shadowVertex , shadowFragment);
    std::unique_ptr<Shader> shadowInstance = std::make_unique<Shader>(shadowInstanceVertex , shadowFragment);


    // ---- UNIFORM BUFFERS ----
    auto mainMatrix = std::make_unique<UniformBuffer>( (GLuint)0 , (sizeof(glm::mat4) * 2));
    auto fogSetting = std::make_unique<UniformBuffer>((GLuint)1 , std::size_t(32));
    auto shadowSetting = std::make_unique<UniformBuffer>((GLuint)2 , sizeof(glm::mat4));
    auto main2D = std::make_unique<UniformBuffer>((GLuint)3 , sizeof(glm::mat4) * 2);
    // ---- ---- ---- ----

    // default setting since it wont change at all 
    main2D->addData(sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(glm::mat4(1.0f)));
    {
        glm::mat4 proj = glm::ortho(-aspect , aspect , -1.0f , 1.0f);
        main2D->addData(0 , sizeof(glm::mat4) , glm::value_ptr(proj));
    
    }

    user->flash.linear = 0.008f;
    user->flash.quadratic = 0.001f;

    crystalLight Crystals;
    Crystals.diffuse = crystalDiffuse;
    Crystals.ambient = crystalAmbient;
    Crystals.linear = 0.05f;
    Crystals.quadratic = 0.02f;
    Crystals.specular = crystalSpecular;
    
    // first set will be for mainShader
    try{
        setupCrystals(mainShader.get() , "crystal" , crystalPositions , Crystals);
        setupCrystals(instanceShader.get() , "crystal" , crystalPositions,  Crystals);
        setupCrystals(normalShader.get() , "crystal" , crystalPositions , Crystals);
        setupCrystals(instanceNormalShader.get() , "crystal" , crystalPositions , Crystals);
    }
    catch(std::exception &e){
        std::cout << "CRYSTAL SETUP ERROR: " << e.what() << "\n";
    }

    // outline color setup 
    singleColorShader->use();
    singleColorShader->set("color" , crystalDiffuse);
    singleColorShader->off();


    // skybox setup

    skyboxShader->linear = false;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP , GL_KEEP , GL_REPLACE);
    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW);
    /*
    hello everyone today we will do the simple task 
    
    
    */

    const float CurrentNear = 0.1f;

    float lastSecond = 0.0f;
    user->flash.duration = constValues::maxSecondsFlashLight;

    std::array<bool , 10> tempStatusItem;
    tempStatusItem.fill(false);

    
    


    // main cycle 
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        DeltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        

        // system for calculating the night / day colors
        const float factor = pow(cos(glm::radians(currentFrame) / 5.0f) , 2);
        
        if(factor > 0.5f){ DAY = true ; NIGHT = false;}
        else{DAY = false ; NIGHT = true;}


        const auto currentDiffuse = glm::mix(moon_diffuse , sun_diffuse , factor);
        const auto currentAmbient = glm::mix(moon_ambient , sun_ambient , factor);
        const auto currentSpecular = glm::mix(moon_specular, sun_specular , factor);
        const auto currentFogColor = glm::mix(fogColor , fogColor_day , factor);

        // sun direction calculation
        const float t = pow(cos(glm::radians(currentFrame / 5.0f)), 2); 
        const float angle = glm::mix(0.0f, glm::radians(180.0f), t); 

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::rotate(sunModel, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        sunModel = glm::translate(sunModel, glm::vec3(50.0f, 0.0f, 0.0f));


        const auto sunDirection = glm::normalize(-glm::vec3(sunModel[3]));

        const glm::mat4 lightSpaceMatrix = calcLightSpaceMatrix(sunDirection , user->position , 200.0f);
        
        shadowSetting->addData(0 , sizeof(glm::mat4) , glm::value_ptr(lightSpaceMatrix));

        mainShader->use();
        mainShader->set("sun.direction" , sunDirection);
        mainShader->off();

        instanceShader->use();
        instanceShader->set("sun.direction" , sunDirection);
        instanceShader->off();

        normalShader->use();
        normalShader->set("sun.direction" , sunDirection);
        normalShader->off();

        instanceNormalShader->use();
        instanceNormalShader->set("sun.direction" , sunDirection);
        instanceNormalShader->off();


        assetInput(window);

        mainShader->linear = linear;
        outlineShader->linear = linear;
        singleColorShader->linear = linear;
        instanceShader->linear = linear;
        normalShader->linear = linear;
        instanceNormalShader->linear = linear;

        // current view and projection 
        glm::mat4 proj = glm::mat4(1.0f);
        if(linear) proj = glm::perspective(glm::radians(user->currentFov) , (float)Width/(float)Height , CurrentNear , far_linear);
        else proj =glm::perspective(glm::radians(user->currentFov) , (float)Width/(float)Height , CurrentNear , far_notLinear);
        glm::mat4 view = glm::lookAt(user->position , user->position + user->front , user->Up);




        // data setup
        mainMatrix->addData(0 , sizeof(glm::mat4) , glm::value_ptr(proj));
        mainMatrix->addData(sizeof(glm::mat4) , sizeof(glm::mat4) , glm::value_ptr(view));

        const auto currentFar = (linear ? far_linear : far_notLinear);

        fogSetting->addData(0  , sizeof(float) , &currentFar);
        fogSetting->addData(sizeof(float) , sizeof(float) , &CurrentNear);
        fogSetting->addData(16 , sizeof(glm::vec3) , glm::value_ptr(currentFogColor));

        // -------------------------------------
        




        // SHADOW SETUP
        glBindFramebuffer(GL_FRAMEBUFFER , shadowFrame);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0 , shadowWidth,  shadowHeight);
        glCullFace(GL_FRONT);
        
        glFrontFace(GL_CCW);

        shadowInstance->use();

        tree->draw(shadowInstance.get());
        Grass->draw(shadowInstance.get());
        Stone->draw(shadowInstance.get());
        mushroomTree->draw(shadowInstance.get());
        treeHuge->draw(shadowInstance.get());
        smallMushroom->draw(shadowInstance.get());
        flowers->draw(shadowInstance.get());
        flowers1->draw(shadowInstance.get());
        flowers2->draw(shadowInstance.get());
        shadowInstance->off();

        

        shadowShader->use();

        for(auto& temp : structures){
            const float angle = 20.0f * temp->ID;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , structuresPos[temp->ID]);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
            model = glm::scale(model , glm::vec3(1.2f));
            temp->model = model;
            temp->draw(shadowShader.get());
        }
    

        for(auto& temp : items){
            if(!temp) continue;
            if(temp->hasTaken) continue;
            if(!temp->hasType) continue;
            if(user->currentItem == temp) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , temp->position);
            model = glm::scale(model , glm::vec3(temp->size));
            temp->model = model;
            temp->draw(shadowShader.get());
        }
        
        {
            unsigned short i = 0;
            for(auto& item : accessories){
                const float angle = (float)(i++) * 20.0f;
                if(!item) continue;
                if(item->isUsed) continue;
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , item->pos);
                model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f, 0.0f));
                model = glm::scale(model , glm::vec3(item->size));
                item->model = model;
                item->draw(shadowShader.get());   
            }
        }

        for(auto& statue : statues){
            [[unlikely]] if(!statue) continue;
            statue->drawStatue(shadowShader.get() , user.get() , DAY);
        }

        glFrontFace(GL_CW);
        for(auto& temp : floor){
            temp->draw(shadowShader.get());
        }
        glFrontFace(GL_CCW);

        shadowShader->off();

        glBindFramebuffer(GL_FRAMEBUFFER , 0);
        glViewport(0 , 0 , Width , Height);
        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER , msaaFrame);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D , shadowTexture);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS , 0 , 0xFF);
        glClearStencil(0);
        if(!linear){ 
            glClearColor(currentFogColor.r ,  currentFogColor.g , currentFogColor.b , 1.0f);
        }
        else {
            glClearColor(0.6f, 0.6f , 0.6f , 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // SKYBOX 
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        skyboxShader->use();
        skyboxShader->set("factor" , factor);
        skyboxShader->set("fogColor" , currentFogColor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP , skyboxDay);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP , skyboxNight);
        skybox->draw(skyboxShader.get());
        skyboxShader->off();
        // -------------------------------------------------------

        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        // rain system

        if(rainSystem->raining){
            rainShader->use();
            rainSystem->draw(user->position , rainShader.get());
            rainShader->off();
        }
        instanceShader->use();

        if(!linear){
            setupSun(instanceShader.get() , "sun" , currentDiffuse , currentSpecular , currentAmbient);
        } else {
            setupSun(instanceShader.get() , "sun" , on , on , on);
        }
        instanceShader->set("cameraPos" , user->position);

        if(user->hasNightVision){
            setupSun(instanceShader.get() , "sun" , on , on , on);
        }
        
        setupFlash(instanceShader.get() , user.get() , "flash");

        glFrontFace(GL_CCW);
        // huge tree
        treeHuge->draw(instanceShader.get());

        // small mushroom
        smallMushroom->draw(instanceShader.get());

        // grass
        Grass->draw(instanceShader.get());
        // mushroom tree
        mushroomTree->draw(instanceShader.get());
        instanceShader->off();


            instanceNormalShader->use();



        setupFlash(instanceNormalShader.get(), user.get(), "flash");
        if(!linear){
            setupSun(instanceNormalShader.get(), "sun", currentDiffuse, currentSpecular, currentAmbient);
        }else{
            setupSun(instanceNormalShader.get(), "sun", on, on, on);
        }
        if(user->hasNightVision) 
            setupSun(instanceNormalShader.get(), "sun", on, on, on);


        tree->draw(instanceNormalShader.get());

        Stone->draw(instanceNormalShader.get());
        
        flowers->draw(instanceNormalShader.get());

        flowers1->draw(instanceNormalShader.get());

        flowers2->draw(instanceNormalShader.get());

            instanceNormalShader->off();


        // the floor
        normalShader->use();
        setupFlash(normalShader.get() , user.get() , "flash");
        if(!linear){
            setupSun(normalShader.get() , "sun" , currentDiffuse , currentSpecular , currentAmbient);
        }else{
            setupSun(normalShader.get() , "sun" , on , on , on);
        }
        if(user->hasNightVision) setupSun(normalShader.get() , "sun" , on , on ,on);

        glFrontFace(GL_CW);
        for(auto& temp : floor){
            temp->draw(normalShader.get());
        }
        glFrontFace(GL_CCW);

        normalShader->off();



        


        // common set for mainShader with lights
        mainShader->use();

        if(!linear){
            setupSun(mainShader.get() , "sun" , currentDiffuse , currentSpecular , currentAmbient);
        }else{
            setupSun(mainShader.get() , "sun" , on , on , on);
        }

        mainShader->set("cameraPos" , user->position);

        if(user->hasNightVision){
            setupSun(mainShader.get() , "sun" , on , on , on);
        }

        setupFlash(mainShader.get() , user.get() , "flash");
        // structures;
        for(auto& temp : structures){
            const float angle = 20.0f * temp->ID;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , structuresPos[temp->ID]);
            model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f , 0.0f));
            model = glm::scale(model , glm::vec3(1.2f));
            temp->model = model;
            temp->draw(mainShader.get());
        }

        
        mainShader->off();

        
        // ---- crystals ----
        singleColorShader->use();
        for(auto& temp : crystals){
            [[unlikely]] if(!temp) continue;
            const float angle = glfwGetTime();
            const float newY = cos(glm::radians(angle) * 20.0f);
            const glm::vec3 location = glm::vec3(temp->pos.x ,
                                                temp->pos.y + newY + 2.0f ,
                                                temp->pos.z);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , location);
            model = glm::rotate(model , glm::radians(angle * 5.0f) , glm::vec3(0.0f , 1.0f , 0.0f));
            model = glm::scale(model , glm::vec3(0.5f));
            temp->model = model;
            
            
            float distance = findDistance(temp->pos , user->position);
            temp->playerNearBy = false;
            if(distance < 20.0f){
                temp->playerNearBy = true;
            }else{
                temp->playerNearBy = false;
            }
            distance = glm::smoothstep(60.0f , 15.0f , distance);
            distance = std::max(0.1f , distance);
            singleColorShader->set("color" , glm::vec3(distance * crystalDiffuse));
            temp->draw(singleColorShader.get());
            if(!temp->playerNearBy) continue;
            if(temp->playerNearBy.value() && COLLECT_BUTTON){
                std::cout << findDistance(temp->pos , user->position) << std::endl;
                user->randomStat();
                increaseMobsSpeed(TempMobs);
                singleColorShader->off();
                mainShader->use();
                const std::string name = "crystal[" + std::to_string(temp->ID) + "].";
                mainShader->set(name + "diffuse" , off);
                mainShader->set(name + "specular" , off);
                mainShader->set(name + "ambient" , off);
                std::cout << "crystal " << temp->ID << " collected\n";
                crystalsToRemove.push_back(temp);
                crystalCollected++;
                COLLECT_BUTTON = false;
                user->flash.duration += 20;
                mainShader->off();
                instanceShader->use();
                instanceShader->set(name + "diffuse" , off);
                instanceShader->set(name + "specular" , off);
                instanceShader->set(name + "ambient" , off);
                instanceShader->off();
                normalShader->use();
                normalShader->set(name + "diffuse" , off);
                normalShader->set(name + "specular" , off);
                normalShader->set(name + "ambient" , off);
                normalShader->off();
                instanceNormalShader->use();
                instanceNormalShader->set(name + "diffuse" , off);
                instanceNormalShader->set(name + "specular" , off);
                instanceNormalShader->set(name + "ambient" , off);
                instanceNormalShader->off();
                singleColorShader->use();
            }
        }
        
        // Remove collected crystals after iteration
        for(const auto& crystal : crystalsToRemove){
            const auto it = std::find(crystals.begin() , crystals.end() , crystal);
            if(it != crystals.end()){
                crystals.erase(it);
            }
        }
        
        singleColorShader->off();


            mainShader->use();
        // items

        for(auto& temp : items){
            if(!temp) continue;
            if(temp->hasTaken) continue;
            if(!temp->hasType) continue;
            if(user->currentItem == temp) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model , temp->position);
            model = glm::scale(model , glm::vec3(temp->size));
            temp->model = model;
            temp->checkPlayerNear(user->position);
            temp->draw(mainShader.get());
            if(temp->playerNearBy.value() && COLLECT_BUTTON){
                COLLECT_BUTTON = false;
                temp->hasTaken = true;
                user->addItem(temp);
                temp->playerNearBy = false;
            }
        }

        // accessories
        {
            unsigned short i = 0;
            for(auto& item : accessories){
                const float angle = (float)(i++) * 20.0f;
                if(!item) continue;
                if(item->isUsed) continue;
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , item->pos);
                model = glm::rotate(model , glm::radians(angle) , glm::vec3(0.0f , 1.0f, 0.0f));
                model = glm::scale(model , glm::vec3(item->size));
                item->model = model;
                item->playerNear(user->position);
                item->draw(mainShader.get());
                if(item->playerNearBy.value() && COLLECT_BUTTON){
                    COLLECT_BUTTON = false;
                    item->useOnPlayer(user.get());
                }
            }

            checkUsedAccessories(accessories);
        }



        // current items
        user->drawCurrentItem(mainShader.get() , outlineShader.get());
        // if user flash duration is greater than max set , it is possible and does not count as a bug 
        [[unlikely]]if(user->flash.duration > user->flash.maxDuration) user->flash.maxDuration = user->flash.duration;
        // statues
        for(auto& statue : statues){
            [[unlikely]] if(!statue) continue;
            statue->drawStatue(mainShader.get() , user.get() , DAY);
            user->shouldDie = statue->checkPlayerDamage(user->position);
            [[unlikely]] if(user->shouldDie) {
                glfwSetWindowShouldClose(window ,true);
                std::cout << "player is dead by statue\n";
            }
        }

        // mobs
        for(auto& temp : mobs){
            [[unlikely]] if(!temp) continue;
            temp->drawAutomaticDirection(mainShader.get(), user->position , outlineShader.get());
            user->shouldDie = temp->checkPlayerDamage(user->position);
            [[unlikely]] if(user->shouldDie){
                glfwSetWindowShouldClose(window , true);
                std::cout << "player is dead by rat\n";
            }
        }
        mainShader->off();

        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);
        tempStatusItem = user->getItemStatus();
        glBindVertexArray(VAO);


        // drawing the inventory
        
        
        if(!user->hideInventory){
            inventorySquare->use();
            {
                // white back
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model , glm::vec3(0.0f , -0.925f, 0.0f));
                    model = glm::scale(model , glm::vec3(1.31f , 0.21f , 0.0f));
                    inventorySquare->set("color" , glm::vec3(0.2f));
                    inventorySquare->set("model", model);
                    glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
                }
            inventorySquare->off();
                // black front 
            inventoryTexture->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D , UI);
                { 
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model , glm::vec3( 0.0f , -0.925f , 0.0f));
                    model = glm::scale(model , glm::vec3(1.3f , 0.2f , 0.0f));
                    inventoryTexture->set("model" , model);
                    glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
                }
                glBindTexture(GL_TEXTURE_2D , 0);
            }
            inventoryTexture->off();

            inventoryShader->use();
            tempStatusItem = user->getItemStatus();
            
            // setup for inventory
            {

                for(unsigned short i = 0 ; i < MaxInventorySize ; i++){
                    const float position = -1.16f + (float)i * 0.26f;
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model , glm::vec3(position , -0.9f , 0.01f));
                    /*
                    
                    */
                    if(i == user->currentSlot){
                        model = glm::scale(model , glm::vec3(0.1f));
                        if(user->currentItem){
                            if(user->currentItem->isActive){
                                const float factor = glm::smoothstep( 0.0f , (float)user->currentItem->maxDuration , (float)user->currentItem->duration);
                                const float green = factor;
                                const float red = 1.0f - green;
                                inventoryShader->set("color" , glm::vec3(red , green , 0.0f));
                            }else{
                                inventoryShader->set("color" , glm::vec3(0.0f , 1.0f , 0.0f));
                            }
                        }else{
                            inventoryShader->set("color" , glm::vec3(1.0f , 0.0f , 0.0f));
                        }
                    }else{
                        model = glm::scale(model , glm::vec3(0.07f));
                        if(tempStatusItem[i]){
                            inventoryShader->set("color" , glm::vec3(0.0f , 1.0f , 0.0f));
                        }else{
                            inventoryShader->set("color" , glm::vec3(1.0f , 0.0f , 0.0f));
                        }
                    }
                    inventoryShader->set("model" , model);

                    
                    glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
                }
            }
            inventoryShader->off();
            // setup for showing of stamina [[maybe_unused]] i will do flash.duration
            

            // running stamina
            inventorySquare->use();
            {
                const float perc = getPercentage((float)user->runningStamina , (float)constValues::maxRunningTime);
                const float factor = glm::smoothstep(0.0f , (float)constValues::maxRunningTime , (float)user->runningStamina);
                const float green = factor;
                const float red = 1.0f - factor;
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , glm::vec3(0.0f , -0.79f , 0.02f));
                model = glm::rotate(model , glm::radians(90.0f) , glm::vec3(0.0f , 0.0f, 1.0f));
                model = glm::scale(model , glm::vec3(0.01f , 1.2f * perc, 0.05f));
                inventorySquare->set("color" , glm::vec3(red , green, 0.0f));
                inventorySquare->set("model" , model);
                glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
            }
            
            {
                const float perc = getPercentage((float)user->flash.duration , user->flash.maxDuration);
                const float factor = glm::smoothstep(0.0f , (float)user->flash.maxDuration , (float)user->flash.duration);
                const glm::vec3 color = glm::mix(glm::vec3(0.0f) , glm::vec3(0.0f, 0.0, 1.0f) , factor);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model , glm::vec3(0.0f , -0.75f , 0.02f));
                model = glm::rotate(model , glm::radians(90.0f) , glm::vec3(0.0f , 0.0f, 1.0f));
                model = glm::scale(model , glm::vec3(0.01f , 1.2f * perc, 0.05f));
                inventorySquare->set("model" , model);
                inventorySquare->set("color" , color);
                glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);

            }
            inventorySquare->off();
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER , msaaFrame);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glBlitFramebuffer(0 , 0 , Width , Height , 0 , 0 , Width , Height , GL_COLOR_BUFFER_BIT , GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER , 0);
        glClearColor(0.0f , 0.0f , 0.0f , 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D , texture);

        if(!user->hasNightVision){
            screenShader->use();
            glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
            screenShader->off();
        }else{
            screenNightVision->use();
            glDrawElements(GL_TRIANGLES , 6 , GL_UNSIGNED_SHORT , 0);
            screenNightVision->off();
        }

        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D , 0);
        glBindVertexArray(0);

        

        glfwSwapBuffers(window);
        glfwPollEvents();
        [[unlikely]] if(crystalCollected == 10){
            glfwSetWindowShouldClose(window , true);
            std::cout << "player has won\n";
        }
        checkDeadMobs(TempMobs);
        float time = glfwGetTime();
        const unsigned int currentSecond = static_cast<unsigned int>(time);

        // --- here all the system for SECOND PROCESS --- 
        
        if(currentSecond != static_cast<int>(lastSecond)){
            lastSecond = time;
            Timer--;
            if(Timer == 0){
                glfwSetWindowShouldClose(window , true);
                std::cout << "[TIMER IS OUT]\n";
            }
            // flash system
            if(user->flash.flashTurn){
                user->flash.duration--;
                std::cout << user->flash.duration << std::endl;
                if(user->flash.duration <= 0){
                    user->flash.batteryEmpty = true;
                    user->flash.flashTurn = false;
                }
            }
            // running system
            if(user->running){
                user->runningStamina -= 1;
                if(user->runningStamina <= 0){
                    user->secondsExhaust = 0;
                    user->needToWaitSeconds_5 = true;
                    user->runningStamina = 0;
                    user->running = false;
                }
            }else if(!user->needToWaitSeconds_5){
                user->runningStamina++;
                if(user->runningStamina > constValues::maxRunningTime) user->runningStamina = constValues::maxRunningTime;
            }
            if(user->needToWaitSeconds_5){
                user->secondsExhaust++;
                if(user->secondsExhaust >= 5){
                    user->needToWaitSeconds_5 = false;
                    user->secondsExhaust = 0;
                }
            }
            // items process
            processSecondsItem(items);


            if(user->currentItem){
                if((user->currentItem->isActive) && user->currentItem->isUsed){
                user->useItem(TempMobs);
                }
            }

            // raining system
            {
                if(!rainSystem->raining){
                    const unsigned short needRain = rand()%101;
                    const unsigned short index = rand()%100 + 1;
                    if(needRain % index == 0){
                        rainSystem->raining = true;
                        rainSystem->duration = constValues::rainDuration;
                    }                    
                }else{
                    rainSystem->duration--;
                    if(rainSystem->duration == 0){
                        rainSystem->raining = false;
                    }
                }
            }

        }
        for(unsigned short i = 0 ; i < items.size(); i++){
            if(items[i]->isUsed) items.erase(items.begin() + i);
        }
    }
    
    std::cout << "[" + std::to_string(crystalCollected) + "]" << " is the amount of crystatls player collected\n";

    user.reset();
    std::cout << "[ENDING THE PROGRAM]" << std::endl;
    glDeleteBuffers(1 ,&EBO);
    glDeleteBuffers(1 , &VBO);
    glDeleteVertexArrays( 1 ,&VAO);

    glDeleteFramebuffers(1 , &frameBuffer);
    glDeleteRenderbuffers(1 , &renderBuffer);
    glDeleteFramebuffers(1, &msaaFrame);
    glDeleteRenderbuffers(1 , &msaaRender);
    glDeleteTextures(1, &texture);
    glDeleteTextures(1 , &msaaTexture);
    
    

    glDeleteBuffers(1 , &shadowFrame);
    glfwTerminate();
    return 0;
}

