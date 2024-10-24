#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
	debug_ = false;
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(0, 0);
	return true;
}

bool Player::Start() {

	position = Vector2D(0, 0);

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();


	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	move.LoadAnimations(parameters.child("animations").child("move"));
	jumping.LoadAnimations(parameters.child("animations").child("jumping"));
	falling.LoadAnimations(parameters.child("animations").child("falling"));
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), 16, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;
	
	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");
	jumpFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/King/Land/king_jump.wav");
	landFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/King/Land/king_land.wav");
	return true;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);
	//LOG("%i, %i", position.getX(), position.getY());

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		debug_ = !debug_;
	}
	
	if (debug_)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;

		if (velocity.y < 0)
		{
			velocity.y = 0;
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -0.4 * dt;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = 0.4 * dt;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -0.4 * dt;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = 0.4 * dt;
		}
	}

	if (!debug_)
	{

		if (velocity.x == 0)
		{
			currentAnimation = &idle;
			running = false;
		}

		// Move left
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			flipSprite = true;
			velocity.x = -0.2 * dt;
			running = true;
			if (flipSprite == true && hflip == SDL_FLIP_NONE) {
				hflip = SDL_FLIP_HORIZONTAL;
			}
		}

		// Move right
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			flipSprite = false;
			velocity.x = 0.2 * dt;
			running = true;
			if (flipSprite == false && hflip == SDL_FLIP_HORIZONTAL) {
				hflip = SDL_FLIP_NONE;
			}
		}

		if (isJumping == true) {
			currentAnimation = &jumping;
		}

		if (running == true && currentAnimation != &jumping)
		{
			currentAnimation = &move;
		}

		//Jump
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false && isFalling == false) {
			// Apply an initial upward force
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isJumping = true;
			Engine::GetInstance().audio.get()->PlayFx(jumpFxId);
		}

		//if (position.getY() < 0)
		//{
		//	Engine::GetInstance().scene.get()->changeLevel(currentLevel + 1);
		//	position.setY(100);
		//}

		//if (position.getY() == 300)
		//{
		//	Engine::GetInstance().scene.get()->changeLevel(currentLevel - 1);
		//}

		// If the player is jumpling, we don't want to apply gravity, we use the current velocity prduced by the jump
		if (isJumping == true)
		{
			velocity.y = pbody->body->GetLinearVelocity().y;
		}

		if (x == 0 && currentAnimation == &idle && isFalling != true)
		{
			Engine::GetInstance().audio.get()->PlayFx(landFxId);
			x = 1;
		}

		if (x == 0 && currentAnimation == &move && isFalling != true)
		{
			Engine::GetInstance().audio.get()->PlayFx(landFxId);
			x = 1;
		}

		if (velocity.y > 1)
		{
			isFalling = true;
		}

		if (isFalling == true) {
			currentAnimation = &falling;
			x = 0;
		}
	}
	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (position.getY() < -20 and currentLevel != maxLevel)
	{
		ascend(true);
	}

	if (position.getY() > 370 and currentLevel != 1)
	{
		ascend(false);
	}

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();
	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		isFalling = false;
		
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::ascend(bool upDown)
{
	b2Transform pbodyPos = pbody->body->GetTransform();

	if (upDown)
	{
		position.setY(350);
		Engine::GetInstance().scene.get()->changeLevel(currentLevel + 1, currentLevel);
		pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY())), 0);
	}
	else
	{
		position.setY(10);
		Engine::GetInstance().scene.get()->changeLevel(currentLevel - 1, currentLevel);
		pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY())), 0);
	}
}