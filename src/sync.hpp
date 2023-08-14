#pragma once

#include "game.hpp"
#include "renderer.hpp"
#include "ui.hpp"
#include "voxel.hpp"
#include <atomic>
#include <mutex>

namespace SMOBA {
struct Sync {
    std::mutex Mutex;
    std::atomic_bool Running;
    bool UpdateLoop;
    std::queue<RenderCommand> *Rq;
    Input *Ip;
    ViewportInfo *Viewport;
    UI_Context *UiContext;
    Camera *Cams;
    r32 delta;

    inline Sync() : Mutex(), Running(), UpdateLoop(true) {
        Rq = 0;
        Ip = 0;
    }
};

struct GameState {
    Sync *GameSync;
    UI_ID ButtonTest;
    UI_ID PanelTest;
};
} // namespace SMOBA
