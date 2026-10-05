#include "erebus/utils/Model.h"
#include <erebus/utils/ModelLoader.h>
#include <erebus/utils/Vertex.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <print>
#include <unordered_map>

namespace objParser
{
	erb::Model parseObjModel(const std::string& path)
	{
		tinyobj::attrib_t attrib;

		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
		{
			std::println("failed to load model: {}", path);
			return {};
		}

		std::unordered_map<erb::Vertex, std::uint32_t> uniqueVertices{};
		std::vector<erb::Vertex>                       verticies;
		std::vector<std::uint32_t>                     indicies;

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				erb::Vertex vertex{};

				vertex.pos = {
				    attrib.vertices[(3 * index.vertex_index) + 0],
				    attrib.vertices[(3 * index.vertex_index) + 1],
				    attrib.vertices[(3 * index.vertex_index) + 2]};

				vertex.texCoord = {
				    attrib.texcoords[(2 * index.texcoord_index) + 0],
				    1.0 - attrib.texcoords[(2 * index.texcoord_index) + 1]};

				vertex.color = {1.0F, 1.0F, 1.0F};

				auto [iter, inserted] = uniqueVertices.insert({vertex, static_cast<std::uint32_t>(verticies.size())});
				if (inserted)
					verticies.push_back(vertex);

				indicies.push_back(iter->second);
			}
		}

		erb::Model model;
		model.setVerticies(verticies);
		model.setIndicies(indicies);

		return std::move(model);
	}
} // namespace objParser

namespace erb
{
	void ModelLoader::parseModel(const std::string& path)
	{
		// TODO: Detect if its an obj file then parse if it is, otherwise detect if its a glb and parse that instead
		m_model = objParser::parseObjModel(path);
	}
} // namespace erb