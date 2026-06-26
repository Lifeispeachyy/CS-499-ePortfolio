///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//
//  ENHANCEMENT:
//  Added Octree spatial partitioning structures to support improved scene
//  organization and more efficient spatial lookup for larger 3D scenes.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

	// Axis-Aligned Bounding Box used for Octree spatial partitioning.
	// This defines a 3D region using minimum and maximum coordinates.
	struct AABB
	{
		glm::vec3 minBounds;
		glm::vec3 maxBounds;

		bool Contains(const glm::vec3& point) const
		{
			return point.x >= minBounds.x && point.x <= maxBounds.x &&
				point.y >= minBounds.y && point.y <= maxBounds.y &&
				point.z >= minBounds.z && point.z <= maxBounds.z;
		}

		bool Intersects(const AABB& other) const
		{
			return minBounds.x <= other.maxBounds.x && maxBounds.x >= other.minBounds.x &&
				minBounds.y <= other.maxBounds.y && maxBounds.y >= other.minBounds.y &&
				minBounds.z <= other.maxBounds.z && maxBounds.z >= other.minBounds.z;
		}
	};

	// Represents an object in the 3D scene that can be inserted
	// into the Octree based on its spatial position.
	struct SCENE_OBJECT
	{
		std::string objectName;
		glm::vec3 position;
		glm::vec3 scale;
	};

	// Octree node used to organize scene objects by 3D spatial region.
	// This improves scalability by avoiding simple linear searches
	// through all scene objects as the scene grows.
	struct OCTREE_NODE
	{
		AABB bounds;
		std::vector<SCENE_OBJECT> objects;
		std::array<std::unique_ptr<OCTREE_NODE>, 8> children;
		bool isSubdivided = false;
	};

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// Enhanced data structure: Octree root node for spatial partitioning
	std::unique_ptr<OCTREE_NODE> m_sceneOctree;

	// Scene object collection used for Octree insertion and spatial queries
	std::vector<SCENE_OBJECT> m_sceneObjects;

	// Build the Octree structure for scene object organization
	void BuildSceneOctree();

	// Insert a scene object into the Octree based on its position
	void InsertObjectIntoOctree(OCTREE_NODE* node, const SCENE_OBJECT& object, int depth);

	// Split an Octree node into eight child regions
	void SubdivideOctreeNode(OCTREE_NODE* node);

	// Query the Octree for objects inside or near a selected region
	void QueryOctree(const OCTREE_NODE* node, const AABB& queryBounds, std::vector<SCENE_OBJECT>& results);

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		float XrotationDegrees,
		float YrotationDegrees,
		float ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		float redColorValue,
		float greenColorValue,
		float blueColorValue,
		float alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);

public:

	// The following methods are for the students to 
	// customize for their own 3D scene
	void PrepareScene();
	void RenderScene();

};