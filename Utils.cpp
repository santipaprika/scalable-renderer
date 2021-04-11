#include "Utils.h"
#include <algorithm>


float Utils::max3(glm::vec3 v) 
{
    return std::max(v.x, std::max(v.y,v.z));
}
