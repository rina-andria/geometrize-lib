#include "model.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <future>
#include <memory>
#include <vector>

#include "bitmap/bitmap.h"
#include "commonutil.h"
#include "core.h"
#include "rasterizer/rasterizer.h"
#include "shape/shape.h"
#include "shaperesult.h"
#include "shape/shapemutator.h"
#include "shape/shapetypes.h"

namespace geometrize
{

class Model::ModelImpl
{
public:
    ModelImpl(geometrize::Model* pQ, const geometrize::Bitmap& target) :
        q{pQ},
        m_target{target},
        m_current{target.getWidth(), target.getHeight(), geometrize::commonutil::getAverageImageColor(m_target)},
        m_lastScore{geometrize::core::differenceFull(m_target, m_current)},
        m_baseRandomSeed{0U},
        m_randomSeedOffset{0U}
    {}

    ModelImpl(geometrize::Model* pQ, const geometrize::Bitmap& target, const geometrize::Bitmap& initial) :
        q{pQ},
        m_target{target},
        m_current{initial},
        m_lastScore{geometrize::core::differenceFull(m_target, m_current)},
        m_baseRandomSeed{0U},
        m_randomSeedOffset{0U}
    {
        assert(m_target.getWidth() == m_current.getWidth());
        assert(m_target.getHeight() == m_current.getHeight());
    }

    ~ModelImpl() = default;
    ModelImpl& operator=(const ModelImpl&) = delete;
    ModelImpl(const ModelImpl&) = delete;

    void reset(const geometrize::rgba backgroundColor)
    {
        m_current.fill(backgroundColor);
        m_lastScore = geometrize::core::differenceFull(m_target, m_current);
    }

    std::int32_t getWidth() const
    {
        return m_target.getWidth();
    }

    std::int32_t getHeight() const
    {
        return m_target.getHeight();
    }

    std::vector<geometrize::State> getHillClimbState(
            const geometrize::ShapeTypes shapeTypes,
            const std::uint8_t alpha,
            const std::uint32_t shapeCount,
            const std::uint32_t maxShapeMutations,
            std::uint32_t maxThreads)
    {
        // Ensure that the maximum number of threads is a sane value
        if(maxThreads == 0) {
            maxThreads = std::thread::hardware_concurrency();
            if(maxThreads == 0) {
                assert(0 && "Failed to get the number of concurrent threads supported by the implementation");
                maxThreads = defaultMaxThreads;
            }
        }

        std::vector<std::future<geometrize::State>> futures{maxThreads};
        for(std::uint32_t i = 0; i < futures.size(); i++) {
            std::future<geometrize::State> handle{std::async(std::launch::async, [&](const std::uint32_t seed, const float lastScore) {
                // Ensure that the results of the random generation are the same between tasks with identical settings
                // The RNG is thread-local and std::async may use a thread pool (which is why this is necessary)
                // Note this implementation requires maxThreads to be the same between tasks for each task to produce the same results.
                geometrize::commonutil::seedRandomGenerator(seed);

                geometrize::Bitmap buffer{m_current};
                return core::bestHillClimbState(*q, shapeTypes, alpha, shapeCount, maxShapeMutations, m_target, m_current, buffer, lastScore);
            }, m_baseRandomSeed + m_randomSeedOffset++, m_lastScore)};
            futures[i] = std::move(handle);
        }

        std::vector<geometrize::State> states;
        for(auto& f : futures) {
            states.emplace_back(f.get());
        }
        return states;
    }

    std::vector<geometrize::ShapeResult> step(
            const geometrize::ShapeTypes shapeTypes,
            const std::uint8_t alpha,
            const std::uint32_t shapeCount,
            const std::uint32_t maxShapeMutations,
            const std::uint32_t maxThreads)
    {
        std::vector<geometrize::State> states{getHillClimbState(shapeTypes, alpha, shapeCount, maxShapeMutations, maxThreads)};
        if(states.empty()) {
            assert(0 && "Failed to get a hill climb state");
            return {};
        }

        std::vector<geometrize::State>::iterator it = std::min_element(states.begin(), states.end(), [](const geometrize::State& a, const geometrize::State& b) {
            return a.m_score < b.m_score;
        });

        const std::vector<geometrize::ShapeResult> results{drawShape((*it).m_shape, alpha)};
        return results;
    }

