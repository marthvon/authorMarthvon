#ifndef CHARACTER_2D
#define CHARACTER_2D

#include "scene/2d/physics_body_2d.h"

class Character2DSideScroller : public CharacterBody2D {
	GDCLASS(Character2DSideScroller, CharacterBody2D);

public:
	enum class State {
		STATE_IDLE = -1,
		STATE_RUNNING,
		STATE_WALKING,
		STATE_CRAWLING,
		STATE_JUMPING,
		STATE_FALLING,
		MAX_BUILT_IN_STATES
	};

private:
	NodePath character_object = NodePath();

	Vector2 _snap_vector = Vector2(0, -1);
	Vector2 velocity = Vector2(0, 0);
	State state = State::STATE_IDLE;
	bool disable_movement = false;

	real_t initial_jump_velocity = 0;
	real_t jump_duration = 0;
	real_t jump_height = 0;
	unsigned short max_jump_count = 1;

	real_t gravity = 0;
	real_t air_drag = 0;
	real_t air_directional_speed = 0;

	real_t running_speed = 0;
	real_t walking_speed = 0;
	real_t crawling_speed = 0;

	//try implement this
	//real_t speed[3] = {0,0,0};
	//real_t max_speed[3] = {0,0,0};
	//real_t acceleration[3] = {0,0,0};

	real_t max_speed = 0;
	real_t acceleration = 0;
	real_t friction = 0;
protected:

	void _notification(int p_notification);
	static void _bind_methods();
public:
	//
	void set_jump_height(const real_t p_height);
	real_t get_jump_height() const;
	void set_max_jump_count(const unsigned short int p_count);
	int get_max_jump_count() const;
	void set_jump_duration(const real_t p_duration);
	real_t get_jump_duration() const;
	real_t get_initial_jump_velocity() const;
	//
	void set_gravity(const real_t p_gravity);
	real_t get_gravity() const;
	void set_air_directional_speed(const real_t p_speed);
	real_t get_air_directional_speed() const;
	void set_air_drag(const real_t p_air_drag);
	real_t get_air_drag() const;
	//
	void set_friction(const real_t p_friction);
	real_t get_friction() const;
	void set_walking_speed(const real_t p_speed);
	real_t get_walking_speed() const;
	void set_crawling_speed(const real_t p_speed);
	real_t get_crawling_speed() const;
	void set_running_speed(const real_t p_speed);
	real_t get_running_speed() const;
	void set_acceleration(const real_t p_acceleration);
	real_t get_acceleration() const;
	void set_max_speed(const real_t p_max_speed);
	real_t get_max_speed() const;
	//
	void toggle_movement_disable(const bool p_disable);
	bool is_movement_disable() const;
	//
	void create_character_object();
private:
	void _update_jump_cache();
};

#endif
