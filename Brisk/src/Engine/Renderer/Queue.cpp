#include "Queue.hpp"

namespace Brisk
{
	std::shared_ptr<Queue> Queue::Create() {
		return std::make_shared<Queue>();
	}
}