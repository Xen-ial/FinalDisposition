#pragma once
#include <erebus/engine/Camera.h>
#include <erebus/engine/Engine.h>
#include <erebus/vulkan/RenderObject.h>

namespace fd
{
	class FPController
	{
	public:
		FPController() = default;
		FPController(erb::Engine& engine) : m_engine(&engine) {}

		void init();
		void update(float deltaTime);

		erb::Camera3D&     getCamera() { return m_fpCamera; }
		erb::RenderObject* getFPRenderObject()
		{
			auto object = m_engine->findRenderObject("fpController");
			if (!object.has_value())
				return nullptr;

			return object.value();
		}

		void setMovementSpeed(float speed) { m_movementSpeed = speed; }
		void setMouseSensitivity(float sens) { m_mouseSensitivity = sens; }

		[[nodiscard]] float getMouseSens() const { return m_mouseSensitivity; }
		[[nodiscard]] float getMovementSpeed() const { return m_movementSpeed; }

	private:
		erb::Camera3D m_fpCamera;

		erb::Engine* m_engine = nullptr;

		float m_movementSpeed{0.25F};
		float m_runSpeed{2.0F};
		float m_jumpHeight{2.0F};

		float m_mouseSensitivity{2.0F};
		float m_interactionDistance{10.0F};

		void syncCameraMovement();
		void updatePhysicalMovement(float deltaTime);

		erb::RaycastHit getCameraRaycast();
	};
} // namespace fd