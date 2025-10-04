#include "raytracer.h"

#include "renderer/renderer.h"
#include "resource.h"

#include <memory>

namespace cg::renderer {
class ray_tracing_renderer : public renderer {
  public:
    explicit ray_tracing_renderer(std::shared_ptr<cg::settings> settings);

    void init() override;
    void destroy() override;

    void update() override;
    void render() override;

  protected:
    // NOLINTBEGIN(*non-private*)
    std::shared_ptr<cg::resource<cg::unsigned_color>> render_target;

    std::shared_ptr<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>> raytracer;
    std::shared_ptr<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>> shadow_raytracer;

    std::vector<cg::renderer::light> lights{}; // NOLINT(*redundant-member-init) linter bug in constructor's definition
    // NOLINTEND(*non-private*)
};
} // namespace cg::renderer
