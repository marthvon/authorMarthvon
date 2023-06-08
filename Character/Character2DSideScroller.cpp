#include "Character2DSideScroller.h"

void Character2DSideScroller::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_state", "state"), &Character2DSideScroller::set_state);
	ClassDB::bind_method(D_METHOD("get_state"), &Character2DSideScroller::get_state);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "state", PROPERTY_HINT_FLAGS), "set_state", "get_state");

	ClassDB::bind_method(D_METHOD("set_grounded_movement_data", "grounded_movement"), &Character2DSideScroller::set_grounded_movement_data);
	ClassDB::bind_method(D_METHOD("get_grounded_movement_data"), &Character2DSideScroller::get_grounded_movement_data);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "grounded_movement", PROPERTY_HINT_RESOURCE_TYPE, "GroundedMovementData1D"), "set_grounded_movement_data", "get_grounded_movement_data");

	ClassDB::bind_method(D_METHOD("set_jumping_movement_data", "jumping_movement"), &Character2DSideScroller::set_jumping_movement_data);
	ClassDB::bind_method(D_METHOD("get_jumping_movement_data"), &Character2DSideScroller::get_jumping_movement_data);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "jumping_movement", PROPERTY_HINT_RESOURCE_TYPE, "MovementData2D"), "set_jumping_movement_data", "get_jumping_movement_data");
	
	ClassDB::bind_method(D_METHOD("toggle_movement_disable", "disable_movement"), &Character2DSideScroller::toggle_movement_disable);
	ClassDB::bind_method(D_METHOD("is_movement_disable"), &Character2DSideScroller::is_movement_disable);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disable_movement"), "toggle_movement_disable", "is_movement_disable");

	ClassDB::bind_method(D_METHOD("set_max_jump_count", "count"), &Character2DSideScroller::set_max_jump_count);
	ClassDB::bind_method(D_METHOD("get_max_jump_count"), &Character2DSideScroller::get_max_jump_count);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_jump_count"), "set_max_jump_count", "get_max_jump_count");
	ClassDB::bind_method(D_METHOD("get_jump_counter"), &Character2DSideScroller::get_jump_counter);

	ClassDB::bind_method(D_METHOD("set_friction", "friction"), &Character2DSideScroller::set_friction);
	ClassDB::bind_method(D_METHOD("get_friction"), &Character2DSideScroller::get_friction);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "friction"), "set_friction", "get_friction");

	ClassDB::bind_method(D_METHOD("set_character_path", "character_path"), &Character2DSideScroller::set_character_path);
	ClassDB::bind_method(D_METHOD("get_character_path"), &Character2DSideScroller::get_character_path);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "character_path"), "set_character_path", "get_character_path");

	ClassDB::bind_method(D_METHOD("toggle_facing_right", "facing_right"), &Character2DSideScroller::toggle_facing_right);
	ClassDB::bind_method(D_METHOD("is_facing_right"), &Character2DSideScroller::is_facing_right);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "facing_right"), "toggle_facing_right", "is_facing_right");
}

void Character2DSideScroller::_character_process(const double delta) { //refactor
	const uint8_t grounded_state = (state >> 1) & 0b1111;
	uint8_t air_state = ((state >> 5) & 0b1111);
	const uint8_t custom_state = (state >> 10);
	const bool reverse_transition = state & (unsigned short)(State::REVERSE_TRANSITION_BIT_FLAG);
	if ((grounded_state || (state & 0b1)) && air_state) {
		if (reverse_transition) {
			if (state & 0b1) {
				if (_call_script_instance(String("transition_to_idle_from_") + (grounded_state ? grounded_state : air_state), delta)) {
					set_velocity(Vector2());
					set_state((unsigned short)State::STATE_IDLE);
				}
				return;
			}
			if (_call_script_instance(String("_transition_") + air_state + "to" + grounded_state, delta)) {
				set_velocity(states_grounded_movement_data.get(grounded_state)->get_velocity(get_velocity(), delta, true));
				set_state(grounded_state);
			}
			return;
		}
		if (_call_script_instance(String("_transition_") + grounded_state + "to" + air_state, delta)) {
			set_velocity(states_jumping_movement_data.get(air_state)->get_velocity(get_velocity(), delta, true));
			set_state(air_state);
		}
		return;
	}
	if (!custom_state) {
		if (air_state) {
			if (reverse_transition) {
				if (_call_script_instance(String("_transition_") + custom_state + "to" + air_state, delta)) {
					set_velocity(states_jumping_movement_data.get(air_state)->get_velocity(get_velocity(), delta, true));
					set_state(air_state);
				}
				return;
			}
			if(_call_script_instance(String("_transition_") + air_state + "to" + custom_state, delta))
				set_state(custom_state);
			return;
		}
		if (grounded_state) {
			if (reverse_transition) {
				if (_call_script_instance(String("_transition_") + custom_state + "to" + grounded_state, delta)) {
					set_velocity(states_grounded_movement_data.get(grounded_state)->get_velocity(get_velocity(), delta, true));
					set_state(grounded_state);
				}
			}
			if(_call_script_instance(String("_transition_") + grounded_state + "to" + custom_state, delta))
				set_state(custom_state);
			return;
		}
		_call_script_instance(String("_state_") + custom_state, delta);
		return;
	}
	if (grounded_state) {
		set_velocity(states_grounded_movement_data.get(grounded_state)->get_velocity(get_velocity(), delta));
		if (!is_on_floor()) {
			set_state((unsigned short)(State::STATE_FALLING));
			air_state = 1;
		}
	}
	if (air_state) {
		set_velocity(states_jumping_movement_data.get(air_state)->get_velocity(get_velocity(), delta));
		if (is_on_floor()) {
			set_state(air_state | (unsigned short)(State::STATE_IDLE) | (unsigned short)(State::REVERSE_TRANSITION_BIT_FLAG));
			_character_process(delta); // transition
		}
		return;
	}
}

