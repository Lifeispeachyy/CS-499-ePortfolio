///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// Manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Modified by: Lacey Mikolon for CS-330 Final Project Milestone
//  DESCRIPTION:
//      Replicates the iMac-style monitor from the 2D desk image using
//      multiple basic 3D shapes (boxes and cylinders), and adds additional
//      desk objects (books, mug, and pencil cup) to better match the scene.
///////////////////////////////////////////////////////////////////////////////


#include "SceneManager.h"


#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


#include <glm/gtx/transform.hpp>


// Declaration of global uniform names
namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}


/***********************************************************
*  SceneManager()
*
*  Constructor for the class
***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();


    // Initialize texture info
    for (int i = 0; i < 16; i++)
    {
        m_textureIDs[i].tag = "/0";
        m_textureIDs[i].ID = -1;
    }
    m_loadedTextures = 0;
}


/***********************************************************
*  ~SceneManager()
*
*  Destructor for the class
***********************************************************/
SceneManager::~SceneManager()
{
    m_pShaderManager = NULL;
    delete m_basicMeshes;
    m_basicMeshes = NULL;


    // Release GPU textures we created
    DestroyGLTextures();
}


/***********************************************************
*  CreateGLTexture()
*
*  Load an image from disk, configure OpenGL texture state,
*  generate mipmaps, and store it in the next free slot.
***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0;
    int height = 0;
    int colorChannels = 0;
    GLuint textureID = 0;


    // Flip image vertically so UVs line up with OpenGL
    stbi_set_flip_vertically_on_load(true);


    unsigned char* image = stbi_load(
        filename,
        &width,
        &height,
        &colorChannels,
        0);


    if (image)
    {
        std::cout << "Loaded texture: " << filename
            << " (w=" << width << ", h=" << height
            << ", channels=" << colorChannels << ")\n";


        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);


        // Wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        if (colorChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                width, height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, image);
        }
        else if (colorChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                width, height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        else
        {
            std::cout << "Unsupported channel count: "
                << colorChannels << std::endl;
            stbi_image_free(image);
            glBindTexture(GL_TEXTURE_2D, 0);
            return false;
        }


        glGenerateMipmap(GL_TEXTURE_2D);


        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);


        // Register texture in local table
        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;


        return true;
    }


    std::cout << "Failed to load texture: " << filename << std::endl;
    return false;
}


/***********************************************************
*  BindGLTextures()
*
*  Bind all loaded textures to consecutive texture units.
***********************************************************/
void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}


/***********************************************************
*  DestroyGLTextures()
*
*  Free GPU memory for all textures we created.
***********************************************************/
void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        if (m_textureIDs[i].ID != -1)
        {
            glDeleteTextures(1, &m_textureIDs[i].ID);
            m_textureIDs[i].ID = -1;
        }
    }
}


/***********************************************************
*  FindTextureID()
*
*  Return the OpenGL texture ID associated with a tag.
***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
    int textureID = -1;
    int index = 0;
    bool bFound = false;


    while ((index < m_loadedTextures) && !bFound)
    {
        if (m_textureIDs[index].tag == tag)
        {
            textureID = m_textureIDs[index].ID;
            bFound = true;
        }
        else
        {
            index++;
        }
    }


    return textureID;
}


/***********************************************************
*  FindTextureSlot()
*
*  Return the texture unit slot index for a given tag.
***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
    int textureSlot = -1;
    int index = 0;
    bool bFound = false;


    while ((index < m_loadedTextures) && !bFound)
    {
        if (m_textureIDs[index].tag == tag)
        {
            textureSlot = index;
            bFound = true;
        }
        else
        {
            index++;
        }
    }


    return textureSlot;
}


/***********************************************************
*  FindMaterial()
*
*  Look up an OBJECT_MATERIAL by tag. Currently not used
*  for this texturing milestone, but implemented for
*  completeness.
***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (const auto& m : m_objectMaterials)
    {
        if (m.tag == tag)
        {
            material = m;
            return true;
        }
    }
    return false;
}


/***********************************************************
*  SetTransformations()
*
*  Set model matrix based on scale, rotations, and translation.
***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 scale = glm::scale(scaleXYZ);
    glm::mat4 rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 translation = glm::translate(positionXYZ);


    glm::mat4 modelView = translation * rotationX * rotationY * rotationZ * scale;


    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
    }
}


/***********************************************************
*  SetShaderColor()
*
*  Use solid color for the next draw call (no texture).
***********************************************************/
void SceneManager::SetShaderColor(
    float redColorValue,
    float greenColorValue,
    float blueColorValue,
    float alphaValue)
{
    glm::vec4 currentColor(
        redColorValue,
        greenColorValue,
        blueColorValue,
        alphaValue);


    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}


