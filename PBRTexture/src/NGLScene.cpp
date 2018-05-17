#include "NGLScene.h"
#include <QGuiApplication>
#include <QMouseEvent>
#include <glm/gtc/type_ptr.hpp>
#include <ngl/Camera.h>
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Texture.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <array>
#include "TexturePack.h"

//----------------------------------------------------------------------------------------------------------------------
/// @brief extents of the bbox
//----------------------------------------------------------------------------------------------------------------------
const static int s_extents=20;

NGLScene::NGLScene()
{
  setTitle( "Game" );
  m_animate=true;
  m_checkSphereSphere=false;

  m_numSpheres=1;

  m_timer.start();
}


NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setProjection( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


// lights
static std::array<ngl::Vec3,4> g_lightPositions = {{
        ngl::Vec3(-5.0f,  4.0f, -5.0f),
        ngl::Vec3( 5.0f,  4.0f, -5.0f),
        ngl::Vec3(-5.0f,  4.0f, 5.0f),
        ngl::Vec3( 5.0f,  4.0f, 5.0f)
    }};
void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();


  // Set background colour
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);

  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

  // Create and compile the vertex and fragment shader
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  // lights
  static std::array<ngl::Vec3,4> g_lightPositions = {{
          ngl::Vec3(-5.0f,  4.0f, -5.0f),
          ngl::Vec3( 5.0f,  4.0f, -5.0f),
          ngl::Vec3(-5.0f,  4.0f, 5.0f),
          ngl::Vec3( 5.0f,  4.0f, 5.0f)
      }};

    TexturePack tp;
    tp.loadJSON("textures/textures.json");


    glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); // Grey Background
    // enable depth testing for drawing
    glEnable( GL_DEPTH_TEST );
    // enable multisampling for smoother drawing
    #ifndef USINGIOS_
    glEnable( GL_MULTISAMPLE );
    #endif
    // now to load the shader and set the values
    // grab an instance of shader manager

    // we are creating a shader called Phong to save typos
    // in the code create some constexpr
    constexpr auto shaderProgram = "PBR";
    constexpr auto vertexShader  = "PBRVertex";
    constexpr auto fragShader    = "PBRFragment";
    // create the shader program
    shader->createShaderProgram( shaderProgram );
    // now we are going to create empty shaders for Frag and Vert
    shader->attachShader( vertexShader, ngl::ShaderType::VERTEX );
    shader->attachShader( fragShader, ngl::ShaderType::FRAGMENT );
    // attach the source
    shader->loadShaderSource( vertexShader, "shaders/PBRVertex.glsl" );
    shader->loadShaderSource( fragShader, "shaders/PBRFragment.glsl" );
    // compile the shaders
    shader->compileShader( vertexShader );
    shader->compileShader( fragShader );
    // add them to the program
    shader->attachShaderToProgram( shaderProgram, vertexShader );
    shader->attachShaderToProgram( shaderProgram, fragShader );


    // now we have associated that data we can link the shader
    shader->linkProgramObject( shaderProgram );
    // and make it active ready to load values
    shader->loadShader("SkyProgram",
                     "shaders/test_vert.glsl",
                     "shaders/test_frag.glsl");

    initEnvironment();
    shader->setUniform("albedoMap", 0);
    ( *shader )[ shaderProgram ]->use();
    // Now we will create a basic Camera from the graphics library
    // This is a static camera so it only needs to be set once
    // First create Values for the camera position
    ngl::Vec3 from( 0, 5, 5 );
    ngl::Vec3 to( 0, 0, 0 );
    ngl::Vec3 up( 0, 1, 0 );
    // now load to our new camera
    m_cam.set( from, to, up );
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.5 and 10
    m_cam.setProjection( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );

    shader->setUniform("camPos",m_cam.getEye());

        std::array<ngl::Vec3,4>  lightColors = {{
            ngl::Vec3(250.0f, 250.0f, 250.0f),
            ngl::Vec3(250.0f, 250.0f, 250.0f),
            ngl::Vec3(250.0f, 250.0f, 250.0f),
            ngl::Vec3(250.0f, 250.0f, 250.0f)

        }};

    for(size_t i=0; i<g_lightPositions.size(); ++i)
    {
      shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
      shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),lightColors[i]);
    }
    shader->setUniform("albedoMap", 0);
    shader->setUniform("normalMap", 1);
    shader->setUniform("metallicMap", 2);
    shader->setUniform("roughnessMap", 3);
    shader->setUniform("aoMap", 4);

    ( *shader )[ ngl::nglColourShader ]->use();
    shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
  /*

*/


    //shader->setUniform("camPos",m_cam.getEye());
    //shader->setUniform("albedoMap", 0);

    (*shader)["nglDiffuseShader"]->use();

    shader->setUniform("Colour",1.0f,1.0f,0.0f,1.0f);
    shader->setUniform("lightPos",1.0f,1.0f,1.0f);
    shader->setUniform("lightDiffuse",1.0f,1.0f,1.0f,1.0f);

    (*shader)["nglColourShader"]->use();
    shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

    m_bbox.reset( new ngl::BBox(ngl::Vec3(),15.0f,6.0f,12.0f));

    m_bbox->setDrawMode(GL_LINE);

    m_sphereUpdateTimer=startTimer(40);



  // Load the Obj file and create a Vertex Array Object
  m_mesh.reset(new ngl::Obj("models/level.obj"));
  m_mesh->createVAO();


  m_crystal1.reset(new ngl::Obj("models/crystal1.obj"));
  m_crystal1->createVAO();


  m_crystal2.reset(new ngl::Obj("models/crystal2.obj"));
  m_crystal2->createVAO();


  m_crystal3.reset(new ngl::Obj("models/crystal3.obj"));
  m_crystal3->createVAO();


  m_mesh2.reset(new ngl::Obj("models/skybox2.obj"));
  m_mesh2->createVAO();

  ngl::VAOPrimitives *prim =  ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",1.0f,40.0f);


}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 N;
  ngl::Mat4 M;
  M            = m_transform.getMatrix() ;
  MV           = m_cam.getView() * M;
  MVP          = m_cam.getVP() * M;

  N = MV;
  N.inverse().transpose();
  shader->setUniform( "MVP", MVP );
  shader->setUniform( "N", N );
  shader->setUniform( "M", M );
  //ngl::Random *rng = ngl::Random::instance();
  //shader->setUniform("textureRotation",);
 /* ngl::Real textureRotation=ngl::radians((rng->randomNumber(180.0f)));
  float cosTheta=cosf(textureRotation);
  float sinTheta=sinf(textureRotation);
  ngl::Real texRot[4]={cosTheta,sinTheta,-sinTheta,cosTheta};
  shader->setUniformMatrix2fv("textureRotation",&texRot[0]);
  */
  shader->setUniform("camPos",m_cam.getEye());


}

