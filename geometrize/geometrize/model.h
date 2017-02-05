#pragma once

#include <cstdint>
#include <vector>

#include "bitmap/bitmap.h"
#include "shaperesult.h"
#include "shape/shapetypes.h"

namespace geometrize
{
class Shape;
}

namespace geometrize
{

/**
 * @brief The Model class is the model for the primitive optimization/fitting algorithm.
 * @author Sam Twidale (http://samcodes.co.uk/)
 */
class Model
{
public:
    Model(const geometrize::Bitmap& target, const geometrize::rgba backgroundColor);

    inline Model() = delete;
    inline ~Model() = default;
    Model& operator=(const Model&) = delete;
    Model(const Model&) = delete;

    /**
     * @brief reset Resets the model back to the state it was in when it was created.
     * @param backgroundColor The starting background color to use.
     */
    void reset(const geometrize::rgba backgroundColor);

    /**
     * @brief getWidth Gets the width of the target bitmap.
     * @return The width of the target bitmap.
     */
    std::uint32_t getWidth() const;

    /**
     * @brief getHeight Gets the height of the target bitmap.
     * @return The height of the target bitmap.
     */
    std::uint32_t getHeight() const;

    /**
     * @brief getAspectRatio Gets the aspect ratio of the target bitmap.
     * @return The aspect ratio of the target bitmap.
     */
    float getAspectRatio() const;

    /**
     * @brief step Steps the primitive optimization/fitting algorithm.
     * @param shapeTypes The types of shape to use.
     * @param alpha The alpha of the shape.
     * @param repeats How many times to repeat the stepping process with reduced search (per step), adding additional shapes.
     * @return A vector containing data about the shapes added to the model in this step.
     */
    std::vector<geometrize::ShapeResult> step(const geometrize::shapes::ShapeTypes shapeTypes, const std::uint8_t alpha, const std::uint32_t repeats);

    /**
     * @brief addShape Adds a shape to the model.
     * @param shape The shape to add.
     * @param alpha The alpha/opacity of the shape.
     * @return Data about the shape added to the model.
     */
    geometrize::ShapeResult addShape(std::shared_ptr<geometrize::Shape> shape, const std::uint8_t alpha);

    /**
     * @brief getCurrent Gets the current bitmap.
     * @return The current bitmap.
     */
    geometrize::Bitmap& getCurrent();

private:
    geometrize::Bitmap m_target; ///< The target bitmap, the bitmap we aim to approximate.
    geometrize::Bitmap m_current; ///< The current bitmap.
    geometrize::Bitmap m_buffer; ///< Buffer bitmap.
    float m_score; ///< Score derived from calculating the difference between bitmaps.
};

}