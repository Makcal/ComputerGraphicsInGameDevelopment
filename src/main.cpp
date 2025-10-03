#include "renderer/renderer.h"
#include "settings.h"

#include <cstdlib>
#include <iostream>
#include <utility>

int main(int argc, char** argv)
{
	try {
		std::shared_ptr<cg::settings> settings = cg::settings::parse_settings(argc, argv);
		std::shared_ptr<cg::renderer::renderer> renderer = cg::renderer::make_renderer(std::move(settings));

		renderer->init();
		renderer->render();
		renderer->destroy();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