void NGLScene::paintGL()
{
  float currentFrame = m_timer.elapsed()*0.001f;
//  std::cout<<"Current Frame "<<currentFrame<<'\n';
  m_deltaTime = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;

  glViewport( 0, 0, m_win.width, m_win.height );
  // clear the screen and depth buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // grab an instance of the shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  ( *shader )[ "PBR" ]->use();

  /// first we reset the movement values
  float xDirection=0.0;
  float yDirection=0.0;

//  std::cout << "y diretion is " <<yDirection << std::endl;
  // now we loop for each of the pressed keys in the the set
  // and see which ones have been pressed. If they have been pressed
  // we set the movement value to be an incremental value
  foreach(Qt::Key key, m_keysPressed)
  {
    switch (key)
    {
      case Qt::Key_Left :  { yDirection=-1.0f; break;}
      case Qt::Key_Right : { yDirection=1.0f; break;}
      case Qt::Key_Up :		 { xDirection=1.0f; break;}
      case Qt::Key_Down :  { xDirection=-1.0f; break;}
      default : break;
    }
  }
  // if the set is non zero size we can update the ship movement
  // then tell openGL to re-draw
  if(m_keysPressed.size() !=0)
  {
    m_cam.move(xDirection,yDirection,m_deltaTime);
  }
  // get the VBO instance and draw the built in teapot


  TexturePack tp;
  tp.activateTexturePack("level");

  shader->setUniform("roughnessScale",0.0f);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader();
  // Draw our Obj mesh
  m_mesh->draw();
  tp.activateTexturePack("crystal1");
  m_crystal1->draw();
  tp.activateTexturePack("crystal2");
  m_crystal2->draw();
  tp.activateTexturePack("crystal3");
  m_crystal3->draw();

  //(*shader)["nglColourShader"]->use();




  //m_bbox->draw();
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

  // Draw Lights
  if(m_drawLights)
  {
    ( *shader )[ ngl::nglColourShader ]->use();
    ngl::Mat4 MVP;
    ngl::Transformation tx;
    shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

    for(size_t i=0; i<g_lightPositions.size(); ++i)
    {
      if(m_lightOn[i]==true)
      {
        tx.setPosition(g_lightPositions[i]);
        MVP=m_cam.getVP()* m_mouseGlobalTX * tx.getMatrix() ;
        shader->setUniform("MVP",MVP);
        prim->draw("sphere");
      }
    }
  }
  ( *shader )[ "SkyProgram" ]->use();
  tp.activateTexturePack("sky");
  loadMatricesToShader();
  m_mesh2->draw();




  BBoxCollision();
  updateScene();
  update();

}

