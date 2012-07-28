#pragma once

#include "utilities/Platform.h"

#include <functional>

namespace quicktcp {
namespace utilities {

class UTILITIES_API ByteStream;

typedef std::function<ByteStream(const ByteStream&)> RequestToResponse;

}
}