#include "racer_game.hpp"

namespace Racer {

using namespace Tyra;

RacerGame::RacerGame(Engine* t_engine) { engine = t_engine; }
RacerGame::~RacerGame() {}

void RacerGame::init() { TYRA_LOG("Open-world racer init"); }

void RacerGame::loop() {}

}  // namespace Racer
