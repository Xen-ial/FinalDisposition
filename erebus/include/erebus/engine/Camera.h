#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <cstdint>

namespace erb
{
	enum class CameraMovement : std::uint8_t
	{
		FORWARD = 0,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class Camera3D
	{
	public:
		Camera3D(glm::vec3 position = glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0F),
		    float yaw = -90.0F, float pitch = 0.0F)
		    : m_position(position), m_up(up), m_yaw(yaw), m_pitch(pitch) {};

		void processKeyboard(CameraMovement direction, float deltaTime);
		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
		void processMouseScroll(float yOffset);

		void setPosition(const glm::vec3& position);
		void setMovementSpeed(float newSpeed);
		void setMouseSensitivity(float sensitivity);
		void setFov(float fov);

		[[nodiscard]] glm::vec3 getPosition() const { return m_position; }
		[[nodiscard]] glm::vec3 getFront() const { return m_front; }
		[[nodiscard]] glm::vec3 getUp() const { return m_up; }
		[[nodiscard]] glm::vec3 getRight() const { return m_right; }

		[[nodiscard]] float getFOV() const { return m_fov; }
		[[nodiscard]] float getMouseSensitvitiy() const { return m_mouseSensitivity; }
		[[nodiscard]] float getMovementSpeed() const { return m_movementSpeed; }

		[[nodiscard]] glm::mat4 getViewMatrix() const;
		[[nodiscard]] glm::mat4 getProjectionMatrix(float apsectRatio, float nearPlane = 0.1F, float farPlane = 100.0F) const;

	private:
		glm::vec3 m_position{glm::vec3(0.0F, 0.0F, 0.0F)};
		glm::vec3 m_front{glm::vec3(0.0F)};
		glm::vec3 m_up{glm::vec3(0.0F, 1.0F, 0.0F)};
		glm::vec3 m_right{glm::vec3(0.0F)};
		glm::vec3 m_worldUp{glm::vec3(0.0F, 1.0F, 0.0F)};

		float m_yaw{-90.0F};
		float m_pitch{};

		float m_movementSpeed{20.0F};
		float m_mouseSensitivity{1.0F};

		float m_fov{60.0F};

		void updateCameraVectors();
	};
} // namespace erb