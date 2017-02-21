#pragma once

#include <string>
#include <vector>

namespace geometrize
{
struct ShapeResult;
}

namespace geometrize
{

namespace exporter
{

/**
 * @brief exportShapeArray Exports shape data to a compact array-style format.
 * @param data The shape data to export.
 * @return A string containing the exported data.
 */
std::string exportShapeArray(const std::vector<geometrize::ShapeResult>& data);

}

}