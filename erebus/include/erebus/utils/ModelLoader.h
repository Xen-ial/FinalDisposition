#pragma once
#include <erebus/utils/Model.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

namespace erb
{
	class ModelLoader
	{
	public:
		ModelLoader() = default;

		void   parseModel(const std::string& path);
		Model& getModel() { return m_model; }

	private:
		Model m_model;

		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	};
} // namespace erb