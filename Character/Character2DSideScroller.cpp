#include "Character2DSideScroller.h"

void Character2DSideScroller::_bind_methods() {
	/*
	ClassDB::bind_method(D_METHOD("", ""), &Character2DSideScroller::);
	ClassDB::bind_method(D_METHOD(""), &Character2DSideScroller::);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, ""), "", "");

	ADD_GROUP("", "");
	*/
}

void Character2DSideScroller::_notification(int p_notification) {
	switch (p_notification) {
		
	};
}

void Character2DSideScroller::_update_jump_cache() {
	const real_t temp_height = jump_height;
	real_t _jump_duration_with_height = jump_duration / Math::pow(2.0, 1.0 + (Math::log(temp_height) / Math::log(4.0)));
	initial_jump_velocity = 2.0 * Math::sqrt(temp_height) / _jump_duration_with_height;
	gravity = 2.0 / (_jump_duration_with_height * _jump_duration_with_height);
}

void Character2DSideScroller::set_jump_height(const real_t p_height){
	jump_height = p_height;
	_update_jump_cache();
}
real_t Character2DSideScroller::get_jump_height() const{
	return jump_height;
}
void Character2DSideScroller::set_jump_duration(const real_t p_duration){
	jump_duration = p_duration;
	_update_jump_cache();
}
real_t Character2DSideScroller::get_jump_duration() const{
	return jump_duration;
}
void Character2DSideScroller::set_gravity(const real_t p_gravity) {
	gravity = p_gravity;
	jump_duration = Math::sqrt(gravity / Math::pow(2, 2.0 * Math::log(jump_height) / log(4) + 3.0));
	_update_jump_cache();
}
real_t Character2DSideScroller::get_gravity() const {
	return gravity;
}
real_t Character2DSideScroller::get_initial_jump_velocity() const{
	return initial_jump_velocity;
}
//
void Character2DSideScroller::set_air_directional_speed(const real_t p_speed){
	air_directional_speed = p_speed;
}
real_t Character2DSideScroller::get_air_directional_speed() const{
	return air_directional_speed;
}
void Character2DSideScroller::set_air_drag(const real_t p_air_drag){
	air_drag = p_air_drag;
}
real_t Character2DSideScroller::get_air_drag() const {
	return air_drag;
}
void Character2DSideScroller::set_max_jump_count(const unsigned short int p_count) {
	max_jump_count = p_count;
}
int Character2DSideScroller::get_max_jump_count() const {
	return max_jump_count;
}
//
void Character2DSideScroller::set_walking_speed(const real_t p_speed){
	walking_speed = p_speed;
}
real_t Character2DSideScroller::get_walking_speed() const{
	return walking_speed;
}
void Character2DSideScroller::set_crawling_speed(const real_t p_speed){
	crawling_speed = p_speed;
}
real_t Character2DSideScroller::get_crawling_speed() const{
	return crawling_speed;
}
void Character2DSideScroller::set_running_speed(const real_t p_speed){
	running_speed = p_speed;
}
real_t Character2DSideScroller::get_running_speed() const{
	return running_speed;
}
//
void Character2DSideScroller::set_acceleration(const real_t p_acceleration){
	acceleration = p_acceleration;
}
real_t Character2DSideScroller::get_acceleration() const{
	return acceleration;
}
void Character2DSideScroller::set_max_speed(const real_t p_max_speed){
	max_speed = p_max_speed;
}
real_t Character2DSideScroller::get_max_speed() const{
	return max_speed;
}
void Character2DSideScroller::set_friction(const real_t p_friction){
	friction = p_friction;
}
real_t Character2DSideScroller::get_friction() const{
	return friction;
}
//
void Character2DSideScroller::toggle_movement_disable(const bool p_disable){
	disable_movement = p_disable;
}
bool Character2DSideScroller::is_movement_disable() const{
	return disable_movement;
}
