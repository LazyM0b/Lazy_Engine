#include "PlayerController.h"

void PlayerController::UpdatePos(InputDevice* input, GameComponent* object)
{
	//add velocity to player
	Vector3 tmpVelocity;
	if (input->IsKeyDown(Keys::W)) 
		tmpVelocity += object->impulse * -1;
	
	if (input->IsKeyDown(Keys::S)) 
		tmpVelocity += object->impulse;
	
	if (input->IsKeyDown(Keys::A)) 
		tmpVelocity += Vector3(cos(asin(1.0f) + asin(object->impulse.z)), 0.0f, sin(asin(1.0f) + acos(object->impulse.x)));
	
	if (input->IsKeyDown(Keys::D)) 
		tmpVelocity += Vector3(cos(asin(-1.0f) + asin(object->impulse.z)), 0.0f, sin(asin(-1.0f) + acos(object->impulse.x)));
	
	if (input->IsKeyDown(Keys::Space) && object->translation.y == object->sphereCollider.Radius)
		tmpVelocity += Vector3(0.0f, 20.0f * object->sphereCollider.Radius / object->scale.y, 0.0f);

	if (input->IsKeyDown(Keys::LeftShift) && object->dashCD < 0.0f) {
		object->speed = 5.0f;
		object->dashCD = 3.0f;
	}
	
	if (abs(Vector3::Distance(object->velocity + tmpVelocity * object->speed, Vector3::Zero)) < object->speedMax)
		object->velocity += tmpVelocity * object->speed;
	else
		std::cout << "max speed reached" << std::endl;

	object->translation += object->velocity;

	//rotate player relatively to velocity
	if (object->velocity != Vector3::Zero) {
		object->rotationAngle -= Vector3::Distance(object->velocity, Vector3::Zero) / object->speedMax;
		object->rotation = Quaternion::CreateFromAxisAngle(Vector3(
			-object->velocity.z / object->speedMax,
			0.0f,
			object->velocity.x / object->speedMax), object->rotationAngle);
	}

	//decrease velocity for inertia effect
	if (object->translation.y > object->sphereCollider.Radius && object->velocity.y < 10.0f) {
		object->velocity.y -= 1.0f;
	}
	else if (object->translation.y < object->sphereCollider.Radius){
		object->velocity.y = 0.0f;
		object->translation.y = object->sphereCollider.Radius;
	}
	object->velocity -= object->velocity / 20.0f * object->sphereCollider.Radius / object->scale.y;

	object->speed -= 0.1f;

	if (object->speed < object->initialSpeed)
		object->speed = object->initialSpeed;
	
	if (object->dashCD > 0.0f)
		object->dashCD -= 0.016f;
}
