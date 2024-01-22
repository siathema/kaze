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

struct Component {
    ID entity;
    u64 type;
};

struct Position {
    Component comp;
    vec2 pos;
};

struct Player {
    Component comp;
    vec2 vel;
    r32 speed, jump_speed;
    bool jump, jc;
};

struct Sprite {
    Component comp;
    RenderCommand rc;
};

struct Collider {
    Component comp;
    AABB_Box* collider;
};

struct Entity {
    ID id;
    const char * name;
    u64 components;
    bool visible;
    bool alive;
};

enum component_value {
    POS_COM = 1 << 0,
    PLAY_COM = 1 << 1,
    SPRITE_COM = 1 << 2,
    COLLIDE_COM = 1 << 3
};
//NOTE(Aria): Make this more generalized.
#define MAX_COMPONENTS 64
#define MAX_ENTITIES 128
struct Components {
   Position positions[MAX_ENTITIES]; 
   Position players[MAX_ENTITIES]; 
   Sprite sprites[MAX_ENTITIES]; 
   Collider colliders[MAX_ENTITIES]; 
   Entity entities[MAX_ENTITIES];

   u16 num_entities;
};

Component* Get_Entity_Component(ID entity, component_value comp, Components* comps) {
    Entity* e = &comps->entities[entity];
    if(e->components &= comp) {
        switch (comp) {
            case POS_COM:{
                return (Component*)&comps->positions[entity];
                         }break;
            case PLAY_COM:{
                return (Component*)&comps->players[entity];
                          }break;
            case SPRITE_COM:{
                return (Component*)&comps->sprites[entity];
                           }break;
            case COLLIDE_COM:{
                return (Component*)&comps->colliders[entity];
                             }break;
            default:{

                    }break;
        } 
    }
    return 0;
}

void components_create(Components* comps) {
    for(u32 i = 0; i < MAX_ENTITIES; i++) {
        comps->positions[i].comp.entity = i;
        comps->positions[i].comp.type = POS_COM;
        comps->players[i].comp.entity = i;
        comps->players[i].comp.type = PLAY_COM;
        comps->sprites[i].comp.entity = i;
        comps->sprites[i].comp.type = SPRITE_COM;
        comps->colliders[i].comp.entity = i;
        comps->colliders[i].comp.type = COLLIDE_COM;
    }
}

void position_init(ID id, Components* comps) {}
void player_init(ID id, Components* comps) {}
void sprite_init(ID id, Components* comps) {}
void collider_init(ID id, Components* comps) {}

void components_init(Components* comps) {
    u16 num_entities = comps->num_entities;
    for(u32 i = 0; i < num_entities; i++) {
        position_init(i, comps);
        player_init(i, comps);
        sprite_init(i, comps);
        collider_init(i, comps);
    }
}

void position_tick(ID id, Components* comp) {}
void player_tick(ID id, Components* comp, r32 delta, Input* ip, Colliders* colliders) {
    Player* p = (Player*)Get_Entity_Component(id, PLAY_COM, comp);
    Collider* c = (Collider*)Get_Entity_Component(id, COLLIDE_COM, comp);
    Position* pos = (Position*)Get_Entity_Component(id, POS_COM, comp);
    vec2 dir = vec2::zero;
    c->collider->pos = pos->pos;
    /*
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
    */
}

void sprite_tick(ID id, Components* comp) {}
void collider_tick(ID id, Components* comp) {}

void components_tick(Components* comps, r32 delta, Input* ip, Colliders* colliders) {
    u16 num_entities = comps->num_entities;
    for(u32 i = 0; i < num_entities; i++) {
        position_tick(i, comps);
        player_tick(i, comps, delta, ip, colliders);
        sprite_tick(i, comps);
        collider_tick(i, comps);
    }
}

void position_draw(ID id, Components* comp, std::queue<RenderCommand> * rq) {}
void player_draw(ID id, Components* comp, std::queue<RenderCommand> *rq) {}
void sprite_draw(ID id, Components* comp, std::queue<RenderCommand> *rq) {}
void collider_draw(ID id, Components* comp, std::queue<RenderCommand> *rq) {}

void components_draw(Components* comps, std::queue<RenderCommand> *rq) {
    u16 num_entities = comps->num_entities;
    for(u32 i = 0; i < num_entities; i++) {
        position_draw(i, comps, rq);
        player_draw(i, comps, rq);
        sprite_draw(i, comps, rq);
        collider_draw(i, comps, rq);
    }
}


#define PIXELS_PER_METER 100
struct old_entity {
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

    Components comps = {};
    components_create(&comps);
    components_init(&comps);
    comps.num_entities = 2;

    old_entity sasha = {};
    old_entity test_rect = {};
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
                components_tick(&comps, GameSync->delta, GameSync->Ip, &colliders);
                // Rendering
                sasha.render(GameSync->Rq);
                test_rect.render(GameSync->Rq);
                components_draw(&comps, GameSync->Rq);

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
