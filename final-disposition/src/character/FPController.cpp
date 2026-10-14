#include <character/FPController.h>

#include <erebus/utils/PhysicsManager.h>
#include <erebus/engine/Camera.h>

#include <GLFW/glfw3.h>

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/hash.hpp>

#include <print>

namespace fd
{
	void FPController::init()
	{
		m_fpCamera.setFov(80.0F);
		m_engine->setCamera(m_fpCamera);

		auto& physics = m_engine->getPhysicsManager();

		erb::RenderObject fpController("fpController");
		physics.createPhysicsBody(fpController, erb::PhysicsObjectType::DYNAMIC_OBJECT);
		erb::PhysicsManager::createBoxHullShape(fpController, {1.0F, 2.0F, 1.0F},
		    {.dampening = 0});

		m_engine->addEntity(fpController);
	}

	void FPController::update(float deltaTime)
	{
		syncCameraMovement();
	}

	void FPController::fixedUpdate(float deltaTime)
	{
		updatePhysicalMovement(deltaTime);
	}

	void FPController::syncCameraMovement()
	{
		auto* fpController = getFPRenderObject();
		if (fpController == nullptr)
			return;

		auto& input    = m_engine->getInput();
		auto  mousePos = input.getRelativeMousePosition();

		m_fpCamera.setPosition(fpController->transform.getPosition());
		m_fpCamera.processMouseMovement(mousePos.x, mousePos.y);
	}

	void FPController::updatePhysicalMovement(float deltaTime)
	{
		auto* fpController = getFPRenderObject();
		if (fpController == nullptr)
			return;

		erb::PhysicsManager::wakeBody(*fpController);

		auto& input   = m_engine->getInput();
		auto& physics = m_engine->getPhysicsManager();

		auto  position = fpController->transform.getPosition();

		glm::vec3 originCameraFront = m_fpCamera.getFront();
		glm::vec3 originCameraRight = m_fpCamera.getRight();

		glm::vec3 cameraFront = glm::normalize(glm::vec3(originCameraFront.x, 0.0F, originCameraFront.z));
		glm::vec3 cameraRight = glm::normalize(glm::vec3(originCameraRight.x, 0.0F, originCameraRight.z));

		glm::vec3 moveDir = glm::vec3(0.0F);

		m_hasHitGround = m_isGrounded && !m_wasGrounded;

		if (input.isKeyDown(GLFW_KEY_W))
			moveDir += cameraFront;

		if (input.isKeyDown(GLFW_KEY_S))
			moveDir -= cameraFront;

		if (input.isKeyDown(GLFW_KEY_D))
			moveDir += cameraRight;

		if (input.isKeyDown(GLFW_KEY_A))
			moveDir -= cameraRight;

		m_isRunning = input.isKeyDown(GLFW_KEY_LEFT_SHIFT);
		m_isWalking = input.isKeyDown(GLFW_KEY_W) || input.isKeyDown(GLFW_KEY_S) ||
		    input.isKeyDown(GLFW_KEY_D) || input.isKeyDown(GLFW_KEY_A);

		glm::vec3 currentVelocity    = erb::PhysicsManager::getBodyVelocity(*fpController);
		glm::vec3 horizontalVelocity = glm::vec3(currentVelocity.x, 0.0F, currentVelocity.z);

		const float runSpeed  = m_runSpeed * m_scalar;
		const float walkSpeed = m_walkSpeed * m_scalar;
		const float maxSpeed  = m_maxSpeed * m_scalar;

		if (!m_overrideBaseMovement)
		{
			float baseSpeed = m_isRunning ? runSpeed : walkSpeed;

			m_speed = baseSpeed;
			m_speed = std::min(m_speed, m_isRunning ? maxSpeed * 2 : maxSpeed);
		}

		glm::vec3 targetVelocity = glm::vec3(0.0F);
		if (glm::length(moveDir) > 0)
		{
			moveDir        = glm::normalize(moveDir);
			targetVelocity = glm::vec3(moveDir.x, 0.0F, moveDir.z) * m_speed;
		}

		if (glm::length(targetVelocity) > 0)
		{
			glm::vec3 velocityChange  = targetVelocity - horizontalVelocity;
			float     changeMagnitude = glm::length(velocityChange);

			if (changeMagnitude > 0)
			{
				const float acceleration = m_acceleration * m_scalar;
				float       maxChange    = acceleration * deltaTime;
				velocityChange *= std::min(1.0F, maxChange / changeMagnitude);
			}

			horizontalVelocity += velocityChange;
		}
		else
		{
			float speed = glm::length(horizontalVelocity);
			if (speed > 0)
			{
				const float deAcceleration  = m_deAcceleration * m_scalar;
				float       deAccelStrength = deAcceleration * deltaTime;

				speed              = std::max(0.0F, speed - deAccelStrength);
				horizontalVelocity = glm::normalize(horizontalVelocity) * speed;
			}
		}

		glm::vec3 linearVelocity = glm::vec3(horizontalVelocity.x, currentVelocity.y, horizontalVelocity.z);
		erb::PhysicsManager::setLinearVelocity(*fpController, linearVelocity);

		m_wasGrounded = m_isGrounded;
	}

	erb::RaycastHit FPController::getCameraRaycast()
	{
		auto raycastManager = m_engine->getRaycastManager();
		return raycastManager.castRay(m_fpCamera.getPosition(), m_fpCamera.getFront() * m_interactionDistance);
	}
} // namespace fd