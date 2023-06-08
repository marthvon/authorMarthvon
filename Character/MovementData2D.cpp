#include "MovementData2D.h"

Vector2 MovementData2D::_get_velocity(const GroundedMovementData1D::Data& p_data) const {
	Vector2 res = GroundedMovementData1D::_get_velocity(p_data);
	res.y = (p_data.transitioning?
		(-initial_jump_velocity - (Math::abs(p_data.previous_speed.x) * xVel_to_yVel_ratio) + (p_data.previous_speed.y * scale_inherited_ySpeed) + (gravity * p_data.delta * 0.5)) :
		(p_data.previous_speed.y + (gravity * p_data.delta))
	);
	return res;
}


void MovementData2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_jump_height", "jump_height"), &MovementData2D::set_jump_height);
	ClassDB::bind_method(D_METHOD("get_jump_height"), &MovementData2D::get_jump_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_height"), "set_jump_height", "get_jump_height");

	ClassDB::bind_method(D_METHOD("set_jump_duration", "jump_duration"), &MovementData2D::set_jump_duration);
	ClassDB::bind_method(D_METHOD("get_jump_duration"), &MovementData2D::get_jump_duration);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_duration"), "set_jump_duration", "get_jump_duration");

	ClassDB::bind_method(D_METHOD("set_gravity", "gravity", "keep_jump_duration"), &MovementData2D::set_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity_fixed_height", "gravity"), &MovementData2D::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &MovementData2D::get_gravity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity_fixed_height", "get_gravity");

	ClassDB::bind_method(D_METHOD("set_xVel_to_yVel_ratio", "ratio"), &MovementData2D::set_xVel2yVel_ratio);
	ClassDB::bind_method(D_METHOD("get_xVel_to_yVel_ratio"), &MovementData2D::get_xVel2yVel_ratio);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xVel_to_yVel_ratio"), "set_xVel_to_yVel_ratio", "get_xVel_to_Vel_ratio");

	ClassDB::bind_method(D_METHOD("set_scale_inherited_ySpeed", "scale"), &MovementData2D::set_scale_inherited_ySpeed);
	ClassDB::bind_method(D_METHOD("get_scale_inherited_ySpeed"), &MovementData2D::get_scale_inherited_ySpeed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_inherited_ySpeed"), "set_scale_inherited_ySpeed", "get_scale_inherited_ySpeed");
}

void MovementData2D::_update_jump_cache() {
	real_t _jump_duration_with_height = jump_duration / Math::pow(2.0, 1.0 + (Math::log(jump_height) / Math::log(4.0)));
	initial_jump_velocity = 2.0 * Math::sqrt(jump_height) / _jump_duration_with_height;
	gravity = 2.0 / (_jump_duration_with_height * _jump_duration_with_height);
}

void MovementData2D::set_jump_height(const real_t p_height) {
	jump_height = p_height;
	_update_jump_cache();
}
real_t MovementData2D::get_jump_height() const {
	return jump_height;
}
void MovementData2D::set_jump_duration(const real_t p_duration) {
	jump_duration = p_duration;
	_update_jump_cache();
}
real_t MovementData2D::get_jump_duration() const {
	return jump_duration;
}
void MovementData2D::set_gravity(const real_t p_gravity, const bool keep_jump_duration) {
	gravity = p_gravity;
	if (keep_jump_duration)
		jump_duration = Math::sqrt(Math::pow(2, 3.0 + 2.0 * Math::log(jump_height) / Math::log(4.0)) / p_gravity);
	else
		jump_height = Math::pow(4.0, (Math::log(jump_duration * jump_duration * p_gravity) / (Math::log(2.0) * 2.0)) - 1.5);
	_update_jump_cache();
}
real_t MovementData2D::get_gravity() const {
	return gravity;
}
real_t MovementData2D::get_initial_jump_velocity() const {
	return initial_jump_velocity;
}

void MovementData2D::set_xVel2yVel_ratio(const real_t p_ratio) {
	xVel_to_yVel_ratio = p_ratio;
}
real_t MovementData2D::get_xVel2yVel_ratio() const {
	return xVel_to_yVel_ratio;
}

void MovementData2D::set_scale_inherited_ySpeed(const real_t p_scale) {
	scale_inherited_ySpeed = p_scale;
}
real_t MovementData2D::get_scale_inherited_ySpeed() const {
	return scale_inherited_ySpeed;
}
