#include "game.hpp"
#include "gameo.hpp"
#include "sync.hpp"
#include "assets.hpp"
#include "renderer.hpp"
#include "ui.hpp"
#include <queue>
#include <vector>


namespace SMOBA
{
	struct entity
	{
		ID id;
		vec3 pos;
		vec3 dir;
		r32 speed;
		quat rot;
		RenderCommand render_info;
		entity* debug_waypoint;

		void init() {
			render_info.RenderType = MESHRENDER;
			render_info.ShaderType = SIMPLEMESH;
			render_info.Mesh = 0;
			render_info.Texture = ASSETS::TEXTURES::Sasha;
			render_info.Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			render_info.Pos = pos;
			render_info.Scale = vec3(1.0f, 1.0f, 1.0f);
			rot = quat(toRadians(-90), vec3(1.0f, 0.0f, 0.0f));
			render_info.Quat = rot;
		}

		void tick(float delta) {
            //printf("pos: %f, %f\n", pos.x, pos.z);
			if (debug_waypoint != nullptr) {
				dir = debug_waypoint->pos - pos;
                if(dir.length() < 0.05f) {
                    pos = debug_waypoint->pos;
                    return;
                } else {
                    dir.normalize();
                }
			}
			pos += dir * speed * delta;
		}

		void render(std::queue<RenderCommand>* rq) {
			render_info.Pos = pos;
			render_info.Scale = vec3(1.0f, 1.0f, 1.5f);

			rq->push(render_info);
		}
	};

	struct waypoint : entity {

	};

	struct field {
		RenderCommand render_info;
		vec2 size;
		vec3 pos;
		plane p;

		void init(r32 w, r32 l, vec3 pos) {}
	};

	vec2 mouse_ray_intersect_plane(Input& input, Camera& cam, plane p) {
		return vec2::zero;
	}

    void update_loop(Sync* GameSync)
    {
		// NOTE(matthias): inialization of Game starts here. Stays in scope for entire game but lives on the stack.
		GameState game = {};
		game.GameSync = GameSync;

		b8 DebugGame = true;

		field field = {};
		entity sasha = {};
		sasha.init();
		sasha.pos.x = 20.f;
		sasha.pos.z = 20.f;
		//sasha.pos.z = 10.f;
		waypoint wayPoint = {};
		wayPoint.init();
		wayPoint.pos.x = 50.f;
		wayPoint.pos.z = 50.f;
		wayPoint.render_info.Color =  vec4(1.0f, 0.2f, 0.2f, 1.0f);

		entity entities[100] = {};
		for (u32 i = 0; i < 100; i++)
		{
			entities[i].init();
			r32 xPos = static_cast <r32> (rand()) / (static_cast <r32> (RAND_MAX / 100.f));
			r32 zPos = static_cast <r32> (rand()) / (static_cast <r32> (RAND_MAX / 100.f));
            //printf("xPos: %f, zPos %f\n", xPos, zPos);
			entities[i].pos.x = xPos;
			entities[i].pos.z = zPos;
			entities[i].debug_waypoint = &wayPoint;
			entities[i].speed = 5.0f;
            printf("pos: %f, %f\n", entities[i].pos.x, entities[i].pos.z);

		}

		field.render_info.RenderType = MESHRENDER;
		field.render_info.ShaderType = SIMPLEMESH;
		field.render_info.Mesh = 0;
		field.render_info.Texture = 4;
		field.render_info.Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		field.render_info.Pos = vec3(0.0f, 0.0f, 0.0f);
		field.render_info.Scale = vec3(100.0f, 100.0f, 100.0f);
		field.render_info.Quat = quat::zero;


		if (DebugGame)
		{
			// NOTE(matthias): Main Game loop
			while (GameSync->Running.load())
			{
				std::lock_guard<std::mutex> lock(GameSync->Mutex);
				if (GameSync->UpdateLoop)
				{
					// Update
					if (GameSync->Ip->LeftMouseDown) {
						mouse_ray_intersect_plane(*GameSync->Ip, GameSync->Cams[1], field.p);
					}
					GameSync->Cams[1].Update(GameSync->Ip);
					for (u32 i = 0; i < 100; i++)
					{
                        printf("entity: %d pos: %f, %f\n", i, entities[i].pos.x, entities[i].pos.z);
						entities[i].tick(GameSync->delta);
                        printf("entity: %d pos: %f, %f\n", i, entities[i].pos.x, entities[i].pos.z);
                        printf("delta: %f\n", GameSync->delta);
					}

					// Rendering
					GameSync->Rq->push(field.render_info);
					sasha.render(GameSync->Rq);
					wayPoint.render(GameSync->Rq);
					for (u32 i = 0; i < 100; i++)
					{
						entities[i].render(GameSync->Rq);
					}


					GameSync->UpdateLoop = false;
				}
			}
		}
    }

    Game::Game()
    {
        
    }

    Game::~Game()
    {
    }

    void Game::tick(r32 delta)
    {
    }

    void Game::render(std::queue<RenderCommand>* rq)
    {
        
    }
}