/***********************************************************
*  SetShaderTexture()
*
*  Enable texturing and choose which texture to use.
***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);


        int slot = FindTextureSlot(textureTag);
        m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
    }
}


/***********************************************************
*  SetTextureUVScale()
*
*  Control tiling / scaling of texture coordinates.
***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}


/***********************************************************
*  SetShaderMaterial()
*
*  Stub implementation for this milestone. In a later
*  lighting-focused task, this would push material
*  properties to the shader.
***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL mat;
    if (FindMaterial(materialTag, mat))
    {
        // Not required for texturing milestone; implemented as a stub.
        (void)mat;
        (void)materialTag;
    }
}


/***********************************************************
*  PrepareScene()
*
*  Load textures, meshes, and configure lighting.
***********************************************************/
void SceneManager::PrepareScene()
{
    // --- Load Textures ---
    CreateGLTexture("textures/gold-seamless-texture.jpg", "desk");
    CreateGLTexture("textures/stainless.jpg", "metal");
    CreateGLTexture("textures/abstract.jpg", "screen");

    BindGLTextures();

    // --- Load Meshes ---
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadConeMesh();

    // --- Setup Lighting ---
    SetupSceneLights();
}

/***********************************************************
*  SetupSceneLights()
*
*  Configure point lights for the 3D scene using
*  the Phong lighting model.
***********************************************************/
void SceneManager::SetupSceneLights()
{
    if (m_pShaderManager == nullptr)
        return;

    // Enable custom Phong lighting in the fragment shader
    m_pShaderManager->setBoolValue(g_UseLightingName, true);

    // Global material settings (used by Phong specular term)
    m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(0.6f, 0.6f, 0.6f));
    m_pShaderManager->setFloatValue("material.shininess", 32.0f);

    /**************************************************************
     * LIGHT 1: Directional Room Light (warm white)
     * - Simulates a ceiling / room light.
     * - Provides soft ambient + diffuse across the whole scene.
     **************************************************************/
    m_pShaderManager->setBoolValue("directionalLight.bActive", true);
    m_pShaderManager->setVec3Value("directionalLight.direction",
        glm::vec3(-0.2f, -1.0f, -0.3f)); // above, slightly left/front

    m_pShaderManager->setVec3Value("directionalLight.ambient",
        glm::vec3(0.12f, 0.11f, 0.10f));
    m_pShaderManager->setVec3Value("directionalLight.diffuse",
        glm::vec3(0.30f, 0.27f, 0.25f));
    m_pShaderManager->setVec3Value("directionalLight.specular",
        glm::vec3(0.35f, 0.32f, 0.30f));

    /**************************************************************
     * LIGHT 2: Blue Point Light (cool monitor / fill)
     * - CLEARLY COLORED (blue).
     * - Placed to the right of the monitor so the blue tint
     *   shows up on the right edges of the screen and desk.
     **************************************************************/
    m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
    m_pShaderManager->setVec3Value("pointLights[0].position",
        glm::vec3(4.0f, 4.0f, -2.0f)); // to the right, above desk

    m_pShaderManager->setVec3Value("pointLights[0].ambient",
        glm::vec3(0.02f, 0.03f, 0.10f));
    m_pShaderManager->setVec3Value("pointLights[0].diffuse",
        glm::vec3(0.20f, 0.40f, 1.00f)); // strong blue cast
    m_pShaderManager->setVec3Value("pointLights[0].specular",
        glm::vec3(0.30f, 0.60f, 1.00f));

    /**************************************************************
     * LIGHT 3: Warm Point Light (desk lamp feel)
     * - CLEARLY WARM (orange).
     * - Placed over the book stack on the left side.
     **************************************************************/
    m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
    m_pShaderManager->setVec3Value("pointLights[1].position",
        glm::vec3(-5.0f, 3.0f, -3.5f)); // above left book area

    m_pShaderManager->setVec3Value("pointLights[1].ambient",
        glm::vec3(0.10f, 0.06f, 0.03f));
    m_pShaderManager->setVec3Value("pointLights[1].diffuse",
        glm::vec3(1.00f, 0.65f, 0.25f)); // warm orange
    m_pShaderManager->setVec3Value("pointLights[1].specular",
        glm::vec3(1.00f, 0.80f, 0.45f));

    /**************************************************************
     * Disable remaining point lights and spotlight so the shader
     * knows not to use them.
     **************************************************************/
    for (int i = 2; i < 5; ++i)
    {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        m_pShaderManager->setBoolValue((base + ".bActive").c_str(), false);
    }

    m_pShaderManager->setBoolValue("spotLight.bActive", false);
}



