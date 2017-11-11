#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

int width = 1024;
int height = 768;

int main( void )
{
  if( !glfwInit() )
  {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      return -1;
  }
  
  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

  // Open a window and create its OpenGL context
  window = glfwCreateWindow( width, height, "Tutorial 01", NULL, NULL);
  if( window == NULL ){
      fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
      glfwTerminate();
      return -1;
  }
    
  glfwMakeContextCurrent(window); // Initialize GLEW
  glewExperimental=true; // Needed in core profile
  if (glewInit() != GLEW_OK) {
      fprintf(stderr, "Failed to initialize GLEW\n");
      return -1;
  }  
  
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);  
  // Cull triangles which normal is not towards the camera
  glDisable(GL_CULL_FACE);  
  
//   // Enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);    
  
  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, width/2, height/2);
    
  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  //----------------------------------------------------------------------------
  // Setup projection matrix
  //----------------------------------------------------------------------------
  
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);
    
  // Or, for an ortho camera :
  //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
    
  // Camera matrix
  glm::mat4 View = glm::lookAt(
      glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
      glm::vec3(0,0,0), // and looks at the origin
      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
      );
    
  //----------------------------------------------------------------------------
  // Setup shaders
  //----------------------------------------------------------------------------  
  
  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
  GLuint customColorShaderID = LoadShaders( "CustomColor.vertexshader", "CustomColor.fragmentshader" );    
  
  // Get a handle for our "MVP" uniform
  // Only during the initialisation
  GLuint MVP_mID = glGetUniformLocation(programID, "MVP");
  GLuint V_mID = glGetUniformLocation(programID, "V");
  GLuint M_mID = glGetUniformLocation(programID, "M");
  GLuint lightPos_mID = glGetUniformLocation(programID, "LightPosition_worldspace");
  GLuint MatrixIDAxes = glGetUniformLocation(customColorShaderID, "MVP");
  
  //----------------------------------------------------------------------------
  // Setup models
  //----------------------------------------------------------------------------  

  // Axes
  float s = 0.1f;
  static const GLfloat axes_vertex_buffer_data[] = {  -s/2,-s/2,-s/2, s/2,-s/2,-s/2, -s/2,-s/2,-s/2, -s/2,s/2,-s/2, -s/2,-s/2,-s/2, -s/2,-s/2,s/2 };
  static const GLfloat axes_color_buffer_data[]  = { 1,0,0, 1,0,0, 0,1,0, 0,1,0, 0,0,1, 0,0,1 };
  
