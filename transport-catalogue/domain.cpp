#include "domain.h"

Bus::Bus(std::string& bus_name)
    : bus_name(std::move(bus_name)) {
}

Distance::Distance(int value)
    : meters(value) {
}

std::size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*>& pair) const {
    const void* p1 = static_cast<const void*>(pair.first);
    const void* p2 = static_cast<const void*>(pair.second);
    return std::hash<const void*>{}(p1) ^ std::hash<const void*>{}(p2);
}
