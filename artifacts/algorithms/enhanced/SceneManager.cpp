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
#include <iostream>

// Declaration of global variables / uniform names
namespace
{
    const char* g_ModelName        = "model";
    const char* g_ColorValueName   = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName   = "bUseTexture";
    const char* g_UseLightingName  = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  Constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes    = new ShapeMeshes();
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
}

/***********************************************************
 *  SetTransformations()
 *
 *  Set the model matrix using scale, rotation, and translation.
 *  NOTE: Conceptually we think in this order:
 *      1. Scale   (size)
 *      2. Rotate  (orientation)
 *      3. Translate (position)
 ***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 modelView;
    glm::mat4 scale;
    glm::mat4 rotationX;
    glm::mat4 rotationY;
    glm::mat4 rotationZ;
    glm::mat4 translation;

    // Scale the mesh
    scale = glm::scale(scaleXYZ);

    // Rotate around X, Y, Z
    rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));

    // Translate into final position
    translation = glm::translate(positionXYZ);

    // Final model matrix: T * Rx * Ry * Rz * S
    modelView = translation * rotationX * rotationY * rotationZ * scale;

    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
    }
}

/***********************************************************
 *  SetShaderColor()
 *
 *  Set the object color for the next draw call.
 ***********************************************************/
void SceneManager::SetShaderColor(
    float redColorValue,
    float greenColorValue,
    float blueColorValue,
    float alphaValue)
{
    glm::vec4 currentColor;

    currentColor.r = redColorValue;
    currentColor.g = greenColorValue;
    currentColor.b = blueColorValue;
    currentColor.a = alphaValue;

    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
    }
}

/**************************************************************/
// STUDENTS MODIFY the methods BELOW to prepare and render
// their own 3D replicated scenes.
/**************************************************************/

/***********************************************************
 *  BuildSceneOctree()
 *
 *  Enhancement for CS-499 Algorithms and Data Structures:
 *  Builds an Octree spatial partitioning structure for the
 *  objects in the 3D scene. This organizes objects by position
 *  rather than relying only on a linear sequence of draw calls.
 ***********************************************************/
void SceneManager::BuildSceneOctree()
{
    // Clear previous scene object data
    m_sceneObjects.clear();

    // Define the overall 3D region occupied by the scene
    AABB sceneBounds;
    sceneBounds.minBounds = glm::vec3(-10.0f, -5.0f, -10.0f);
    sceneBounds.maxBounds = glm::vec3(10.0f, 5.0f, 5.0f);

    m_sceneOctree = std::make_unique<OCTREE_NODE>();
    m_sceneOctree->bounds = sceneBounds;

    const float deskCenterY = -1.6f;

    // Register major scene objects with names, positions, and scale values.
    // These objects are inserted into the Octree for spatial organization.
    m_sceneObjects.push_back({ "Desk Surface", glm::vec3(0.0f, deskCenterY, -4.0f), glm::vec3(12.0f, 0.2f, 3.5f) });
    m_sceneObjects.push_back({ "Monitor Frame", glm::vec3(0.0f, 1.5f, -4.0f), glm::vec3(6.0f, 3.5f, 0.3f) });
    m_sceneObjects.push_back({ "Monitor Screen", glm::vec3(0.0f, 1.5f, -3.8f), glm::vec3(5.4f, 2.9f, 0.1f) });
    m_sceneObjects.push_back({ "Monitor Stand Neck", glm::vec3(0.0f, -0.2f, -4.3f), glm::vec3(0.5f, 1.5f, 0.5f) });
    m_sceneObjects.push_back({ "Monitor Stand Base", glm::vec3(0.0f, deskCenterY + 0.05f, -4.3f), glm::vec3(3.2f, 0.25f, 1.4f) });

    m_sceneObjects.push_back({ "Bottom Book", glm::vec3(-5.0f, deskCenterY + 0.2f, -4.2f), glm::vec3(2.5f, 0.25f, 1.2f) });
    m_sceneObjects.push_back({ "Middle Book", glm::vec3(-5.0f, deskCenterY + 0.45f, -4.15f), glm::vec3(2.3f, 0.22f, 1.1f) });
    m_sceneObjects.push_back({ "Top Book", glm::vec3(-5.0f, deskCenterY + 0.65f, -4.1f), glm::vec3(2.0f, 0.20f, 1.0f) });

    m_sceneObjects.push_back({ "Mug Body", glm::vec3(-3.4f, deskCenterY + 0.7f, -3.8f), glm::vec3(0.7f, 1.0f, 0.7f) });
    m_sceneObjects.push_back({ "Mug Handle", glm::vec3(-2.9f, deskCenterY + 0.7f, -3.8f), glm::vec3(0.25f, 0.55f, 0.15f) });

    m_sceneObjects.push_back({ "Pencil Cup", glm::vec3(4.2f, deskCenterY + 0.8f, -4.0f), glm::vec3(0.6f, 1.1f, 0.6f) });
    m_sceneObjects.push_back({ "Pencil 1 Body", glm::vec3(4.15f, deskCenterY + 1.4f, -4.0f), glm::vec3(0.07f, 1.1f, 0.07f) });
    m_sceneObjects.push_back({ "Pencil 1 Tip", glm::vec3(4.15f, deskCenterY + 2.0f, -4.0f), glm::vec3(0.12f, 0.25f, 0.12f) });
    m_sceneObjects.push_back({ "Pencil 2 Body", glm::vec3(4.25f, deskCenterY + 1.35f, -3.85f), glm::vec3(0.07f, 1.0f, 0.07f) });
    m_sceneObjects.push_back({ "Pencil 2 Tip", glm::vec3(4.25f, deskCenterY + 1.9f, -3.85f), glm::vec3(0.11f, 0.22f, 0.11f) });

    // Insert all registered scene objects into the Octree.
    for (const SCENE_OBJECT& object : m_sceneObjects)
    {
        InsertObjectIntoOctree(m_sceneOctree.get(), object, 3);
    }

    std::cout << "Octree spatial partitioning initialized with "
              << m_sceneObjects.size()
              << " scene objects." << std::endl;
}

