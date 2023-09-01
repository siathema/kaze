#include "game.hpp"
#include "assets.hpp"
#include "gameo.hpp"
#include "renderer.hpp"
#include "sMath.hpp"
#include "sync.hpp"
#include "ui.hpp"
#include <math.h>
#include <queue>
#include <vector>

namespace SMOBA {
/*
    bool point_in_polygon_convex(const std::vector<vec2> polygon, const vec2 point) {
        for(u32 i = 0; i < polygon.size(); i++) {
           size_t p2i = i == polygon.size() -1 ? 0 : i;
           vec2 v1 = polygon[p2i] - polygon[i];
           vec2 v2 = point - polygon[i];
           r64 orient = v1.cross(v2);
           if(orient > 0) {
               return false;
           }
        }
        return true;
    }
*/

    struct AABBCollider {
        rect bb;
        ID id;

        AABBCollider() : bb(rect::zero), id(0) {}

        AABBCollider(const AABBCollider& r) {
            bb = r.bb;
            id = r.id;
        }

        AABBCollider(vec2 pos, ID id) {

            this->bb = bb;
            this->id = id;
        }

        void go_to(const vec2& new_pos) {
            bb.pos = new_pos;

        }

        bool collides(const AABBCollider& o) {
            for(u32 i = 0; i < points.size(); i++) {
                if(true) {
                    return true;
                }
            }
            return false;
        }
    };

    struct collision_manager {
        std::vector<AABBCollider> AABBColliders;
        
        AABBCollider* check_collisions(const ID col_Id) {
            AABBCollider col = AABBColliders[col_Id];
            for(i32 i = 0; i < AABBColliders.size(); i++) {
               if(AABBColliders[i].id != col.id && AABBColliders[i].collides(col)) {
                    return &AABBColliders[i];
               }
            }
            return nullptr;
        }
    };

#define PIXELS_PER_METER 100
struct entity {
    ID id;
    vec2 pos;
    vec2 vel;
    r32 speed, jump_speed;
    quat rot;
    RenderCommand render_info;
    bool jump, jc;
    AABBCollider* col;

    void init() {
        speed = 7 * PIXELS_PER_METER;
        jump_speed = 50000;
        vel = vec2::zero;
        jump = jc = false;
        render_info.RenderType = TEXTURERENDER;
        render_info.ShaderType = SIMPLEBLIT;
        render_info.Mesh = 0;
        render_info.Texture = ASSETS::TEXTURES::Sasha;
        render_info.TextureRect = iRect(0, 0, 1800, 3200);
        render_info.Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        render_info.Pos = vec3(pos.x, pos.y, 1.0f);;
        render_info.Scale = vec3(1.0f, 1.0f, 1.0f);
        render_info.Rot = 0.0f;
    }

    void tick(float delta, Input* ip) {
        vec2 dir = vec2::zero;
        printf("%d\n", ip->Up);
        if( ip->Up ) {
            if(!jc) {
                jump = true;
                jc = true;
            }
        }
        if( ip->Down) {
            dir.y -= 1.0f;
        }
        if( ip->Left) {
            dir.x -= 1.0f;
        }
        if( ip->Right) {
            dir.x += 1.0f;
        }
        if(dir.length() > 1.0f) {
            dir.normalize();
        }
        vec2 acc = dir * speed;
        //gravity
        if(jump) {
            acc.y = jump_speed;
            jump = false;
        }
        acc.y -= 9.8 * PIXELS_PER_METER;

        //friction
        if(acc.x == 0.0f && !jc) {
            acc.x += -vel.x * 9.5f;
        }



        vec2 npos = (acc * 0.5 * pow(delta, 2.0f)) + vel * delta + pos;
        if(npos.y < -200) {
            npos.y = -201;
            jc = false;
        }
        pos = npos;

        vel = acc * delta + vel;

    }

    void render(std::queue<RenderCommand> *rq) {
        render_info.Pos = vec3(pos.x, pos.y, 1.0f);
        render_info.Scale = vec3(0.05f, -0.05f, 0.05f);

        rq->push(render_info);
    }
};

void update_loop(Sync *GameSync) {
    // NOTE(matthias): inialization of Game starts here. Stays in scope for
    // entire game but lives on the stack.
    GameState game = {};
    game.GameSync = GameSync;

    b8 DebugGame = true;

    entity sasha = {};
    sasha.init();
    sasha.pos.x = 0.f;
    sasha.pos.y = 0.f;

    if (DebugGame) {
        // NOTE(matthias): Main Game loop
        while (GameSync->Running.load()) {
            std::lock_guard<std::mutex> lock(GameSync->Mutex);
            if (GameSync->UpdateLoop) {
                // Update
                sasha.tick(GameSync->delta, GameSync->Ip);
                // Rendering
                sasha.render(GameSync->Rq);

                GameSync->UpdateLoop = false;
            }
        }
    }
}

Game::Game() {}

Game::~Game() {}

void Game::tick(r32 delta) {}

void Game::render(std::queue<RenderCommand> *rq) {}
} // namespace SMOBA
