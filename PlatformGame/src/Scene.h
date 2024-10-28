#pragma once

#include "Module.h"
#include "Player.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void changeLevel(int level, int previousLevel);

private:
	SDL_Texture* bg;
	bool CTVisible = false;
	SDL_Texture* CTtexture = nullptr;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
};