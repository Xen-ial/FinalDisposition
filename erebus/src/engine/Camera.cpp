#include <erebus/engine/Camera.h>

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>

namespace erb
{
	void Camera3D::processKeyboard(CameraMovement direction, const float deltaTime)
	{
		const float velocity = m_movementSpeed * deltaTime;
		auto        position = m_transform.getPosition();
		auto        rotation = m_transform.getRotation();

		switch (direction)
		{
		case CameraMovement::FORWARD:
			position += rotation * velocity;
			break;
		case CameraMovement::BACKWARD:
			position -= rotation * velocity;
			break;
		case CameraMovement::LEFT:
			position -= m_right * velocity;
			break;
		case CameraMovement::RIGHT:
			position += m_right * velocity;
			break;
		case CameraMovement::UP:
			position += m_up * velocity;
			break;
		case CameraMovement::DOWN:
			position -= m_up * velocity;
			break;
		}

		updateCameraVectors();
		m_transform.setPosition(position);
	}

	void Camera3D::processMouseMovement(float xOffset, float yOffset, const bool constrainPitch)
	{
		xOffset *= m_mouseSensitivity;
		yOffset *= m_mouseSensitivity;

		m_yaw += xOffset;
		m_pitch += yOffset;

		if (constrainPitch)
		{
			const float maxNegativePitch = -89.0F;
			const float maxPositivePitch = 89.0F;

			m_pitch = std::clamp(m_pitch, maxNegativePitch, maxPositivePitch);
		}

		updateCameraVectors();
	}

	void Camera3D::updateCameraVectors()
	{
		glm::vec3 newRot;

		newRot.x = static_cast<float>(cos(glm::radians(m_yaw))) * static_cast<float>(cos(glm::radians(m_pitch)));
		newRot.y = static_cast<float>(sin(glm::radians(m_pitch)));
		newRot.z = static_cast<float>(sin(glm::radians(m_yaw))) * static_cast<float>(cos(glm::radians(m_pitch)));

		m_transform.setRotation(glm::normalize(newRot));
		auto rotation = m_transform.getRotation();

		m_right = glm::normalize(glm::cross(rotation, m_worldUp));
		m_up    = glm::normalize(glm::cross(m_right, rotation));
	}

	glm::mat4 Camera3D::getViewMatrix() const
	{
		return glm::lookAt(m_transform.getPosition(), m_transform.getPosition() + m_transform.getRotation(), m_up);
	}

	glm::mat4 Camera3D::getProjectionMatrix(const float apsectRatio, const float nearPlane, const float farPlane) const
	{
		return glm::perspective(glm::radians(m_fov), apsectRatio, nearPlane, farPlane);
	}

	void Camera3D::setPosition(const glm::vec3& position)
	{
		m_transform.setPosition(position);
	}

	void Camera3D::setMovementSpeed(const float newSpeed)
	{
		m_movementSpeed = newSpeed;
	}

	void Camera3D::setMouseSensitivity(const float sensitivity)
	{
		m_mouseSensitivity = sensitivity;
	}

	void Camera3D::setFov(const float fov)
	{
		m_fov = fov;
	}
} // namespace erb