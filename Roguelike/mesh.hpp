#ifndef MESH_HPP
#define MESH_HPP
#include <optional>
#include "texture.hpp"
#include <memory>

class Mesh {
protected:
    GLuint VBO , EBO , VAO;
    Mesh() = default;
    void setup() noexcept {
        glGenBuffers(1 , &VBO);
        glGenBuffers(1  , &EBO);
        glGenVertexArrays(1 , &VAO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER , VBO);
        glBufferData(GL_ARRAY_BUFFER , sizeof(vertices[0]) * vertices.size() , &(vertices[0]) , GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER , sizeof(indices[0]) * indices.size() , &(indices[0]) , GL_STATIC_DRAW);

        glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 8 * sizeof(float) , (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1 , 3 , GL_FLOAT , GL_FALSE , 8  * sizeof(float) , (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2 , 2 , GL_FLOAT , GL_FALSE , 8 * sizeof(float) , (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER , 0);
    }
public:
    glm::mat4 model{glm::mat4(1.0f)};
    float radius;
    bool linear{false};
    bool hasTexture{true};
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;
    
    GLint shiny{2};
    float trans{1.0f};
    GLuint texture;
    std::vector<unsigned short> indices;
    std::vector<float> vertices;
    std::optional<GLuint> normalTexture;
    std::optional<GLuint> depthTexture;
    
    Mesh(std::vector<float> const &vertices , std::vector<unsigned short> const &indices){
        this->vertices = vertices;
        this->indices = indices;
        hasTexture = false;
        setup();
    }
    Mesh(std::vector<float> const &vertices , std::vector<unsigned short> const &indices , const GLuint &texture): Mesh(vertices , indices){
        hasTexture = true;
        this->texture = texture;
    }

    void Delete() noexcept {
        glDeleteBuffers( 1,  &VBO);
        glDeleteBuffers(1,  &EBO);
        glDeleteVertexArrays(1 , &VAO);
    }

    void draw(Shader* shader) noexcept {
        shader->set("model" , model);
        shader->set("material.shiny" , shiny);
        shader->set("material.trans" , trans);
        linear = shader->linear;
        glBindVertexArray(VAO);
        if(hasTexture){ 
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D , texture);
            if(normalTexture){
                glActiveTexture(GL_TEXTURE10);
                glBindTexture(GL_TEXTURE_2D , normalTexture.value());
            }
            if(depthTexture){
                glActiveTexture(GL_TEXTURE11);
                glBindTexture(GL_TEXTURE_2D , depthTexture.value());
            }
        }
        if(!linear){
            glDrawElements(GL_TRIANGLES , indices.size() , GL_UNSIGNED_SHORT , 0);
        }else{
            glDrawElements(GL_LINE_LOOP , indices.size() , GL_UNSIGNED_SHORT , 0);
        }
        glBindVertexArray(0);
        if(hasTexture){
            glBindTexture(GL_TEXTURE_2D , 0);
        }
    }
    ~Mesh() {Delete();}
};


class InstanceMesh{
protected:
    InstanceMesh() = default;
    GLuint VBO , VAO , EBO , IVBO;
    inline void setup() noexcept{
        glGenBuffers(1 , &VBO);
        glGenBuffers(1 , &EBO);
        glGenBuffers(1 , &IVBO);

        glGenVertexArrays(1, &VAO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER , VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ARRAY_BUFFER , vertices.size() * sizeof(float) , vertices.data() , GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER , indices.size() * sizeof(unsigned short) , indices.data() , GL_STATIC_DRAW);

        glVertexAttribPointer(0 , 3 , GL_FLOAT , GL_FALSE , 8 * sizeof(float) , (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1 , 3 , GL_FLOAT , GL_FALSE , 8 * sizeof(float) , (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT , GL_FALSE , 8 * sizeof(float) , (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER , IVBO);
        glBufferData(GL_ARRAY_BUFFER , matrix.size() * sizeof(glm::mat4) , matrix.data() , GL_STATIC_DRAW);
        const std::size_t v_s = sizeof(glm::vec4);
        glVertexAttribPointer(3 , 4 , GL_FLOAT , GL_FALSE , 4 * v_s , (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(4 , 4, GL_FLOAT , GL_FALSE , 4 * v_s , (void*)(v_s));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(5 , 4 , GL_FLOAT , GL_FALSE , 4 * v_s , (void*)(2 * v_s));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(6 , 4 , GL_FLOAT , GL_FALSE , 4 * v_s , (void*)(3 * v_s));
        glEnableVertexAttribArray(6);

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER , 0);
    }

public:
    std::vector<glm::mat4> matrix;
    GLint shiny{2};
    float trans{1.0f};
    GLuint texture;
    std::vector<unsigned short> indices;
    std::vector<float> vertices;
    std::optional<GLuint> normalTexture;
    std::optional<GLuint> depthTexture;

    bool linear{false};
    bool hasTexture{true};

    explicit InstanceMesh(std::vector<float> const &vertices , const std::vector<unsigned short> &indices , std::vector<glm::mat4> const &matrix){
        hasTexture = false;
        this->vertices = vertices;
        this->indices = indices;
        this->matrix = matrix;
        setup();
    }
    explicit InstanceMesh(std::vector<float> const &vertices , const std::vector<unsigned short> &indices , std::vector<glm::mat4> const &matrix , GLuint &texture) :
    InstanceMesh(vertices , indices , matrix){
        hasTexture = true;
        this->texture = texture;
    }

    inline void draw(Shader* shader) noexcept{ // SHADER HAS TO BE ONLY WITH INSTANCE
        [[unlikely]] if(!shader) return;
        if(hasTexture){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D , texture);
            if(normalTexture){
                glActiveTexture(GL_TEXTURE10);
                glBindTexture(GL_TEXTURE_2D , normalTexture.value());
            }
            if(depthTexture){
                glActiveTexture(GL_TEXTURE11);
                glBindTexture(GL_TEXTURE_2D , depthTexture.value());
            }
        }
        glBindVertexArray(VAO);
        shader->set("material.shiny" , shiny);
        shader->set("material.trans" , trans);
        linear = shader->linear;
        if(linear) glDrawElementsInstanced(GL_LINE_LOOP , indices.size() , GL_UNSIGNED_SHORT , NULL , matrix.size());
        else glDrawElementsInstanced(GL_TRIANGLES , indices.size() , GL_UNSIGNED_SHORT , NULL , matrix.size());
        glBindVertexArray(0);
        if(hasTexture){
            glBindTexture(GL_TEXTURE_2D , 0);
        }
    }


    inline void Delete() const noexcept{glDeleteBuffers(1 , &VBO); glDeleteBuffers(1 , &EBO); glDeleteBuffers(1 , &IVBO); glDeleteVertexArrays(1 , &VAO);}


    virtual ~InstanceMesh() {Delete();}
};



#endif