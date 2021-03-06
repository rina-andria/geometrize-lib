#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "shaperesult.h"
#include "shape/shapemutator.h"
#include "shape/shapetypes.h"

namespace geometrize
{
class Bitmap;
class Shape;
}

namespace geometrize
{

/**
 * @brief The Model class is the model for the core optimization/fitting algorithm.
 * @author Sam Twidale (http://samcodes.co.uk/)
 */
class Model
{
public:
    /**
     * @brief Model Creates a model that will aim to replicate the target bitmap with shapes.
     * @param target The target bitmap to replicate with shapes.
     */
    Model(const geometrize::Bitmap& target);

    /**
     * @brief Model Creates a model that will optimize for the given target bitmap, starting from the given initial bitmap.
     * The target bitmap and initial bitmap must be the same size (width and height).
     * @param target The target bitmap to replicate with shapes.
     * @param initial The starting bitmap.
     */
    Model(const geometrize::Bitmap& target, const geometrize::Bitmap& initial);
    ~Model();
    Model& operator=(const Model&) = delete;
    Model(const Model&) = delete;

    /**
     * @brief reset Resets the model back to the state it was in when it was created.
     * @param backgroundColor The starting background color to use.
     */
    void reset(geometrize::rgba backgroundColor);

    /**
     * @brief getWidth Gets the width of the target bitmap.
     * @return The width of the target bitmap.
     */
    std::int32_t getWidth() const;

    /**
     * @brief getHeight Gets the height of the target bitmap.
     * @return The height of the target bitmap.
     */
    std::int32_t getHeight() const;

    /**
     * @brief step Steps the primitive optimization/fitting algorithm.
     * @param shapeTypes The types of shape to use.
     * @param alpha The alpha of the shape.
     * @param shapeCount The number of random shapes to generate (only 1 is chosen in the end).
     * @param maxShapeMutations The maximum number of times to mutate each random shape.
     * @param maxThreads The maximum number of threads to use during this step.
     * @return A vector containing data about the shapes added to the model in this step.
     */
    std::vector<geometrize::ShapeResult> step(
            geometrize::ShapeTypes shapeTypes,
            std::uint8_t alpha,
            std::uint32_t shapeCount,
            std::uint32_t maxShapeMutations,
            std::uint32_t maxThreads);

    /**
     * @brief drawShape Draws a shape on the model. The appropriate color to use is determined by the model.
     * @param shape The shape to draw.
     * @param alpha The alpha/opacity of the shape.
     * @return Data about the shape drawn on the model.
     */
    geometrize::ShapeResult drawShape(std::shared_ptr<geometrize::Shape> shape, std::uint8_t alpha);

    /**
     * @brief drawShape Draws a shape on the model.
     * @param shape The shape to draw.
     * @param color The color (including alpha) of the shape.
     * @return Data about the shape drawn on the model.
     */
    geometrize::ShapeResult drawShape(std::shared_ptr<geometrize::Shape> shape, geometrize::rgba color);

    /**
     * @brief getCurrent Gets the current bitmap.
     * @return The current bitmap.
     */
    geometrize::Bitmap& getCurrent();

    /**
     * @brief getTarget Gets the target bitmap.
     * @return The target bitmap.
     */
    geometrize::Bitmap& getTarget();

    /**
     * @brief getCurrent Gets the current bitmap, const-edition.
     * @return The current bitmap.
     */
    const geometrize::Bitmap& getCurrent() const;

    /**
     * @brief getTarget Gets the target bitmap, const-edition.
     * @return The target bitmap.
     */
    const geometrize::Bitmap& getTarget() const;

    /**
     * @brief setSeed Sets the seed that the random number generators of this model use. Note that the model also uses an internal seed offset which is incremented when the model is stepped.
     * @param seed The random number generator seed.
     */
    void setSeed(std::uint32_t seed);

    /**
     * @brief getShapeMutator Gets the object the model uses for setting up/mutating shapes.
     * @return The shape mutator.
     */
    const geometrize::ShapeMutator& getShapeMutator() const;

    /**
     * @brief getShapeMutator Gets the object the model uses for setting up/mutating shapes.
     * @return The shape mutator.
     */
    geometrize::ShapeMutator& getShapeMutator();

    /**
     * @brief setupShape Performs the initial setup on a shape.
     * @param shape The shape to set up.
     */
    template<typename T>
    void setupShape(T& shape) const
    {
        getShapeMutator().setup(shape);
    }

    /**
     * @brief mutateShape Mutates the given shape.
     * @param shape The shape to mutate.
     */
    template<typename T>
    void mutateShape(T& shape) const
    {
        getShapeMutator().mutate(shape);
    }

    /**
     * @brief setShapeSetupFunction Sets the setup function for the type of shape passed as a parameter in the passed function.
     * @param func The shape setup function to use for the type of shape this function accepts.
     */
    template<typename T>
    void setShapeSetupFunction(const T& func)
    {
        getShapeMutator().setMutatorFunction(func);
    }

    /**
     * @brief setShapeMutatorFunction Sets the mutation function for the type of shape passed as a parameter in the passed function.
     * @param func The shape mutation function to use for the type of shape this function accepts.
     */
    template<typename T>
    void setShapeMutatorFunction(const T& func)
    {
        getShapeMutator().setMutatorFunction(func);
    }

private:
    class ModelImpl;
    std::unique_ptr<Model::ModelImpl> d;
};

}