//----------------------------------------------------------------------------------------------------------------------



void NGLScene::resetSpheres()
{
    m_sphereArray.clear();
    ngl::Vec3 dir;
    ngl::Random *rng=ngl::Random::instance();
    // loop and create the initial particle list
    for(int i=0; i<m_numSpheres; ++i)
    {
        dir=rng->getRandomVec3();
        // add the spheres to the end of the particle list
    m_sphereArray.push_back(Sphere(rng->getRandomPoint(s_extents,s_extents,s_extents),dir,rng->randomPositiveNumber(2)+0.5f));
    }

}

void NGLScene::updateScene()
{
    for(Sphere &s : m_sphereArray)
    {
        s.move();
    }
    checkCollisions();
}


void NGLScene::keyPressEvent( QKeyEvent* _event )
{
  // add to our keypress set the values of any keys pressed
  m_keysPressed += static_cast<Qt::Key>(_event->key());
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  ngl::Random *rng=ngl::Random::instance();
  auto setLight=[](std::string _num,bool _mode)
  {
    ngl::ShaderLib *shader= ngl::ShaderLib::instance();
    shader->use("PBR");
    if(_mode == true)
    {
      ngl::Vec3 colour={255.0f,255.0f,255.0f};
      shader->setUniform(_num,colour);
    }
    else
    {
      ngl::Vec3 colour={0.0f,0.0f,0.0f};
      shader->setUniform(_num,colour);

    }

  };
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
    case Qt::Key_R :
      m_seed=static_cast<unsigned int>(rng->randomPositiveNumber(100000));
    break;
// turn on wirframe rendering
#ifndef USINGIOS_
    case Qt::Key_W:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      break;
    // turn off wire frame
    case Qt::Key_S:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      break;
#endif
    // show full screen
    case Qt::Key_F:
      showFullScreen();
      break;
    case Qt::Key_L :
      m_drawLights^=true;
    break;
    // show windowed
    case Qt::Key_N:
      showNormal();
      break;
    case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());
    break;
  case Qt::Key_1 :
    setLight("lightColors[0]",m_lightOn[0]^=true); break;
  case Qt::Key_2 :
    setLight("lightColors[1]",m_lightOn[1]^=true); break;
  case Qt::Key_3 :
    setLight("lightColors[2]",m_lightOn[2]^=true); break;
  case Qt::Key_4 :
    setLight("lightColors[3]",m_lightOn[3]^=true); break;

    default:
      break;
  }
  BBoxCollision();
  update();
}

void NGLScene::keyReleaseEvent( QKeyEvent *_event	)
{
  // remove from our key set any keys that have been released
  m_keysPressed -= static_cast<Qt::Key>(_event->key());
}

// Code to load up textures without using a texture map. Originally by Richard Southern.
void NGLScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
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
 *
 */
void NGLScene::initEnvironment() {
    // Enable seamless cube mapping
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Placing our environment map texture in texture unit 0
    glActiveTexture (GL_TEXTURE0);

    // Generate storage and a reference for our environment map texture
    glGenTextures (1, &m_envTex);

    // Bind this texture to the active texture unit
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTex);

    // Now load up the sides of the cube
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "textures/skybox/sky_zneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "textures/skybox/sky_zpos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "textures/skybox/sky_ypos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "textures/skybox/sky_yneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "textures/skybox/sky_xneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "textures/skybox/sky_xpos.png");

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
    shader->use("SkyProgram");
    shader->setUniform("envMap", 10);

}

/**
 * @brief Scene::initEnvironmentSide
 * @param texture
 * @param target
 * @param filename
 * This function should only be called when we have the environment texture bound already
 * copy image data into 'target' side of cube map
 */
