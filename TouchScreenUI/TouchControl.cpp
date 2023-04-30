#include "TouchControl.h"

void TouchControl::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_minimum_size"), &TouchControl::update_minimum_size);

	ClassDB::bind_method(D_METHOD("get_finger_index"), &TouchControl::get_finger_index);
	ClassDB::bind_method(D_METHOD("is_passby_press"), &TouchControl::is_passby_press);
	ClassDB::bind_method(D_METHOD("set_passby_press", "passby_press"), &TouchControl::set_passby_press);
	
	ADD_GROUP("Touch", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "passby press"), "set_passby_press", "is_passby_press");
}

void TouchControl::_set_finger_index(const int p_finger_pressed) {
	finger_pressed = p_finger_pressed;
}

int TouchControl::get_finger_index() const {
	return finger_pressed;
}

const bool TouchControl::is_passby_press() const {
	return passby_press;
}

void TouchControl::set_passby_press(const bool p_passby_press) {
	passby_press = p_passby_press;
	queue_redraw();
}
