#pragma once
#include "game.hpp"
#include "sync.hpp"
#include "array.hpp"

namespace SMOBA
{
	void update_loop(Sync* GameSync);

	struct Game
	{
		Game();
		~Game();
		void tick(r32 delta);
		void render(Queue_Array<RenderCommand>* rq);
	};
}