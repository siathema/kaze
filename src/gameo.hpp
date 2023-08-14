#pragma once
#include "array.hpp"
#include "game.hpp"
#include "sync.hpp"

namespace SMOBA {
void update_loop(Sync *GameSync);

struct Game {
    Game();
    ~Game();
    void tick(r32 delta);
    void render(std::queue<RenderCommand> *rq);
};
} // namespace SMOBA