/***********************************************************
 *  InsertObjectIntoOctree()
 *
 *  Inserts a scene object into the appropriate Octree node.
 *  This replaces a simple linear organization with spatial
 *  grouping based on object position.
 ***********************************************************/
void SceneManager::InsertObjectIntoOctree(OCTREE_NODE* node, const SCENE_OBJECT& object, int depth)
{
    if (node == nullptr)
    {
        return;
    }

    if (!node->bounds.Contains(object.position))
    {
        return;
    }

    // Stop subdividing when maximum depth is reached
    if (depth <= 0)
    {
        node->objects.push_back(object);
        return;
    }

    // Keep a small number of objects in the current node before subdividing
    if (node->objects.size() < 2 && !node->isSubdivided)
    {
        node->objects.push_back(object);
        return;
    }

    if (!node->isSubdivided)
    {
        SubdivideOctreeNode(node);

        // Reinsert existing objects into child nodes when possible
        std::vector<SCENE_OBJECT> existingObjects = node->objects;
        node->objects.clear();

        for (const SCENE_OBJECT& existingObject : existingObjects)
        {
            InsertObjectIntoOctree(node, existingObject, depth - 1);
        }
    }

    // Attempt to insert the object into one of the child nodes
    bool insertedIntoChild = false;

    for (auto& child : node->children)
    {
        if (child && child->bounds.Contains(object.position))
        {
            InsertObjectIntoOctree(child.get(), object, depth - 1);
            insertedIntoChild = true;
            break;
        }
    }

    // If an object does not fit cleanly into a child, keep it in the current node
    if (!insertedIntoChild)
    {
        node->objects.push_back(object);
    }
}

/***********************************************************
 *  SubdivideOctreeNode()
 *
 *  Splits one Octree node into eight child regions.
 ***********************************************************/
void SceneManager::SubdivideOctreeNode(OCTREE_NODE* node)
{
    if (node == nullptr)
    {
        return;
    }

    glm::vec3 min = node->bounds.minBounds;
    glm::vec3 max = node->bounds.maxBounds;
    glm::vec3 center = (min + max) * 0.5f;

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 childMin = min;
        glm::vec3 childMax = center;

        if (i & 1)
        {
            childMin.x = center.x;
            childMax.x = max.x;
        }

        if (i & 2)
        {
            childMin.y = center.y;
            childMax.y = max.y;
        }

        if (i & 4)
        {
            childMin.z = center.z;
            childMax.z = max.z;
        }

        node->children[i] = std::make_unique<OCTREE_NODE>();
        node->children[i]->bounds.minBounds = childMin;
        node->children[i]->bounds.maxBounds = childMax;
    }

    node->isSubdivided = true;
}