void NGLScene::initEnvironmentSide(GLenum target, const char *filename) {
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

//----------------------------------------------------------------------------------------------------------------------
bool NGLScene::sphereSphereCollision( ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2 )
{
  // the relative position of the spheres
  ngl::Vec3 relPos;
  //min an max distances of the spheres
  GLfloat dist;
  GLfloat minDist;
  GLfloat len;
  relPos =_pos1-_pos2;
  // and the distance
  len=relPos.length();
  dist=len*len;
  minDist =_radius1+_radius2;
  // if it is a hit
  if(dist <=(minDist * minDist))
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Attempt to make the camera not able to pass the bounding box. It detects the collision but doesn't stop the camera from moving.
void NGLScene::BBoxCollision()
{
  //create an array of the extents of the bounding box
  float xDirection=0.0;
  float yDirection=0.0;
  float ext[6];
  ext[0]=ext[1]=(m_bbox->height()/2.0f);
  ext[2]=ext[3]=(m_bbox->width()/2.0f);
  ext[4]=ext[5]=(m_bbox->depth()/2.0f);
  // Dot product needs a Vector so we convert The Point Temp into a Vector so we can
  // do a dot product on it
  ngl::Vec3 p;
  // D is the distance of the Agent from the Plane. If it is less than ext[i] then there is
  // no collision
  GLfloat D;

    p=m_cam.getEye();
    //Now we need to check the Sphere agains all 6 planes of the BBOx
    //If a collision is found we change the dir of the Sphere then Break
    for(int i=0; i<6; ++i)
    {
      //to calculate the distance we take the dotporduct of the Plane Normal
      //with the new point P
      D=m_bbox->getNormalArray()[i].dot(p);
      //Now Add the Radius of the sphere to the offsett
      D+=26;
      // If this is greater or equal to the BBox extent /2 then there is a collision
      //So we calculate the Spheres new direction
      if(D >=ext[i])
      {

        std::cout << "Collision detected";
        m_cam.move(xDirection,yDirection,m_deltaTime);
        update();
      }//end of hit test
     }//end of each face test

}

void  NGLScene::checkSphereCollisions()
{
  bool collide;

  unsigned int size=m_sphereArray.size();

    for(unsigned int ToCheck=0; ToCheck<size; ++ToCheck)
    {
        for(unsigned int Current=0; Current<size; ++Current)
        {
            // don't check against self
            if(ToCheck == Current)  continue;

      else
      {
        //cout <<"doing check"<<endl;
        collide =sphereSphereCollision(m_sphereArray[Current].getPos(),m_sphereArray[Current].getRadius(),
                                       m_sphereArray[ToCheck].getPos(),m_sphereArray[ToCheck].getRadius()
                                      );
        if(collide== true)
        {
          m_sphereArray[Current].reverse();
          m_sphereArray[Current].setHit();
        }
      }
    }
  }
}


void  NGLScene::checkCollisions()
{

    if(m_checkSphereSphere == true)
    {
        checkSphereCollisions();
    }
    BBoxCollision();

}


/*
 Code for sphere collision - I think it works, but the spheres don't.
 void NGLScene::BBoxCollision()
{
  //create an array of the extents of the bounding box
  float ext[6];
  ext[0]=ext[1]=(m_bbox->height()/2.0f);
  ext[2]=ext[3]=(m_bbox->width()/2.0f);
  ext[4]=ext[5]=(m_bbox->depth()/2.0f);
  // Dot product needs a Vector so we convert The Point Temp into a Vector so we can
  // do a dot product on it
  ngl::Vec3 p;
  // D is the distance of the Agent from the Plane. If it is less than ext[i] then there is
  // no collision
  GLfloat D;
  // Loop for each sphere in the vector list
  for(Sphere &s : m_sphereArray)
  {
    p=s.getPos();
    //Now we need to check the Sphere agains all 6 planes of the BBOx
    //If a collision is found we change the dir of the Sphere then Break
    for(int i=0; i<6; ++i)
    {
      //to calculate the distance we take the dotporduct of the Plane Normal
      //with the new point P
      D=m_bbox->getNormalArray()[i].dot(p);
      //Now Add the Radius of the sphere to the offsett
      D+=s.getRadius();
      // If this is greater or equal to the BBox extent /2 then there is a collision
      //So we calculate the Spheres new direction
      if(D >=ext[i])
      {
        //We use the same calculation as in raytracing to determine the
        // the new direction
        GLfloat x= 2*( s.getDirection().dot((m_bbox->getNormalArray()[i])));
        ngl::Vec3 d =m_bbox->getNormalArray()[i]*x;
        s.setDirection(s.getDirection()-d);
        s.setHit();
      }//end of hit test
     }//end of each face test
    }//end of for
}
*/
