// Base code taken from objviewer and environment example code by Richard Southern
#include "DrawGame.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <array>

DrawGame::DrawGame() : Scene() {}

//Initialize GL
void DrawGame::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("EnvironmentProgram",
                       "shaders/test_vert.glsl",
                       "shaders/test_frag.glsl");

    shader->loadShader("SkyProgram",
                       "shaders/env_vert.glsl",
                       "shaders/env_frag.glsl");

    shader->loadShader("TableProgram",
                       "shaders/table_vert.glsl",
                       "shaders/table_frag.glsl");

    shader->loadShader("CrystalProgram",
                       "shaders/shadow_vert.glsl",
                       "shaders/shadow_frag.glsl");
    // Initialise our environment map here
    initEnvironment();

    // Initialise our textures map here
    // data taken from google search
    initTexture(5, m_dirtTex, "data/level_gloss.png");
    initTexture(4, m_specMapTex, "data/spec.jpg");
    initTexture(3, m_normalTex, "data/level_normal.jpg");
    initTexture(2, m_colorTex, "data/level_color.jpg");
    initTexture(1, m_glossMapTex, "data/metal2.jpg");
    initTexture(7, m_woodTex, "data/wood.tx");

    shader->use("EnvironmentProgram");
    shader->setUniform("glossMap", 1);
    shader->setUniform("colorMap", 2);
    shader->setUniform("normalMap", 3);
    shader->setUniform("specMap", 4);
    shader->setUniform("dirtMap", 5);

    shader->use("CrystalProgram");
    initTexture(14, m_dirtTex, "data/TEX_Crystal3_E.png");
    initTexture(10, m_specMapTex, "data/crystal1_Metallic.png");
    initTexture(11, m_normalTex, "data/crystal1_Normal_OpenGL.png");
    initTexture(12, m_colorTex, "data/TEX_Crystal3_BC.png");
    initTexture(13, m_glossMapTex, "data/crystal1_Roughness.png");
    shader->setUniform("dirtMap", 14);
    shader->setUniform("glossMap", 13);
    shader->setUniform("colorMap", 12);
    shader->setUniform("normalMap", 11);
    shader->setUniform("specMap", 10);
    //shader->setUniform("dirtMap", 5);


    // Load the Obj file and create a Vertex Array Object
    m_mesh.reset(new ngl::Obj("data/level.obj"));
    m_mesh->createVAO();

    //shader->use("SkyProgram");
    m_crystal1.reset(new ngl::Obj("data/crystal1.obj"));
    m_crystal1->createVAO();

    //shader->use("SkyProgram");
    m_crystal2.reset(new ngl::Obj("data/crystal2.obj"));
    m_crystal2->createVAO();

    //shader->use("SkyProgram");
    m_crystal3.reset(new ngl::Obj("data/crystal3.obj"));
    m_crystal3->createVAO();

    //shader->use("SkyProgram");
    m_mesh2.reset(new ngl::Obj("data/skybox.obj"));
    m_mesh2->createVAO();

    initTexture(8, m_glossMapTex, "data/gloss.png");
    shader->use("TableProgram");
    shader->setUniform("coloringMap", 7);
    shader->setUniform("glossMap", 8);
    m_table.reset(new ngl::Obj("data/wood.obj"));
    m_table->createVAO();
}

void DrawGame::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["EnvironmentProgram"]->use();
    GLint pid = shader->getProgramID("EnvironmentProgram");

    // Our MVP matrices
    glm::mat4 M = glm::translate(glm::mat4(3.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data

    // Draw our Obj mesh
    m_mesh->draw();

    (*shader)["CrystalProgram"]->use();
    GLint pid4 = shader->getProgramID("CrystalProgram");


    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid4, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid4, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid4, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data

    //m_table->draw();
    m_crystal1->draw();
    m_crystal2->draw();
    m_crystal3->draw();

    // same process for table shader and mesh
    (*shader)["TableProgram"]->use();
    GLint pid3 = shader->getProgramID("TableProgram");


    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid3, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid3, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid3, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    //m_table->draw();


    // skybox shader as well
    (*shader)["SkyProgram"]->use();
    GLint pid2 = shader->getProgramID("SkyProgram");



    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid2, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid2, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid2, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data



   // ngl::VAOPrimitives::instance()->draw("cube");
    m_mesh2->draw();

}

void DrawGame::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
    // Set our active texture unit
    glActiveTexture(GL_TEXTURE0 + texUnit);

    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Create storage for our new texture
    glGenTextures(1, &texId);

    // Bind the current texture
    glBindTexture(GL_TEXTURE_2D, texId);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                GL_RGB,          // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/**
 * @brief Scene::initEnvironment in texture unit 0
 */
void DrawGame::initEnvironment() {
    // Enable seamless cube mapping
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Placing our environment map texture in texture unit 0
    glActiveTexture (GL_TEXTURE0);

    // Generate storage and a reference for our environment map texture
    glGenTextures (1, &m_envTex);

    // Bind this texture to the active texture unit
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTex);

    // Now load up the sides of the cube
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "data/sky_zneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "data/sky_zpos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "data/sky_ypos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "data/sky_yneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "data/sky_xneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "data/sky_xpos.png");

    // Generate mipmap levels
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Set the texture parameters for the cube map
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfloat anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    // Set our cube map texture to on the shader so we can use it
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use("EnvironmentProgram");
    shader->setUniform("envMap", 0);
    shader->use("SkyProgram");
    shader->setUniform("enviMap", 0);
    shader->use("TableProgram");
    shader->setUniform("environmentMap", 0);
}

/**
 * @brief Scene::initEnvironmentSide
 * @param texture
 * @param target
 * @param filename
 * This function should only be called when we have the environment texture bound already
 * copy image data into 'target' side of cube map
 */
void DrawGame::initEnvironmentSide(GLenum target, const char *filename) {
    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
      target,           // The target (in this case, which side of the cube)
      0,                // Level of mipmap to load
      img.format(),     // Internal format (number of colour components)
      img.width(),      // Width in pixels
      img.height(),     // Height in pixels
      0,                // Border
      GL_RGBA,          // Format of the pixel data
      GL_UNSIGNED_BYTE, // Data type of pixel data
      img.getPixels()   // Pointer to image data in memory
    );
}
