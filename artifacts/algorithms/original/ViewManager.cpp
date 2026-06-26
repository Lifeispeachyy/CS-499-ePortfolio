///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport - camera, projection
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Modified by: Lacey Mikolon for CS-330 Final Project
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    // camera object used for viewing and interacting with
    // the 3D scene
    Camera* g_pCamera = nullptr;

    // these variables are used for mouse movement processing
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool  gFirstMouse = true;

    // time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // when false: perspective projection
    // when true : orthographic projection
    bool bOrthographicProjection = false;

    // simple edge-detect for the projection toggle key
    bool gLastPKeyPressed = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    // initialize the member variables
    m_pShaderManager = pShaderManager;
    m_pWindow = NULL;

    // create a new camera instance
    g_pCamera = new Camera();

    // default camera view parameters (pulled back so all
    // objects and lighting are visible in the final scene)
    g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80.0f;   // field of view
    g_pCamera->MovementSpeed = 20.0f;   // base movement speed
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
    // free up allocated memory
    m_pShaderManager = NULL;
    m_pWindow = NULL;

    if (NULL != g_pCamera)
    {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = nullptr;

    // try to create the displayed OpenGL window
    window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowTitle,
        NULL,
        NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    // tell GLFW to capture all mouse events (for free-look camera)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // mouse callbacks for orientation and speed
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);
    glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

    // enable blending for supporting transparent rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 *  It updates the camera orientation (pitch and yaw).
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    // when the first mouse move event is received, this needs to be recorded so that
    // all subsequent mouse moves can correctly calculate the X position offset and Y
    // position offset for proper operation
    if (gFirstMouse)
    {
        gLastX = static_cast<float>(xMousePos);
        gLastY = static_cast<float>(yMousePos);
        gFirstMouse = false;
    }

    // calculate the X offset and Y offset values for moving the 3D camera accordingly
    float xOffset = static_cast<float>(xMousePos) - gLastX;
    float yOffset = gLastY - static_cast<float>(yMousePos); // reversed since y-coordinates go from bottom to top

    // set the current positions into the last position variables
    gLastX = static_cast<float>(xMousePos);
    gLastY = static_cast<float>(yMousePos);

    // move the 3D camera according to the calculated offsets
    if (g_pCamera != nullptr)
    {
        g_pCamera->ProcessMouseMovement(xOffset, yOffset);
    }
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse scroll wheel is used. Here we map scrolling
 *  to the camera movement speed so users can fine-tune
 *  how fast they move around the scene.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
    if (g_pCamera == nullptr)
        return;

    // adjust movement speed using scroll input
    g_pCamera->MovementSpeed += static_cast<float>(yOffset) * 2.0f;

    // clamp to a reasonable range so it does not become too slow or too fast
    if (g_pCamera->MovementSpeed < 2.0f)
    {
        g_pCamera->MovementSpeed = 2.0f;
    }
    if (g_pCamera->MovementSpeed > 80.0f)
    {
        g_pCamera->MovementSpeed = 80.0f;
    }
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 *
 *  WASD -> forward/back/strafe
 *  QE   -> vertical movement (up/down)
 *  P    -> toggle perspective / orthographic projection
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
    // close the window if the escape key has been pressed
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    if (g_pCamera != nullptr)
    {
        // process camera forward and backward
        if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
        }
        if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
        }

        // process camera panning left and right
        if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
        }
        if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
        }

        // vertical movement: Q = up, E = down
        if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        {
            g_pCamera->Position += g_pCamera->Up * g_pCamera->MovementSpeed * gDeltaTime;
        }
        if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        {
            g_pCamera->Position -= g_pCamera->Up * g_pCamera->MovementSpeed * gDeltaTime;
        }
    }

    // handle projection toggle (P key) with edge detection so it only toggles once per tap
    int pKeyState = glfwGetKey(m_pWindow, GLFW_KEY_P);
    if (pKeyState == GLFW_PRESS && !gLastPKeyPressed)
    {
        bOrthographicProjection = !bOrthographicProjection;
    }
    gLastPKeyPressed = (pKeyState == GLFW_PRESS);
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene view by
 *  calculating the view and projection matrices and sending
 *  them to the shader.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
    glm::mat4 view;
    glm::mat4 projection;

    // per-frame timing
    float currentFrame = static_cast<float>(glfwGetTime());
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // process any keyboard events that may be waiting in the 
    // event queue
    ProcessKeyboardEvents();

    // get the current view matrix from the camera
    view = g_pCamera->GetViewMatrix();

    // define the current projection matrix
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

    if (!bOrthographicProjection)
    {
        // standard 3D perspective projection
        projection = glm::perspective(
            glm::radians(g_pCamera->Zoom),
            aspectRatio,
            0.1f,
            100.0f);
    }
    else
    {
        // orthographic projection (2D-style view of the same 3D scene)
        float orthoScale = 10.0f;
        float right = orthoScale * aspectRatio;
        float left = -orthoScale * aspectRatio;
        float top = orthoScale;
        float bottom = -orthoScale;

        projection = glm::ortho(left, right, bottom, top, 0.1f, 100.0f);
    }

    // if the shader manager object is valid
    if (NULL != m_pShaderManager)
    {
        // set the view matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ViewName, view);
        // set the projection matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        // set the view position of the camera into the shader for proper lighting (Phong)
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}