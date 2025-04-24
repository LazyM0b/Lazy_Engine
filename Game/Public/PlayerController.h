#pragma once

#include "InputDevice.h"
#include "GameComponent.h"

class PlayerController {
public:
	void UpdatePos(InputDevice* input, GameComponent* object);
};