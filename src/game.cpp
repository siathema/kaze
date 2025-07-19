#include "game.hpp"
#include "assets.hpp"
#include "gameo.hpp"
#include "renderer.hpp"
#include "sMath.hpp"
#include "sync.hpp"
#include "ui.hpp"
#include <cmath>
#include <math.h>
#include <queue>
#include <vector>

namespace SMOBA {
#define MAX(a, b) (a > b) ? (a) : (b)
#define MIN(a, b) (a < b) ? (a) : (b)

struct sprite {
    vec3 pos;
    vec2 scale;
    r32 rot;

    RenderCommand rc;

    void tick(r32 delta) {
            
    }

    void render(std::queue<RenderCommand> * rq) {
        rc.RenderType = TEXTURERENDER;
        rc.ShaderType = SIMPLEBLIT;
        rc.Texture = ASSETS::TEXTURES::Shell;
        rc.TextureRect = iRect(0, 0, 288, 288);
        rc.Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        //todo(aria): fix this.
        rc.Pos = pos;
        rc.Scale = vec3( -scale.x, scale.y, 0.0f);
        rc.Rot = rot;
        rq->push(rc);

    }
};

void update_loop(Sync *GameSync) {
    // NOTE(matthias): initialization of Game starts here. Stays in scope for
    // entire game lives on the stack rn.
    GameState game = {};
    game.GameSync = GameSync;


    sprite s = {};
    s.pos = vec3(900, 0.0f, 1.0f);
    s.scale = vec2(2.0f, -2.0f);
    s.rot = 0.0f;

    b8 DebugGame = true;
    if (DebugGame) {
        // NOTE(matthias): Main Game loop
        while (GameSync->Running.load()) {
            std::lock_guard<std::mutex> lock(GameSync->Mutex);
            if (GameSync->UpdateLoop) {
                // Update
                // Rendering
                s.render(GameSync->Rq);

                GameSync->UpdateLoop = false;
            }
        }
    }
}

Game::Game() {}

Game::~Game() {}
} // namespace SMOBA
