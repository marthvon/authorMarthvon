#ifndef CHARACTER_2D
#define CHARACTER_2D

#include "scene/2d/physics_body_2d.h"

class Character2DSideScroller : public CharacterBody2D {
	GDCLASS(Character2DSideScroller, CharacterBody2D);

public:
	enum class State {
		STATE_IDLE = 0,
		STATE_RUNNING,
		STATE_WALKING,
		STATE_CRAWLING,
		STATE_JUMPING,
		STATE_FALLING,
		MAX_BUILT_IN_STATES
	};

private:
	Vector2 _snap_vector = Vector2(0, -1);
	Vector2 velocity = Vector2(0, 0);
	State state = State::STATE_IDLE;
	bool disable_movement = false;

	real_t gravity = 0;
	real_t max_speed = 0;
	real_t accereleration = 0;
	real_t air_drag = 0;
	real_t air_directional_input = 0;

protected:

	void _notification(int p_notification);
	static void _bind_methods();
public:

};

#endif
