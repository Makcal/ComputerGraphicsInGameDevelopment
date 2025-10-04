#pragma once

#include "settings.h"
#include "world/camera.h"
#include "world/model.h"

namespace cg::renderer {

class renderer { // NOLINT(*special-member*)
  public:
    explicit renderer(std::shared_ptr<cg::settings> settings);
    virtual ~renderer() = default;

    unsigned get_height();
    unsigned get_width();

    virtual void init() = 0;
    virtual void destroy() = 0;

    virtual void update() = 0;
    virtual void render() = 0;

    static constexpr float kMovePrecision = 0.01F;
    void move_forward(float delta = kMovePrecision);
    void move_backward(float delta = kMovePrecision);
    void move_left(float delta = kMovePrecision);
    void move_right(float delta = kMovePrecision);
    void move_yaw(float delta = 0.F);
    void move_pitch(float delta = 0.F);

    void load_model();
    void load_camera();

  protected:
    // NOLINTBEGIN(*-non-private-*)
    std::shared_ptr<cg::settings> settings;

    std::shared_ptr<cg::world::camera> camera;
    std::shared_ptr<cg::world::model> model;

    std::chrono::time_point<std::chrono::high_resolution_clock> current_time =
        std::chrono::high_resolution_clock::now();
    float frame_duration = 0.F;
    // NOLINTEND(*-non-private-*)
};

std::shared_ptr<renderer> make_renderer(std::shared_ptr<cg::settings> settings);

} // namespace cg::renderer
