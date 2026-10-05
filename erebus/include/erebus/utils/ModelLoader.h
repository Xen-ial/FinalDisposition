#pragma once
#include <erebus/utils/Model.h>

#include <string>

namespace erb
{
	class ModelLoader
	{
	public:
		ModelLoader() = default;

		void parseModel(const std::string& path);

		Model& getModel() { return m_model; }

	private:
		Model m_model;
	};
} // namespace erb