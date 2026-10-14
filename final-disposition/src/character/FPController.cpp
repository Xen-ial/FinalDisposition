#include "erebus/engine/Camera.h"
#include <character/FPController.h>
#include <erebus/utils/PhysicsManager.h>

#include <GLFW/glfw3.h>
#include <glm/geometric.hpp>
#include <glm/gtx/hash.hpp>
#include <print>

namespace fd
{
	void FPController::init()
	{
		m_engine->getCamera().setFov(80.0F);

		auto& physics = m_engine->getPhysicsManager();

		erb::RenderObject fpController("fpController");
		physics.createPhysicsBody(fpController, erb::PhysicsObjectType::DYNAMIC_OBJECT);
		erb::PhysicsManager::createBoxHullShape(fpController, {1.0F, 2.0F, 1.0F},
		    {.dampening = 0});

		m_engine->addEntity(fpController);
	}

	void FPController::update(float deltaTime)
	{
		auto&        physics     = m_engine->getPhysicsManager();
		static float accumulator = 0.0F;

		accumulator += deltaTime;
		while (accumulator >= physics.getFixedTimeStep())
		{
			updatePhysicalMovement(deltaTime);
			accumulator -= physics.getFixedTimeStep();
		}

		syncCameraMovement();
	}

	void FPController::syncCameraMovement()
	{
		auto* fpController = getFPRenderObject();
		if (fpController == nullptr)
			return;

		auto& input    = m_engine->getInput();
		auto  mousePos = input.getRelativeMousePosition();

		auto& camera = m_engine->getCamera();

		camera.setPosition(fpController->transform.getPosition());
		camera.processMouseMovement(mousePos.x, mousePos.y);
	}

	void FPController::updatePhysicalMovement(float deltaTime)
	{
		auto* fpController = getFPRenderObject();
		if (fpController == nullptr)
			return;

		auto& input   = m_engine->getInput();
		auto& physics = m_engine->getPhysicsManager();

		auto  position = fpController->transform.getPosition();
		auto& camera   = m_engine->getCamera();

		glm::vec3 moveDir       = glm::vec3(0.0F);
		float     movementSpeed = m_movementSpeed;

		glm::vec3 cameraFront = glm::normalize(glm::vec3(camera.getFront().x, 0.0F, camera.getFront().z));
		glm::vec3 cameraRight = glm::normalize(camera.getRight());

		if (input.isKeyDown(GLFW_KEY_W))
			moveDir += cameraFront;

		if (input.isKeyDown(GLFW_KEY_S))
			moveDir -= cameraFront;

		if (input.isKeyDown(GLFW_KEY_D))
			moveDir += cameraRight;

		if (input.isKeyDown(GLFW_KEY_A))
			moveDir -= cameraRight;

		if (input.isKeyDown(GLFW_KEY_LEFT_SHIFT))
			movementSpeed = m_runSpeed;
		else
			movementSpeed = m_movementSpeed;

		if (input.isKeyDown(GLFW_KEY_SPACE))
			moveDir.y += m_jumpHeight;

		glm::vec3 normalizedMoveDir = glm::normalize(moveDir);
		if (glm::length(normalizedMoveDir) > 0)
		{
			erb::PhysicsManager::addLinearVelocity(*fpController,
			    {normalizedMoveDir.x * movementSpeed, 0.0F, normalizedMoveDir.z * movementSpeed});
		}

		position = fpController->transform.getPosition();
		std::println("Pos: {}, {}, {}", position.x, position.y, position.z);
	}

	erb::RaycastHit FPController::getCameraRaycast()
	{
		auto raycastManager = m_engine->getRaycastManager();
		return raycastManager.castRay(m_fpCamera.getPosition(), m_fpCamera.getFront() * m_interactionDistance);
	}
} // namespace fd