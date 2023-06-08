#ifndef CHARACTER_2D_SIDE_SCROLLER
#define CHARACTER_2D_SIDE_SCROLLER

#include "scene/2d/physics_body_2d.h"
#include "GroundedMovementData1D.h"
#include "MovementData2D.h"

class Character2DSideScroller : public CharacterBody2D {
	GDCLASS(Character2DSideScroller, CharacterBody2D);

public:
	enum class State {
		STATE_IDLE = 1,
		//Grounded Movement States
		STATE_RUNNING = (1 << 1),
		STATE_WALKING = (2 << 1),
		STATE_CRAWLING = (3 << 1),
		//Jumping Movement States
		STATE_FALLING = (1 << 5),
		//Combine enums bits with OR operator --- ex. STATE_X | STATE_Y, then label as TRANSITION_FROM_X_TO_Y
		//	--- ex. STATE_X | STATE_Y | REVERSE_TRANSITION_BIT_FLAG, then label as TRANSITION_FROM_Y_TO_X
		REVERSE_TRANSITION_BIT_FLAG = (1 << 9)
		//Custom States starting from bit flag (1 << 10) == 1024
	};

private:
	NodePath character_path = NodePath();
	bool isFacingRight = true;

	unsigned short state = (unsigned short)State::STATE_IDLE;
	bool disable_movement = false;

	Vector<Ref<GroundedMovementData1D>> states_grounded_movement_data;
	Vector<Ref<MovementData2D>> states_jumping_movement_data;

	uint8_t max_jump_count = 1;
	real_t friction = 0;

protected:
	void _notification(int p_notification);
	static void _bind_methods();
public:
	void set_grounded_movement_data(const Array& p_list);
	Array get_grounded_movement_data() const;
	void set_jumping_movement_data(const Array& p_list);
	Array get_jumping_movement_data() const;
	//
	void set_max_jump_count(const unsigned short int p_count);
	int get_max_jump_count() const;
	int get_jump_counter() const;
	void set_friction(const real_t p_friction);
	real_t get_friction() const;
	//
	void toggle_movement_disable(const bool p_disable);
	bool is_movement_disable() const;
	void set_state(const unsigned short p_state);
	int get_state() const;

	void toggle_facing_right(const bool p_is_right);
	bool is_facing_right() const;

	void set_character_path(const NodePath p_path);
	NodePath get_character_path() const;
private:
	void _character_process(const double delta);

	inline void transition(const uint8_t from_state, const uint8_t to_state, const bool reverse_transition, const double delta);
	inline void _transitioning_states(const uint8_t from_state, const uint8_t to_state, const bool reverse_transition, const double delta);
	inline void _transition_custom_states(const uint8_t state, const uint8_t custom_state, const bool reverse_transition, const double delta);
	inline bool _call_script_instance(const String& p_method, const double delta);
};

#endif