/***********************************************************
*  RenderScene()
*
*  Build the desk workspace from basic shapes.
*  Complex textured object: monitor + stand, using multiple
*  coordinated textures.
***********************************************************/
void SceneManager::RenderScene()
{
    glm::vec3 scaleXYZ;
    float XrotationDegrees = 0.0f;
    float YrotationDegrees = 0.0f;
    float ZrotationDegrees = 0.0f;
    glm::vec3 positionXYZ;


    const float deskCenterY = -1.6f;


    /******************************************************************
     * DESK SURFACE (TEXTURED, TILED)
     ******************************************************************/
    scaleXYZ = glm::vec3(12.0f, 0.2f, 3.5f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, deskCenterY, -4.0f);


    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    // --- Desk material: stronger specular so light reflects on the plane ---
    m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(1.3f, 1.3f, 1.3f));
    m_pShaderManager->setFloatValue("material.shininess", 160.0f);

    // Tile the desk texture to avoid stretching and add detail
    SetTextureUVScale(3.0f, 2.0f);
    SetShaderTexture("desk");
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * MONITOR OUTER FRAME (METAL TEXTURE)
     ******************************************************************/
    scaleXYZ = glm::vec3(6.0f, 3.5f, 0.3f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, 1.5f, -4.0f);


    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    // --- Metal material: sharper highlights ---
    m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(1.5f, 1.5f, 1.5f));
    m_pShaderManager->setFloatValue("material.shininess", 128.0f);

    // Slight tiling so metal pattern is visible but not stretched
    SetTextureUVScale(1.5f, 1.0f);
    SetShaderTexture("metal");
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * MONITOR INNER SCREEN (SCREEN TEXTURE)
     ******************************************************************/
    scaleXYZ = glm::vec3(5.4f, 2.9f, 0.1f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, 1.5f, -3.8f);


    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    // --- Matte objects: minimal specular so they don't look plastic ---
    m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(0.08f, 0.08f, 0.08f));
    m_pShaderManager->setFloatValue("material.shininess", 8.0f);

    // Screen uses a different texture but is part of the same object.
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderTexture("screen");
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * MONITOR STAND NECK (METAL)
     ******************************************************************/
    scaleXYZ = glm::vec3(0.5f, 1.5f, 0.5f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, -0.2f, -4.3f);


    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);


    SetTextureUVScale(1.0f, 1.0f);
    SetShaderTexture("metal");
    m_basicMeshes->DrawCylinderMesh();


    /******************************************************************
     * MONITOR STAND BASE (METAL)
     ******************************************************************/
    scaleXYZ = glm::vec3(3.2f, 0.25f, 1.4f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, deskCenterY + 0.05f, -4.3f);


    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);


    SetTextureUVScale(1.0f, 1.0f);
    SetShaderTexture("metal");
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * BOOKS STACK (LEFT SIDE) ? COLOR ONLY
     ******************************************************************/
     // Bottom book
    scaleXYZ = glm::vec3(2.5f, 0.25f, 1.2f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = -5.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.2f, -4.2f);

    // --- Matte objects: minimal specular so they don't look plastic ---
    m_pShaderManager->setVec3Value("material.diffuseColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_pShaderManager->setVec3Value("material.specularColor", glm::vec3(0.3f, 0.3f, 0.3f));
    m_pShaderManager->setFloatValue("material.shininess", 16.0f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.96f, 0.93f, 0.86f, 1.0f);
    m_basicMeshes->DrawBoxMesh();


    // Middle book
    scaleXYZ = glm::vec3(2.3f, 0.22f, 1.1f);
    YrotationDegrees = -3.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.45f, -4.15f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.87f, 0.85f, 0.96f, 1.0f);
    m_basicMeshes->DrawBoxMesh();


    // Top book
    scaleXYZ = glm::vec3(2.0f, 0.20f, 1.0f);
    YrotationDegrees = -1.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.65f, -4.1f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.80f, 0.87f, 0.95f, 1.0f);
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * MUG (LEFT) ? COLOR ONLY
     ******************************************************************/
     // Mug body
    scaleXYZ = glm::vec3(0.7f, 1.0f, 0.7f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-3.4f, deskCenterY + 0.7f, -3.8f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.18f, 0.18f, 0.20f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();


    // Handle
    scaleXYZ = glm::vec3(0.25f, 0.55f, 0.15f);
    positionXYZ = glm::vec3(-2.9f, deskCenterY + 0.7f, -3.8f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.20f, 0.20f, 0.22f, 1.0f);
    m_basicMeshes->DrawBoxMesh();


    /******************************************************************
     * PENCIL CUP + PENCILS ? COLOR ONLY
     ******************************************************************/
     // Cup body
    scaleXYZ = glm::vec3(0.6f, 1.1f, 0.6f);
    positionXYZ = glm::vec3(4.2f, deskCenterY + 0.8f, -4.0f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.93f, 0.94f, 0.97f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();


    // Pencil 1 body
    scaleXYZ = glm::vec3(0.07f, 1.1f, 0.07f);
    YrotationDegrees = -5.0f;
    positionXYZ = glm::vec3(4.15f, deskCenterY + 1.4f, -4.0f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.97f, 0.88f, 0.50f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();


    // Pencil 1 tip
    scaleXYZ = glm::vec3(0.12f, 0.25f, 0.12f);
    XrotationDegrees = 180.0f;
    positionXYZ = glm::vec3(4.15f, deskCenterY + 2.0f, -4.0f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);
    m_basicMeshes->DrawConeMesh();


    // Pencil 2 body
    scaleXYZ = glm::vec3(0.07f, 1.0f, 0.07f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 10.0f;
    positionXYZ = glm::vec3(4.25f, deskCenterY + 1.35f, -3.85f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.90f, 0.65f, 0.55f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();


    // Pencil 2 tip
    scaleXYZ = glm::vec3(0.11f, 0.22f, 0.11f);
    XrotationDegrees = 180.0f;
    positionXYZ = glm::vec3(4.25f, deskCenterY + 1.9f, -3.85f);


    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);
    m_basicMeshes->DrawConeMesh();
}
