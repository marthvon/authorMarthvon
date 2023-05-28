#include "TouchScreenPad.h"

#include "core/os/os.h"

inline constexpr char* _direction_changed = "direction_changed"; 

void TouchScreenPad::_set_center_point(const Point2 p_center_point) {
	_center_point = p_center_point;
}

const bool TouchScreenPad::_set_cardinal_direction_span(real_t p_span) {
	cardinal_direction_span = p_span;
	return true;
}

const Point2 TouchScreenPad::_get_center_point() const {
	return _center_point;
}

void TouchScreenPad::_set_direction(Direction p_direction) {
	direction = p_direction;
}

const bool TouchScreenPad::_set_deadzone_extent(real_t p_extent) {
	deadzone_extent = p_extent;
	return true;
}

void TouchScreenPad::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!Engine::get_singleton()->is_editor_hint() && !(is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
				return;
			}
			queue_redraw();

			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(is_visible_in_tree());
			}
		} break;
		case NOTIFICATION_EXIT_TREE:
			_release();
		break;
		case NOTIFICATION_PAUSED:
			set_process_input(false);
			if(get_finger_index() != -1)
				_propagate_on_unpause = true;
		break;
		case NOTIFICATION_UNPAUSED: {
			set_process_input(true);
			if (_propagate_on_unpause) {
				_release();
				_propagate_on_unpause = false;
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (Engine::get_singleton()->is_editor_hint())
				return;

			if (!is_visible_in_tree()) 
				set_process_input(false);
			else {
				set_process_input(true);
				if (get_finger_index() != -1)
					_release();
			}
		} break;
	}

}

void TouchScreenPad::_bind_methods() {

	ClassDB::bind_method(D_METHOD("is_centered"), &TouchScreenPad::is_centered);
	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &TouchScreenPad::set_centered);

	ClassDB::bind_method(D_METHOD("get_center_offset"), &TouchScreenPad::get_center_offset);
	ClassDB::bind_method(D_METHOD("set_center_offset", "offset"), &TouchScreenPad::set_center_offset);

	ClassDB::bind_method(D_METHOD("get_deadzone_extent"), &TouchScreenPad::get_deadzone_extent);
	ClassDB::bind_method(D_METHOD("set_deadzone_extent", "extent"), &TouchScreenPad::set_deadzone_extent);

	ClassDB::bind_method(D_METHOD("get_cardinal_direction_span"), &TouchScreenPad::get_cardinal_direction_span);
	ClassDB::bind_method(D_METHOD("set_cardinal_direction_span", "span"), &TouchScreenPad::set_cardinal_direction_span);

	ClassDB::bind_method(D_METHOD("get_direction"), &TouchScreenPad::get_direction);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "center offset"), "set_center_offset", "get_center_offset");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "deadzone extent"), "set_deadzone_extent", "get_deadzone_extent");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "direction span"), "set_cardinal_direction_span", "get_cardinal_direction_span");

	ADD_SIGNAL(MethodInfo("direction_changed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::INT, "direction", PROPERTY_HINT_FLAGS)
	));

	BIND_ENUM_CONSTANT(DIR_NEUTRAL);
	BIND_ENUM_CONSTANT(DIR_LEFT);
	BIND_ENUM_CONSTANT(DIR_RIGHT);
	BIND_ENUM_CONSTANT(DIR_DOWN);
	BIND_ENUM_CONSTANT(DIR_DOWN_LEFT);
	BIND_ENUM_CONSTANT(DIR_DOWN_RIGHT);
	BIND_ENUM_CONSTANT(DIR_UP);
	BIND_ENUM_CONSTANT(DIR_UP_LEFT);
	BIND_ENUM_CONSTANT(DIR_UP_RIGHT);
}

void TouchScreenPad::_direction_changed() {
	emit_signal("direction_changed", Variant(get_finger_index()), Variant(direction));
}

void TouchScreenPad::_release() {
	_set_finger_index(-1);
	direction = DIR_NEUTRAL;
	emit_signal("direction_changed", Variant(get_finger_index()), Variant(direction));
}

void TouchScreenPad::set_centered(bool p_centered) {
	centered = p_centered;
	_update_cache_dirty();
	queue_redraw();
}

bool TouchScreenPad::is_centered() const {
	return centered;
}

void TouchScreenPad::set_deadzone_extent(real_t p_extent) {
	if (!_set_deadzone_extent(p_extent))
		return;
	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
		_update_cache_dirty();
		queue_redraw();
	}
}

real_t TouchScreenPad::get_deadzone_extent() const {
	return deadzone_extent;
}

TouchScreenPad::Direction TouchScreenPad::get_direction() const {
	return direction;
}

void TouchScreenPad::set_center_offset(Point2 p_offset) {
	offset_center = p_offset;
	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
		_update_cache_dirty();
		queue_redraw();
	}
}

Point2 TouchScreenPad::get_center_offset() const {
	return offset_center;
}

void TouchScreenPad::set_cardinal_direction_span(real_t p_span) {
	if(!_set_cardinal_direction_span(p_span))
		return;
	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
		_update_cache_dirty();
		queue_redraw();
	}
}

real_t TouchScreenPad::get_cardinal_direction_span() const {
	return cardinal_direction_span;
}

void TouchScreenPad::_update_cache_dirty() {
	update_cache = true;
}

bool TouchScreenPad::is_update_cache() {
	if(!update_cache)
		return false;
	update_cache = false;
	return true;
}

TouchScreenPad::TouchScreenPad(real_t p_extent, real_t p_span)
	: deadzone_extent(p_extent), cardinal_direction_span(p_span) {}

