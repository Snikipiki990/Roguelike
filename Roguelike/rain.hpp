#ifndef rain_hpp
#define rain_hpp

inline const unsigned short amount = 2000;

#include "shader.hpp"
#ifndef Global_HPP
    #include "global.hpp"
    using namespace constValues;
    using namespace globalValues;
#endif

struct Particle{
    glm::vec3 position;
    const glm::vec4 color{glm::vec3(0.0f , 0.0f , 0.5f) , 0.5f};
    const glm::vec3 direction{glm::vec3(0.0f, -1.0f , 0.0f)};

    float time{3.0f};
    const float startTime{3.0f};
    float size{0.2f};

    bool active{true};
    [[nodiscard]] inline float lifeFactor() const noexcept { return time/startTime; }
};



class Rain{
private:
    GLuint VBO , VAO;
    Rain() = delete;
    std::vector<Particle> particles;
    unsigned short maxAmount;
    const unsigned short rangeAround_player = 30.0f;
public:

    unsigned short duration{30};
    bool raining{false};

    explicit Rain(const unsigned short &number) 
    {
        maxAmount = static_cast<unsigned short>((number > 0 ? (number > 200 ? 200 : number) : 200));
        glGenBuffers(1 , &VBO);
        glGenVertexArrays(1 , &VAO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER , VBO);

        glBufferData(GL_ARRAY_BUFFER , amount * sizeof(Particle) , NULL , GL_DYNAMIC_DRAW);
        glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE , sizeof(Particle) , (void*)(offsetof(Particle , position)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer( 1 , 4 , GL_FLOAT , GL_FALSE , sizeof(Particle) , (void*)(offsetof(Particle , color)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer( 2 , 1 , GL_FLOAT , GL_FALSE , sizeof(Particle) , (void*)(offsetof(Particle , size)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER , 0);
        particles.resize(amount);
        for (auto& p : particles) {
            p.position = generatePos(glm::vec3(0.0f)); // Начальная позиция не важна, обновится в draw
            p.time = (float)(rand() % 300) / 100.0f;
        }
    }
    
[[nodiscard]] glm::vec3 generatePos(const glm::vec3 playerPos) const noexcept {
        int pX = (rand() % 2 == 0) ? 1 : -1;
        int pZ = (rand() % 2 == 0) ? 1 : -1;
        
        glm::vec3 temp;
        temp.x = playerPos.x + (float)(pX * (rand() % (int)rangeAround_player));
        temp.y = playerPos.y + 15.0f + (float)(rand()%10); 
        temp.z = playerPos.z + (float)(pZ * (rand() % (int)rangeAround_player));
        return temp;
    }

void draw(const glm::vec3& playerPos, Shader* shader) noexcept {
        if (!shader) return;
        for (auto& p : particles) {
            p.position += p.direction * (globalValues::DeltaTime * 30.0f); 
            p.time -= globalValues::DeltaTime;

            // Респавн капли
            if (p.position.y < (playerPos.y - 3.0f) || p.time <= 0.0f) {
                p.position = generatePos(playerPos);
                p.time = p.startTime;
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, amount * sizeof(Particle), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(Particle), particles.data());
        glEnable(GL_PROGRAM_POINT_SIZE); 
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particles.size());
        glBindVertexArray(0);   
    }


    ~Rain() {glDeleteBuffers(1 , &VBO); glDeleteVertexArrays(1, &VAO);}
};

#endif