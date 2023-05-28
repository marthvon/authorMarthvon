#include "TouchButton.h"

#include "core/input/input_event.h"
#include "scene/main/window.h"

void TouchButton::_notification(int p_what) {
    switch(p_what){
		case NOTIFICATION_PAUSED:
			_reset();
        break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
			if (get_finger_index() != -1)
				accum_t += get_process_delta_time();
		break;
        case NOTIFICATION_DRAW: {
            Color color = Color(1,1,1);
            if(get_finger_index() != -1) {
                if(pressed.is_valid()) {
                    draw_texture_rect_region(pressed, Rect2(Point2(), get_size()), Rect2(Point2(), pressed->get_size()));
                    return;
                }
                color = Color(0.75,0.75,0.75);
            } 
            if(normal.is_valid())
                draw_texture_rect_region(normal, Rect2(Point2(), get_size()), Rect2(Point2(), normal->get_size()), color);
        } break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (Engine::get_singleton()->is_editor_hint())
				return;

			if (!is_visible_in_tree()) {
				set_process_input(false);
				_reset();
			} else
				set_process_input(true);
		} break;
    }
}

void TouchButton::input(const Ref<InputEvent>& p_event) {
    ERR_FAIL_COND(p_event.is_null());

	if (!is_visible_in_tree()) {
		return;
	}
    const InputEventScreenTouch* const st = Object::cast_to<InputEventScreenTouch>(*p_event);

    if(st) {
		Point2 coord = get_global_transform_with_canvas().affine_inverse().xform(st->get_position());
        const bool point_is_inside = Control::has_point(coord) && (!radius || (coord - (get_size()/2.0)).length() <= radius );
        if(point_is_inside && get_finger_index() == -1 && st->is_pressed()) {
            _press(st->get_index());
        } else if (get_finger_index() == st->get_index()) {
			if (signal_only_when_released_inside && !point_is_inside) {
				_reset();
				return;
			}
            _release();
        }
    }
}

void TouchButton::_press(int p_index) {
    _set_finger_index(p_index);
    if (action != StringName()) {
	    Input::get_singleton()->action_press(action);
		Ref<InputEventAction> iea;
		iea.instantiate();
		iea->set_action(action);
		iea->set_pressed(true);
		get_viewport()->push_input(iea, true);
	}

	emit_signal("button_pressed");
	queue_redraw();
}

void TouchButton::_release() {
    _set_finger_index(-1);
    if (action != StringName()) {
	    Input::get_singleton()->action_release(action);
	    Ref<InputEventAction> iea;
	    iea.instantiate();
	    iea->set_action(action);
	    iea->set_pressed(false);
		get_viewport()->push_input(iea, true);
	}

    emit_signal("button_released");
	if (isAccumulate) {
		emit_signal("button_released_with_time_accum", accum_t);
		accum_t = 0;
	}
	queue_redraw();
}

void TouchButton::_reset() {
	_set_finger_index(-1);
	accum_t = 0;
	queue_redraw();
}

void TouchButton::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TouchButton::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &TouchButton::get_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture"), "set_texture", "get_texture");

    ClassDB::bind_method(D_METHOD("set_pressed_texture", "texture"), &TouchButton::set_pressed_texture);
	ClassDB::bind_method(D_METHOD("get_pressed_texture"), &TouchButton::get_pressed_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "pressed"), "set_pressed_texture", "get_pressed_texture");

    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &TouchButton::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &TouchButton::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ClassDB::bind_method(D_METHOD("set_action", "action"), &TouchButton::set_action);
	ClassDB::bind_method(D_METHOD("get_action"), &TouchButton::get_action);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "action"), "set_action", "get_action");

    ClassDB::bind_method(D_METHOD("toggle_accumulate_time", "accumulate"), &TouchButton::toggle_accumulate_time);
	ClassDB::bind_method(D_METHOD("is_accumulate_time"), &TouchButton::is_accumulate_time);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "accumulating"), "toggle_accumulate_time", "is_accumulate_time");

    ClassDB::bind_method(D_METHOD("toggle_signal_release_inside", "inside"), &TouchButton::toggle_signal_release_inside);
	ClassDB::bind_method(D_METHOD("is_signal_release_inside"), &TouchButton::is_signal_release_inside);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "signal_release_when_inside"), "set_action", "is_signal_release_inside");

	ClassDB::bind_method(D_METHOD("get_held_time"), &TouchButton::get_held_time);
	ClassDB::bind_method(D_METHOD("is_held"), &TouchButton::is_held);

    ADD_SIGNAL(MethodInfo("button_pressed"));
    ADD_SIGNAL(MethodInfo("button_released"));
    ADD_SIGNAL(MethodInfo("button_released_with_time_accum", PropertyInfo(Variant::FLOAT, "time")));
}

void TouchButton::set_action(StringName p_name) {
    action = p_name;
}
StringName TouchButton::get_action() const {
    return action;
}
void TouchButton::set_texture(Ref<Texture2D> p_texture){
    normal = p_texture;
}
Ref<Texture2D> TouchButton::get_texture() const{
    return normal;
}
void TouchButton::set_pressed_texture(Ref<Texture2D> p_texture){
    pressed = p_texture;
}
Ref<Texture2D> TouchButton::get_pressed_texture() const {
    return pressed;
}
void TouchButton::set_radius(const real_t p_radius){
    radius = p_radius;
}
real_t TouchButton::get_radius() const{
    return radius;
}
void TouchButton::toggle_accumulate_time(const bool p_accumulate){
    isAccumulate = p_accumulate;
	accum_t = 0;
	set_physics_process_internal(p_accumulate);
}
bool TouchButton::is_accumulate_time() const{
    return isAccumulate;
}
void TouchButton::toggle_signal_release_inside(const bool p_bool) {
    signal_only_when_released_inside = p_bool;
}
bool TouchButton::is_signal_release_inside() const {
    return signal_only_when_released_inside;
}
real_t TouchButton::get_held_time() const {
    return accum_t;
}
bool TouchButton::is_held() const{
    return isHeld;
}
