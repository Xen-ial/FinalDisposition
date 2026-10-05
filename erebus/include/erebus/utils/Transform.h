#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace erb
{
	class Transform
	{
	public:
		void setPosition(const glm::vec3& position)
		{
			m_position    = position;
			m_matrixDirty = true;
		}

		void setRotation(const glm::vec3& rotation)
		{
			m_rotation    = rotation;
			m_matrixDirty = true;
		}

		void setScale(const glm::vec3& scale)
		{
			m_scale       = scale;
			m_matrixDirty = true;
		}

		void setUnformScale(float uniformScale)
		{
			m_scale       = glm::vec3(uniformScale);
			m_matrixDirty = true;
		}

		void rotate(const glm::vec3& eularAngles)
		{
			m_rotation += eularAngles;
			m_matrixDirty = true;
		}

		void scale(const glm::vec3& scaleFactors)
		{
			m_scale *= scaleFactors;
			m_matrixDirty = true;
		}

		glm::vec3 getPosition() { return m_position; }
		glm::vec3 getRotation() { return m_rotation; }
		glm::vec3 getScale() { return m_scale; }

		const glm::mat4& getMatrix()
		{
			if (m_matrixDirty) updateModelMatrix();
			return m_modelMatrix;
		}

	private:
		glm::vec3 m_position = {0.0F, 0.0F, 0.0F};
		glm::vec3 m_rotation = {0.0F, 0.0F, 0.0F};
		glm::vec3 m_scale    = {1.0F, 1.0F, 1.0F};

		glm::mat4 m_modelMatrix = glm::mat4(1.0F);
		bool      m_matrixDirty = false;

		void updateModelMatrix();
	};
} // namespace erb