void Character2DSideScroller::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			_character_process(get_physics_process_delta_time());
		} break;
		case NOTIFICATION_PAUSED:
			set_physics_process_internal(false);
		case NOTIFICATION_UNPAUSED:
			set_physics_process_internal(is_visible_in_tree() && !disable_movement);
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_VISIBILITY_CHANGED:
			set_physics_process_internal(is_visible_in_tree() && !disable_movement);
	};
}

void Character2DSideScroller::set_state(const unsigned short p_state) {
	state = p_state;
}
int Character2DSideScroller::get_state() const {
	return state;
}
void Character2DSideScroller::set_grounded_movement_data(const Array& p_list) {
	if (p_list.size() >= 16)
		return;
	states_grounded_movement_data.clear();
	states_grounded_movement_data.resize(p_list.size());
	for (int i = 0; i != p_list.size(); ++i)
		states_grounded_movement_data.push_back(Ref<GroundedMovementData1D>(Object::cast_to<GroundedMovementData1D>(p_list.get(i))));
}
Array Character2DSideScroller::get_grounded_movement_data() const {
	Array res;
	res.resize(states_grounded_movement_data.size());
	for (auto i = states_grounded_movement_data.begin(); i != states_grounded_movement_data.end(); ++i)
		res.push_back(Variant(**i));

	return res;
}
void Character2DSideScroller::set_jumping_movement_data(const Array& p_list) {
	if (p_list.size() >= 16)
		return;
	states_jumping_movement_data.clear();
	states_jumping_movement_data.resize(p_list.size());
	for (int i = 0; i != p_list.size(); ++i)
		states_jumping_movement_data.push_back(Ref<GroundedMovementData1D>(Object::cast_to<GroundedMovementData1D>(p_list.get(i))));
}
Array Character2DSideScroller::get_jumping_movement_data() const {
	Array res;
	res.resize(states_jumping_movement_data.size());
	for (auto i = states_jumping_movement_data.begin(); i != states_jumping_movement_data.end(); ++i)
		res.push_back(Variant(**i));

	return res;
}
void Character2DSideScroller::toggle_movement_disable(const bool p_disable) {
	set_physics_process_internal(!p_disable);
	disable_movement = p_disable;
}
bool Character2DSideScroller::is_movement_disable() const {
	return disable_movement;
}
void Character2DSideScroller::set_max_jump_count(const unsigned short int p_count) {
	max_jump_count = p_count;
}
int Character2DSideScroller::get_max_jump_count() const {
	return max_jump_count;
}
int Character2DSideScroller::get_jump_counter() const {
	const unsigned short s = state;
	return ((s >= (unsigned short)(State::STATE_FALLING) && s < ((unsigned short)(State::REVERSE_TRANSITION_BIT_FLAG) << 1) )?
		((s >> 5) & 0b1111) : 0);
}
void Character2DSideScroller::set_friction(const real_t p_friction){
	friction = p_friction;
}
real_t Character2DSideScroller::get_friction() const{
	return friction;
}
void Character2DSideScroller::toggle_facing_right(const bool p_is_right) {
	isFacingRight = p_is_right;
}
bool Character2DSideScroller::is_facing_right() const {
	return isFacingRight;
}
void Character2DSideScroller::set_character_path(const NodePath p_path) {
	character_path = p_path;
}
NodePath Character2DSideScroller::get_character_path() const {
	return character_path;
}


bool Character2DSideScroller::_call_script_instance(const String& p_method, const double delta) {
	if (get_script_instance()->has_method(p_method))
		return get_script_instance()->call(p_method, delta);
	return true;
}