/***********************************************************
 *  QueryOctree()
 *
 *  Searches the Octree for objects within a query region.
 *  This demonstrates how spatial partitioning can reduce
 *  unnecessary checks compared to scanning every scene object.
 ***********************************************************/
void SceneManager::QueryOctree(const OCTREE_NODE* node, const AABB& queryBounds, std::vector<SCENE_OBJECT>& results)
{
    if (node == nullptr)
    {
        return;
    }

    if (!node->bounds.Intersects(queryBounds))
    {
        return;
    }

    for (const SCENE_OBJECT& object : node->objects)
    {
        if (queryBounds.Contains(object.position))
        {
            results.push_back(object);
        }
    }

    if (node->isSubdivided)
    {
        for (const auto& child : node->children)
        {
            if (child)
            {
                QueryOctree(child.get(), queryBounds, results);
            }
        }
    }
}

/***********************************************************
 *  PrepareScene()
 *
 *  Load the shapes needed for this milestone. Each mesh is
 *  loaded once and can be drawn many times.
 ***********************************************************/
void SceneManager::PrepareScene()
{
    // Basic box mesh – used for:
    //  - Monitor frame and inner screen
    //  - Monitor stand base
    //  - Books stack
    //  - Mug handle
    m_basicMeshes->LoadBoxMesh();

    // Cylinder mesh – used for:
    //  - Monitor stand neck
    //  - Mug body
    //  - Pencil cup and pencil bodies
    m_basicMeshes->LoadCylinderMesh();

    // Cone mesh – used for:
    //  - Pencil tips
    m_basicMeshes->LoadConeMesh();

    // CS-499 Enhancement:
    // Build Octree structure to organize scene objects spatially.
    BuildSceneOctree();
}


/***********************************************************
 *  RenderScene()
 *
 *  Construct the desk scene from multiple basic shapes:
 *      - Complex monitor object (boxes + cylinder)
 *      - Desk surface (box)
 *      - Books stack (boxes)
 *      - Mug (cylinder + box)
 *      - Pencil cup with pencils (cylinders + cones)
 ***********************************************************/
