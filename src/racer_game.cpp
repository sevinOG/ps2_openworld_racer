#include "racer_game.hpp"

namespace Racer {

using namespace Tyra;

RacerGame::RacerGame(Engine* t_engine) : Game(t_engine) { engine = t_engine; }
RacerGame::~RacerGame() {}

void RacerGame::init() {
    TYRA_LOG("Loading ground plane...");
    
    // Load OBJ file for ground plane
    ObjLoaderOptions options;
    options.scale = 1.0f;
    
    auto data = ObjLoader::load(FileUtils::fromCwd("res/ground.obj"), options);
    groundMesh = std::make_unique<DynamicMesh>(data.get());
}

void RacerGame::loop() {
    // Render the ground plane
    engine->renderer.beginFrame(CameraInfo3D(&cameraPosition, &cameraLookAt));
    dynpip.setRenderer(&engine->renderer.core);
    engine->renderer.renderer3D.usePipeline(dynpip);
    dynpip.render(groundMesh.get());
    engine->renderer.endFrame();
}

} // namespace Racer
