#include "primitives/text.hpp"
#include "layout/sizing.hpp"

elements::TextDescriptor::TextDescriptor():
    text{},
    font{},
    fontSize{Size::pt(12.0f)}
{}
