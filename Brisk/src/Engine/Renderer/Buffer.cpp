#include "Buffer.hpp"

namespace Brisk 
{
	std::shared_ptr<Buffer> Buffer::Create() {
		return std::make_shared<Buffer>();
	}
}