void SceneManager::RenderScene()
{
    // Shared transform variables
    glm::vec3 scaleXYZ;
    float XrotationDegrees = 0.0f;
    float YrotationDegrees = 0.0f;
    float ZrotationDegrees = 0.0f;
    glm::vec3 positionXYZ;

    // Constant for approximate desk height (center of desk box)
    const float deskCenterY = -1.6f;

    /******************************************************************
     * DESK SURFACE (BOX)
     *
     * Wide, thin box under the monitor to suggest the desktop.
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

    // Slightly warm light gray desk surface
    SetShaderColor(0.50f, 0.30f, 0.15f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
     * MONITOR OUTER FRAME (BOX)
     ******************************************************************/
    scaleXYZ = glm::vec3(6.0f, 3.5f, 0.3f);         // width, height, depth
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(0.0f, 1.5f, -4.0f);    // centered above the desk

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    // Dark gray frame (similar to the black bezel in the photo)
    SetShaderColor(0.12f, 0.12f, 0.14f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
     * MONITOR INNER SCREEN (BOX)
     *
     * Slightly smaller white rectangle inset into the outer frame.
     ******************************************************************/
    scaleXYZ = glm::vec3(5.4f, 2.9f, 0.1f);         // slightly smaller than frame
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    // Move slightly forward on Z so it sits "on top" of the frame
    positionXYZ = glm::vec3(0.0f, 1.5f, -3.8f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    SetShaderColor(0.55f, 0.80f, 1.0f, 1.0f);  // light blue screen
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
 * MONITOR STAND NECK (CYLINDER)
 *
 * Moved slightly behind the monitor (more negative Z) so it
 * reads as being *behind* the screen instead of inside it.
 ******************************************************************/
scaleXYZ = glm::vec3(0.5f, 1.5f, 0.5f);         // slim vertical column
XrotationDegrees = 0.0f;                        // cylinder is vertical
YrotationDegrees = 0.0f;
ZrotationDegrees = 0.0f;
// Place neck behind the monitor, aligned with the stand base
positionXYZ = glm::vec3(0.0f, -0.2f, -4.3f);

SetTransformations(
    scaleXYZ,
    XrotationDegrees,
    YrotationDegrees,
    ZrotationDegrees,
    positionXYZ);

// Cool light gray stand neck (aluminum-like)
SetShaderColor(0.82f, 0.83f, 0.86f, 1.0f);
m_basicMeshes->DrawCylinderMesh();

    /******************************************************************
     * MONITOR STAND BASE (BOX)
     *
     * NOTE: Z is slightly more negative than the frame so the base
     *       appears *behind* the monitor from this camera view.
     ******************************************************************/
    scaleXYZ = glm::vec3(3.2f, 0.25f, 1.4f);       // wide and shallow
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

    // Same light gray as the neck for consistency
    SetShaderColor(0.82f, 0.83f, 0.86f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
     * BOOKS STACK (LEFT SIDE) – all boxes
     *
     * Three flat boxes on the left side of the monitor to suggest
     * the closed books/notebooks in the reference image.
     ******************************************************************/

    // Bottom book 
    scaleXYZ = glm::vec3(2.5f, 0.25f, 1.2f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = -5.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.2f, -4.2f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.75f, 0.12f, 0.12f, 1.0f);  // red book
    m_basicMeshes->DrawBoxMesh();

    // Middle book 
    scaleXYZ = glm::vec3(2.3f, 0.22f, 1.1f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = -3.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.45f, -4.15f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.12f, 0.55f, 0.25f, 1.0f);  // green book
    m_basicMeshes->DrawBoxMesh();

    // Top book
    scaleXYZ = glm::vec3(2.0f, 0.20f, 1.0f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = -1.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-5.0f, deskCenterY + 0.65f, -4.1f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.15f, 0.35f, 0.85f, 1.0f);  // blue book
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
     * MUG (LEFT OF MONITOR) – cylinder body + box handle
     ******************************************************************/

    // Mug body (cylinder)
    scaleXYZ = glm::vec3(0.7f, 1.0f, 0.7f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-3.4f, deskCenterY + 0.7f, -3.8f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    SetShaderColor(0.55f, 0.08f, 0.10f, 1.0f);  // dark red mug
    m_basicMeshes->DrawCylinderMesh();

    // Mug handle (box)
    scaleXYZ = glm::vec3(0.25f, 0.55f, 0.15f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(-2.9f, deskCenterY + 0.7f, -3.8f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.45f, 0.06f, 0.08f, 1.0f);  // dark red handle
    m_basicMeshes->DrawBoxMesh();

    /******************************************************************
     * PENCIL CUP (RIGHT OF MONITOR) – cylinder + small pencils
     ******************************************************************/

    // Cup body
    scaleXYZ = glm::vec3(0.6f, 1.1f, 0.6f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(4.2f, deskCenterY + 0.8f, -4.0f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
        
    SetShaderColor(0.20f, 0.40f, 0.90f, 1.0f);  // blue pencil cup
    m_basicMeshes->DrawCylinderMesh();

    // Pencil 1 – body (cylinder)
    scaleXYZ = glm::vec3(0.07f, 1.1f, 0.07f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = -5.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(4.15f, deskCenterY + 1.4f, -4.0f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.97f, 0.88f, 0.50f, 1.0f);   // classic yellow pencil
    m_basicMeshes->DrawCylinderMesh();

    // Pencil 1 – tip (cone)
    scaleXYZ = glm::vec3(0.12f, 0.25f, 0.12f);
    XrotationDegrees = 180.0f;   // point upward
    YrotationDegrees = -5.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(4.15f, deskCenterY + 2.0f, -4.0f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);   // dark tip
    m_basicMeshes->DrawConeMesh();

    // Pencil 2 – body
    scaleXYZ = glm::vec3(0.07f, 1.0f, 0.07f);
    XrotationDegrees = 0.0f;
    YrotationDegrees = 10.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(4.25f, deskCenterY + 1.35f, -3.85f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    // Slight reddish/brown pencil for variation
    SetShaderColor(0.90f, 0.65f, 0.55f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Pencil 2 – tip
    scaleXYZ = glm::vec3(0.11f, 0.22f, 0.11f);
    XrotationDegrees = 180.0f;
    YrotationDegrees = 10.0f;
    ZrotationDegrees = 0.0f;
    positionXYZ = glm::vec3(4.25f, deskCenterY + 1.9f, -3.85f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);
    SetShaderColor(0.25f, 0.25f, 0.25f, 1.0f);
    m_basicMeshes->DrawConeMesh();
}