//   static const GLfloat axes_vertex_buffer_data[] = {  -s/2,-s/2,-s/2, s/2,-s/2,-s/2, -s/2,-s/2,-s/2, -s/2,s/2,-s/2, -s/2,-s/2,-s/2, -s/2,-s/2,s/2,
//                                                       -s/2,s/2,s/2, -s/2,s/2,-s/2, -s/2,s/2,s/2, -s/2,-s/2,s/2, -s/2,s/2,s/2, s/2,s/2,s/2,
//                                                       s/2,s/2,-s/2, s/2,s/2,s/2, s/2,s/2,-s/2, s/2,-s/2,-s/2, s/2,s/2,-s/2, -s/2,s/2,-s/2,
//                                                       s/2,-s/2,s/2, s/2,-s/2,-s/2, s/2,-s/2,s/2, s/2,s/2,s/2, s/2,-s/2,s/2, -s/2,-s/2,s/2,
//                                                       -s/2,-s/2,-s/2, s/2,s/2,s/2, s/2,-s/2,-s/2, -s/2,s/2,s/2, s/2,-s/2,s/2, -s/2,s/2,-s/2, -s/2,-s/2,s/2, s/2,s/2,-s/2
//   };
// //   static const GLfloat axes_color_buffer_data[]  = { 1,0,0, 1,0,0, 0,1,0, 0,1,0, 0,0,1, 0,0,1 };
//   static const GLfloat axes_color_buffer_data[]  = {  1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1,
//                                                       1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1,
//                                                       1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1,
//                                                       1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1,
//                                                       1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1
//   };
  
  // Vertex buffer
  GLuint axesVertexbuffer;
  glGenBuffers(1, &axesVertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, axesVertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertex_buffer_data), axes_vertex_buffer_data, GL_STATIC_DRAW);  
  
  // Color buffer
  GLuint axesColorbuffer;
  glGenBuffers(1, &axesColorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, axesColorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axes_color_buffer_data), axes_color_buffer_data, GL_STATIC_DRAW);  
    
  // Load object
  std::vector< glm::vec3 > vertices_raw;
  std::vector< glm::vec2 > uvs_raw;
  std::vector< glm::vec3 > normals_raw; // Won't be used at the moment.
  bool res = loadOBJ("suzanne.obj", vertices_raw, uvs_raw, normals_raw);
  GLuint Texture = loadDDS("uvmap_suzanne.DDS");
  
  // Index the vertices
  std::vector< glm::vec3 > vertices;
  std::vector< glm::vec2 > uvs;
  std::vector< glm::vec3 > normals; // Won't be used at the moment.  
  std::vector<unsigned short> indices;

  indexVBO(vertices_raw, uvs_raw, normals_raw, indices, vertices, uvs, normals);
  
  // Generate a buffer for the indices
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);  
  
  // Vertex buffer
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);  
    
  // Texture buffer
  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);    
  
  // Normal buffer
  GLuint normalbuffer;
  glGenBuffers(1, &normalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
  
  // Shading
  glm::vec3 lightPosition_cameraSpace (0,0,1);
  glm::vec3 lightPosition_worldspace;
  
  ////////////////////
  // Light source
  std::vector< glm::vec3 > verticesCube;
  std::vector< glm::vec2 > uvsCube;
  std::vector< glm::vec3 > normalsCube; // Won't be used at the moment.

  res = loadOBJ("cube.obj", verticesCube, uvsCube, normalsCube);
  
  for (size_t i = 0; i < verticesCube.size(); i++)
    verticesCube[i] *= 0.001;
  
  // Vertex buffer
  GLuint vertexbufferCube;
  glGenBuffers(1, &vertexbufferCube);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
  glBufferData(GL_ARRAY_BUFFER, verticesCube.size() * sizeof(glm::vec3), &verticesCube[0], GL_STATIC_DRAW);  
  
  // Color buffer
  std::vector< glm::vec3 > colorsCube (verticesCube.size(), glm::vec3(1));
  GLuint colorbufferCube;
  glGenBuffers(1, &colorbufferCube);
  glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
  glBufferData(GL_ARRAY_BUFFER, colorsCube.size() * sizeof(glm::vec3), &colorsCube[0], GL_STATIC_DRAW);  
      
  //----------------------------------------------------------------------------
  // Rendering loop
  //----------------------------------------------------------------------------  
  
  float angle = 0;
  double lastTime = glfwGetTime();
  int nbFrames = 0;
  
  glfwSwapInterval(0); // Disable v-sync, unlock FPS
  
  do{
    
    // Measure speed
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
        // printf and reset timer
        printf("%f ms/frame\n", 1000.0/double(nbFrames));
        nbFrames = 0;
        lastTime += 1.0;
    }
     
    computeMatricesFromInputs();
    glm::mat4 M = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(-1, 0, 0));
    glm::mat4 V = getViewMatrix();
    glm::mat4 MVP = getProjectionMatrix() * V * M;
    
    glm::mat4 AxesPose = getViewMatrix();
    glm::vec4 t;
    t += glm::vec4(0.43, -0.33, -1, 1);
    AxesPose[3] = t; // This just translates the matrix to specified coordinates    
    AxesPose = getProjectionMatrix() * AxesPose;
    
    glm::mat4 V_inv = glm::inverse(getViewMatrix());
    lightPosition_worldspace = vec3(V_inv * vec4(lightPosition_cameraSpace, 1));
//     lightPosition_worldspace = vec3(4, 4, 4);
    
    glm::mat4 cubePose = V_inv;
    cubePose[3] = glm::vec4(lightPosition_worldspace, 1);
    cubePose = getProjectionMatrix() * V * cubePose;
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
    // Draw axes
    glUseProgram(customColorShaderID);
    glUniformMatrix4fv(MatrixIDAxes, 1, GL_FALSE, &AxesPose[0][0]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, axesVertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, axesColorbuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    glDrawArrays(GL_LINES, 0, 2*3);
    
    glUniformMatrix4fv(MatrixIDAxes, 1, GL_FALSE, &cubePose[0][0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    glDrawArrays(GL_TRIANGLES, 0, verticesCube.size());
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Use our shader
    glUseProgram(programID);
    
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MVP_mID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(M_mID, 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(V_mID, 1, GL_FALSE, &V[0][0]);
    glUniform3f(lightPos_mID, lightPosition_worldspace.x, lightPosition_worldspace.y, lightPosition_worldspace.z);
        
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    
    // 2nd attribute buffer : uv coordinates
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    
    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    
    
    // Indices
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
     
//     // 3nd attribute buffer : vertex colors
//     glEnableVertexAttribArray(2);
//     glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
//     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);    

    // Draw the model !
    //glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0); 
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    
    angle = fmod(angle + 0.00f, 360);

  } // Check if the ESC key was pressed or the window was closed
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );  
  
  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
//   glDeleteBuffers(1, &colorbuffer);
  glDeleteProgram(customColorShaderID);
  glDeleteProgram(programID);
  glDeleteTextures(1, &Texture);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();  
  
	return 0;
}

