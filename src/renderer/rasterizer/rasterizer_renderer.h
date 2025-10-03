#include "renderer/rasterizer/rasterizer.h"
#include "renderer/renderer.h"
#include "resource.h"

#include <memory>

namespace cg::renderer
{
	class rasterization_renderer : public renderer
	{
	public:
		explicit rasterization_renderer(std::shared_ptr<cg::settings> settings);

		void init() override;
		void destroy() override;

		void update() override;
		void render() override;

	protected:
		// NOLINTBEGIN(*-non-private-*)
		std::shared_ptr<cg::resource<cg::unsigned_color>> render_target;
		std::shared_ptr<cg::resource<float>> depth_buffer;
		std::shared_ptr<cg::renderer::rasterizer<cg::vertex, cg::unsigned_color>> rasterizer;
		// NOLINTEND(*-non-private-*)
	};
}// namespace cg::renderer
