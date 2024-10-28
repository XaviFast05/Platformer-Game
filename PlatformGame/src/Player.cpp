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

bool Player::Start() 
{
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
	splatted.LoadAnimations(parameters.child("animations").child("splatted"));
	currentAnimation = &idle;

	// L08 TODO 5: Add physics to the player - initialize physics body

	pbodyBody = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), 10, 10, bodyType::DYNAMIC);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), 10, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbodyBody->listener = this;
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbodyBody->ctype = ColliderType::PLAYERSENSOR;
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	jumpFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/King/Land/king_jump.wav");
	landFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/King/Land/king_land.wav");
	splatFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/King/Land/king_splat.wav");
	return true;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		debug_ = !debug_;
		levelsFallen = 0;
	}
	
	if (debug_)
	{
		velocity.y = pbody->body->GetLinearVelocity().y;

		if (velocity.y < 0)
		{
			velocity.y = 0;
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -0.4 * 16;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = 0.4 * 16;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -0.4 * 16;
		}
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = 0.4 * 16;
		}
	}

	if (!debug_)
	{

		if (velocity.x == 0 && currentAnimation != &splatted)
		{
			currentAnimation = &idle;
			running = false;
		}

		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F4) == KEY_DOWN and (isJumping != true or isFalling != true))
		{
			if (isJumping == false)
			{
				currentAnimation = &splatted;
				Engine::GetInstance().audio.get()->PlayFx(splatFxId);
			}
		}

		// Move left
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			flipSprite = true;
			isSplatted = false;
			velocity.x = -0.2 * 16;
			running = true;
			if (flipSprite == true && hflip == SDL_FLIP_NONE) {
				hflip = SDL_FLIP_HORIZONTAL;
			}
		}

		// Move right
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			flipSprite = false;
			isSplatted = false;
			velocity.x = 0.2 * 16;
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
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN and not isJumping and not isFalling) 
		{
			// Apply an initial upward force
 			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isSplatted = false;
			Engine::GetInstance().audio.get()->PlayFx(jumpFxId);
			isJumping = true;
		}

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

		if (isSplatted == true)
		{
			currentAnimation = &splatted;
		}
	}
	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);

	float y = METERS_TO_PIXELS(pbodyPos.p.y) - texH - 0.5 / 2;
	position.setY(y + 12);

	if (position.getY() < -20 and currentLevel != maxLevel)
	{
		ascend(true);
	}

	if (position.getY() > 370 and currentLevel != 1)
	{
		ascend(false);
	}

	Engine::GetInstance().render.get()->DrawTexture(texture, (float)position.getX(), (float)position.getY(), &currentAnimation->GetCurrentFrame(), hflip);
	currentAnimation->Update();

	b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(position.getX()) + 0.3, PIXEL_TO_METERS(position.getY()) + 0.52);
	pbodyBody->body->SetTransform(pos, 0);

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
	if (physA->ctype == ColliderType::PLAYERSENSOR and physB->ctype == ColliderType::PLATFORM and isJumping and isFalling)
	{
		//reset the jump flag when touching the ground
		if (levelsFallen >= 2)
		{
			LOG("ESTAMPADO");
			isSplatted = true;
			Engine::GetInstance().audio.get()->PlayFx(splatFxId);
		}
		levelsFallen = 0;
		isJumping = false;
	    isFalling = false;
	}

	if (physA->ctype == ColliderType::PLAYERSENSOR and physB->ctype == ColliderType::PLATFORM and not isJumping and isFalling)
	{
		isFalling = false;
		if (levelsFallen >= 2)
		{
			LOG("ESTAMPADO");
			isSplatted = true;
			Engine::GetInstance().audio.get()->PlayFx(splatFxId);
		}
		levelsFallen = 0;
	}

	else if (physA->ctype != ColliderType::PLAYERSENSOR)
	{
		switch (physB->ctype)
		{
		case ColliderType::PLATFORM:
			LOG("Collision PLATFORM");
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
	float currentX = PIXEL_TO_METERS(position.getX()) + 0.33333;

	if (upDown)
	{
		position.setY(350);
		Engine::GetInstance().scene.get()->changeLevel(currentLevel + 1, currentLevel);
		pbody->body->SetTransform(b2Vec2(currentX, PIXEL_TO_METERS(position.getY())), 0);
	}
	else
	{
		levelsFallen++;
		position.setY(10);
		Engine::GetInstance().scene.get()->changeLevel(currentLevel - 1, currentLevel);
		pbody->body->SetTransform(b2Vec2(currentX, PIXEL_TO_METERS(position.getY())), 0);
	}
}