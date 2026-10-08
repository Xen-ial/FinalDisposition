#include <erebus/utils/ModelLoader.h>

#include <erebus/utils/Model.h>
#include <erebus/utils/Vertex.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <print>

namespace erb
{
	void ModelLoader::parseModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene*   scene = importer.ReadFile(path,
		    aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (scene == nullptr || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || scene->mRootNode == nullptr)
		{
			std::println("ERROR::ASSIMP:: {}", importer.GetErrorString());
			return;
		}

		processNode(scene->mRootNode, scene);
	}

	void ModelLoader::processNode(aiNode* node, const aiScene* scene)
	{
		for (std::uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_model.addMesh(processMesh(mesh, scene));
		}

		for (std::uint32_t i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i], scene);
	}

	Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		Mesh                       resultMesh{};
		std::vector<Vertex>        vertices;
		std::vector<std::uint32_t> indices;

		for (std::uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex    vertex;
			glm::vec3 vert;

			auto& meshVertices = mesh->mVertices[i];
			vert.x             = meshVertices.x;
			vert.y             = meshVertices.y;
			vert.z             = meshVertices.z;
			vertex.position    = vert;

			if (mesh->HasNormals())
			{
				auto& meshNormals = mesh->mNormals[i];
				vert.x            = meshNormals.x;
				vert.y            = meshNormals.y;
				vert.z            = meshNormals.z;
				vertex.normal     = vert;
			}

			if (mesh->HasTextureCoords(0))
			{
				glm::vec2 tex;

				auto& textureCoords = mesh->mTextureCoords[0][i];
				tex.x               = textureCoords.x;
				tex.y               = textureCoords.y;
				vertex.texCoord     = tex;

				auto& tangents = mesh->mTangents[i];
				vert.x         = tangents.x;
				vert.y         = tangents.y;
				vert.z         = tangents.z;
				vertex.tangent = glm::vec4(vert, 1.0F);
			}
			else
				vertex.texCoord = glm::vec2(0.0F, 0.0F);

			vertices.push_back(vertex);
		}

		for (std::uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (std::uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		resultMesh.setIndicies(indices);
		resultMesh.setVerticies(vertices);

		return std::move(resultMesh);
	}
} // namespace erb