    geometrize::ShapeResult drawShape(
            const std::shared_ptr<geometrize::Shape> shape,
            const std::uint8_t alpha)
    {
        const std::vector<geometrize::Scanline> lines{shape->rasterize()};
        const geometrize::rgba color(geometrize::core::computeColor(m_target, m_current, lines, alpha));
        const geometrize::Bitmap before{m_current};
        geometrize::drawLines(m_current, color, lines);

        m_lastScore = geometrize::core::differencePartial(m_target, before, m_current, m_lastScore, lines);

        const geometrize::ShapeResult result{m_lastScore, color, shape};
        return result;
    }

    geometrize::ShapeResult drawShape(
            const std::shared_ptr<geometrize::Shape> shape,
            const geometrize::rgba color)
    {
        const std::vector<geometrize::Scanline> lines{shape->rasterize()};
        const geometrize::Bitmap before{m_current};
        geometrize::drawLines(m_current, color, lines);

        m_lastScore = geometrize::core::differencePartial(m_target, before, m_current, m_lastScore, lines);

        const geometrize::ShapeResult result{m_lastScore, color, shape};
        return result;
    }

    geometrize::Bitmap& getTarget()
    {
        return m_target;
    }

    geometrize::Bitmap& getCurrent()
    {
        return m_current;
    }

    const geometrize::Bitmap& getTarget() const
    {
        return m_target;
    }

    const geometrize::Bitmap& getCurrent() const
    {
        return m_current;
    }

    void setSeed(const std::uint32_t seed)
    {
        m_baseRandomSeed = seed;
    }

    const geometrize::ShapeMutator& getShapeMutator() const
    {
        return m_shapeMutator;
    }

    geometrize::ShapeMutator& getShapeMutator()
    {
        return m_shapeMutator;
    }

private:
    geometrize::Model* q;
    geometrize::Bitmap m_target; ///< The target bitmap, the bitmap we aim to approximate.
    geometrize::Bitmap m_current; ///< The current bitmap.
    float m_lastScore; ///< Score derived from calculating the difference between bitmaps.
    const static std::uint32_t defaultMaxThreads{4};
    std::atomic<std::uint32_t> m_baseRandomSeed; ///< The base value used for seeding the random number generator (the one the user has control over).
    std::atomic<std::uint32_t> m_randomSeedOffset; ///< Seed used for random number generation. Note: incremented by each std::async call used for model stepping.
    geometrize::ShapeMutator m_shapeMutator; ///< Object responsible for setting up and mutating shapes created by this model.
};

Model::Model(const geometrize::Bitmap& target) : d{std::unique_ptr<Model::ModelImpl>(new Model::ModelImpl(this, target))}
{}

Model::Model(const geometrize::Bitmap& target, const geometrize::Bitmap& initial) : d{std::unique_ptr<Model::ModelImpl>(new Model::ModelImpl(this, target, initial))}
{}

Model::~Model()
{}

void Model::reset(const geometrize::rgba backgroundColor)
{
    d->reset(backgroundColor);
}

std::int32_t Model::getWidth() const
{
    return d->getWidth();
}

std::int32_t Model::getHeight() const
{
    return d->getHeight();
}

std::vector<geometrize::ShapeResult> Model::step(
        const geometrize::ShapeTypes shapeTypes,
        const std::uint8_t alpha,
        const std::uint32_t shapeCount,
        const std::uint32_t maxShapeMutations,
        const std::uint32_t maxThreads)
{
    return d->step(shapeTypes, alpha, shapeCount, maxShapeMutations, maxThreads);
}

geometrize::ShapeResult Model::drawShape(const std::shared_ptr<geometrize::Shape> shape, const std::uint8_t alpha)
{
    return d->drawShape(shape, alpha);
}

geometrize::ShapeResult Model::drawShape(std::shared_ptr<geometrize::Shape> shape, geometrize::rgba color)
{
    return d->drawShape(shape, color);
}

geometrize::Bitmap& Model::getTarget()
{
    return d->getTarget();
}

geometrize::Bitmap& Model::getCurrent()
{
    return d->getCurrent();
}

const geometrize::Bitmap& Model::getTarget() const
{
    return d->getTarget();
}

const geometrize::Bitmap& Model::getCurrent() const
{
    return d->getCurrent();
}

void Model::setSeed(const std::uint32_t seed)
{
    d->setSeed(seed);
}

const geometrize::ShapeMutator& Model::getShapeMutator() const
{
    return d->getShapeMutator();
}

geometrize::ShapeMutator& Model::getShapeMutator()
{
    return d->getShapeMutator();
}

}
