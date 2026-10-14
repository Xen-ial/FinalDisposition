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
		void fixedUpdate(float deltaTime);

		erb::Camera3D&     getCamera() { return m_fpCamera; }
		erb::RenderObject* getFPRenderObject()
		{
			auto object = m_engine->findRenderObject("fpController");
			if (!object.has_value())
				return nullptr;

			return object.value();
		}

		void                setMouseSensitivity(float sens) { m_mouseSensitivity = sens; }
		[[nodiscard]] float getMouseSens() const { return m_mouseSensitivity; }

	private:
		erb::Camera3D m_fpCamera;

		erb::Engine* m_engine = nullptr;

		float m_scalar = 100.0F;

		float m_walkSpeed{0.25F};
		float m_runSpeed{2.5F};
		float m_jumpHeight{2.0F};

		float m_acceleration{5.0F};
		float m_deAcceleration{10.5F};

		float m_speed{};
		float m_maxSpeed{0.05F};

		float m_mouseSensitivity{2.0F};
		float m_interactionDistance{10.0F};

		bool m_isAbleToMove{true};
		bool m_isWalking{false};
		bool m_isRunning{false};
		bool m_isAllowedToJump{true};
		bool m_isAllowedToRun{true};
		bool m_overrideBaseMovement{false};
		bool m_isGrounded{false};
		bool m_hasHitGround{false};
		bool m_wasGrounded{false};

		void syncCameraMovement();
		void updatePhysicalMovement(float deltaTime);

		erb::RaycastHit getCameraRaycast();
	};
} // namespace fd