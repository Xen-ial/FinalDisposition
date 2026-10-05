#include <erebus/utils/Transform.h>

namespace erb
{
	void Transform::updateModelMatrix()
	{
		glm::mat4 translate = glm::translate(glm::mat4(1.0F), m_position);

		glm::quat rotX = glm::angleAxis(m_rotation.x, glm::vec3(1.0F, 0.0F, 0.0F));
		glm::quat rotY = glm::angleAxis(m_rotation.y, glm::vec3(0.0F, 1.0F, 0.0F));
		glm::quat rotZ = glm::angleAxis(m_rotation.z, glm::vec3(0.0F, 0.0F, 1.0F));

		glm::quat quatRotation = rotZ * rotY * rotX;

		glm::mat4 rotation = glm::mat4_cast(quatRotation);
		glm::mat4 scale    = glm::scale(glm::mat4(1.0F), m_scale);

		m_modelMatrix = translate * rotation * scale;
		m_matrixDirty = false;
	}
} // namespace erb