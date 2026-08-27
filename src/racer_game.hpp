#ifndef RACER_GAME_HPP
#define RACER_GAME_HPP

#include <tyra>

namespace Racer {

class RacerGame : public Tyra::Game {
public:
    RacerGame(Tyra::Engine* t_engine);
    ~RacerGame();
    void init();
    void loop();

private:
    Tyra::Engine* engine;
    std::unique_ptr<Tyra::DynamicMesh> groundMesh;
    Tyra::Vec4 cameraPosition = Tyra::Vec4(0.0f, 5.0f, -10.0f);
    Tyra::Vec4 cameraLookAt = Tyra::Vec4(0.0f, 0.0f, 0.0f);
    Tyra::DynamicPipeline dynpip;
};

} // namespace Racer

#endif
