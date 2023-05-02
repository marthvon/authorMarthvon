#ifndef CHARACTER_2D
#define CHARACTER_2D

#include "scene/2d/physics_body_2d.h"

class Character2D : public CharacterBody2D {
	GDCLASS(Character2D, CharacterBody2D);

public:
	enum class State {
		STATE_RUNNING,
		STATE_WALKING,
		STATE_CRAWLING,
		STATE_JUMPING,
		STATE_FALLING
	};

private:

};

#endif
