#include <character/Player.h>

namespace fd
{
	void Player::init()
	{
		m_fpController = FPController(*m_engine);
		m_fpController.init();
	}
	
	void Player::update(float deltaTime)
	{
		m_fpController.update(deltaTime);
	}
} // namespace fd