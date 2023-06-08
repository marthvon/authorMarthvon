#include "GroundedMovementData1D.h"

Vector2 GroundedMovementData1D::_get_velocity(const GroundedMovementData1D::Data& p_data) const {
	return Vector2((p_data.transitioning?
		_assign_velocity(p_data.previous_speed.x, p_data.delta) : _update_velocity(p_data.previous_speed.x, p_data.delta)),
	0);
}
GroundedMovementData1D::Data::Data(const Vector2 p_previous_speed, const double p_delta, const bool p_transitioning)
	: previous_speed(p_previous_speed), delta(p_delta), transitioning(p_transitioning) {}


real_t GroundedMovementData1D::_update_velocity(const real_t previous_speed, const double delta) const {
	if (!acceleration)
		return previous_speed;
	return MAX(MIN(previous_speed + (acceleration * delta), max_speed), min_speed);
}
real_t GroundedMovementData1D::_assign_velocity(const real_t previous_speed, const double delta) const {
	return MAX(MIN(speed + (previous_speed * scale_inherited_speed) + (acceleration * delta * 0.5), max_speed), min_speed);
}

Vector2 GroundedMovementData1D::get_velocity(const Vector2 previous_speed, const double delta, const bool transitioning) const {
	return _get_velocity(Data(previous_speed, delta, transitioning));
}

void GroundedMovementData1D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_velocity", "previous_speed", "delta_time", "transitioning"), &GroundedMovementData1D::get_velocity);

	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &GroundedMovementData1D::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &GroundedMovementData1D::get_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

	ClassDB::bind_method(D_METHOD("set_acceleration", "acceleration"), &GroundedMovementData1D::set_acceleration);
	ClassDB::bind_method(D_METHOD("get_acceleration"), &GroundedMovementData1D::get_acceleration);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "acceleration"), "set_acceleration", "get_acceleration");

	ClassDB::bind_method(D_METHOD("set_max_speed", "speed"), &GroundedMovementData1D::set_max_speed);
	ClassDB::bind_method(D_METHOD("get_max_speed"), &GroundedMovementData1D::get_max_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_speed"), "set_max_speed", "get_max_speed");

	ClassDB::bind_method(D_METHOD("set_min_speed", "speed"), &GroundedMovementData1D::set_min_speed);
	ClassDB::bind_method(D_METHOD("get_min_speed"), &GroundedMovementData1D::get_min_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_speed"), "set_min_speed", "get_min_speed");

	ClassDB::bind_method(D_METHOD("set_scale_inherited_speed", "scale_inherited_speed"), &GroundedMovementData1D::set_scale_inherited_speed);
	ClassDB::bind_method(D_METHOD("get_scale_inherited_speed"), &GroundedMovementData1D::get_scale_inherited_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_inherited_speed"), "set_scale_inherited_speed", "get_scale_inherited_speed");
}

void GroundedMovementData1D::set_speed(const real_t p_speed) {
	speed = p_speed;
}
real_t GroundedMovementData1D::get_speed() const {
	return speed;
}
void GroundedMovementData1D::set_acceleration(const real_t p_acceleration) {
	acceleration = p_acceleration;
}
real_t GroundedMovementData1D::get_acceleration() const {
	return acceleration;
}
void GroundedMovementData1D::set_max_speed(const real_t p_speed) {
	max_speed = MAX(p_speed, speed);
}
real_t GroundedMovementData1D::get_max_speed() const {
	return max_speed;
}
void GroundedMovementData1D::set_min_speed(const real_t p_speed) {
	min_speed = MIN(p_speed, speed);
}
real_t GroundedMovementData1D::get_min_speed() const {
	return min_speed;
}
void GroundedMovementData1D::set_scale_inherited_speed(const real_t p_scale){
	scale_inherited_speed = p_scale;
}
real_t GroundedMovementData1D::get_scale_inherited_speed() const {
	return scale_inherited_speed;
}
