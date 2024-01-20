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

//NOTE(Aria): Were doing AABB collisions and brute forcing detections for now,
// if needed we can change to SAT and quadtrees later.
struct AABB_Box {
    vec2 pos;
    vec2 dim;
    ID id;
};

#define MAX_COLLIDERS 64
struct Colliders {
    i32 size;
    AABB_Box col_array[MAX_COLLIDERS];

    void init() {
        size = MAX_COLLIDERS;
        for(u32 i = 0; i < MAX_COLLIDERS; i++) {
            col_array[i].id = i;
        }
    }
};

// Incase we want to just see if a point is in a box.
bool point_in_box(const vec2& point, const AABB_Box& box) {
    if(((point.x >= box.pos.x) && (point.x <= box.pos.x + box.dim.x)) &&
            ((point.y >= box.pos.y) && (point.y <= box.pos.y + box.dim.y))) {
        return true;
    }
    return false;
}

bool AABB_Collision(const AABB_Box& box1, const AABB_Box& box2) {
    if(box1.pos.x < box2.pos.x + box2.dim.x &&
        box1.pos.x + box1.dim.x > box2.pos.x &&
        box1.pos.y < box2.pos.y + box2.dim.y &&
        box1.pos.y + box1.dim.y > box2.pos.y) {
        return true;
    }
    return false;
}

const AABB_Box* find_collision_AABB(const AABB_Box& box, Colliders& colliders) {
    AABB_Box* result = 0;
    for(u32 i=0; i < colliders.size; i++) {
        if(box.id != colliders.col_array[i].id) {
           if(AABB_Collision(box, colliders.col_array[i])) {
                result = &colliders.col_array[i];
                break;
           }
        }
    }
    return result;
}

#define PIXELS_PER_METER 100
struct entity {
    ID id;
    vec2 pos;
    vec2 vel;
    r32 speed, jump_speed;
    quat rot;
    RenderCommand render_info;
    bool jump, jc;
    AABB_Box* collider;

    void init(AABB_Box* c) {
        speed = 7 * PIXELS_PER_METER;
        jump_speed = 50000;
        vel = vec2::zero;
        jump = jc = false;
        collider = c;
        collider->dim.x = 90;
        collider->dim.y = 160;
        collider->pos.x = pos.x;
        collider->pos.y = pos.y;
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

    void tick(float delta, Input* ip, Colliders* colliders) {
        vec2 dir = vec2::zero;
        collider->pos = pos;
        if(find_collision_AABB(*collider, *colliders)) {
            printf("Collision!\n");
        } else {
            printf("no collision :/\n");
        }
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
    // entire game lives on the stack rn.
    GameState game = {};
    game.GameSync = GameSync;

    b8 DebugGame = true;
    Colliders colliders = {};
    colliders.init();

    entity sasha = {};
    entity test_rect = {};
    sasha.pos.x = 0.f;
    sasha.pos.y = 0.f;
    sasha.init(&colliders.col_array[0]);
    test_rect.pos.x = 100.f;
    test_rect.pos.y = 0.f;
    test_rect.init(&colliders.col_array[1]);


    if (DebugGame) {
        // NOTE(matthias): Main Game loop
        while (GameSync->Running.load()) {
            std::lock_guard<std::mutex> lock(GameSync->Mutex);
            if (GameSync->UpdateLoop) {
                // Update
                sasha.tick(GameSync->delta, GameSync->Ip, &colliders);
                // Rendering
                sasha.render(GameSync->Rq);
                test_rect.render(GameSync->Rq);

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
