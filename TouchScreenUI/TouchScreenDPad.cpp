#include "TouchScreenDPad.h"

#include "core/os/os.h"
#include "core/config/project_settings.h"
#include "core/math/color.h"
#include "core/templates/vector.h"
#include "core/input/input_event.h"

void TouchScreenDPad::input(const Ref<InputEvent>& p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (!is_visible_in_tree()) {
		return;
	}

	const InputEventScreenTouch *st = Object::cast_to<InputEventScreenTouch>(*p_event);
	if (st) {
		Point2 coord = get_global_transform_with_canvas().xform_inv(st->get_position());
		if (Control::has_point(coord) && get_finger_index() == -1 && st->is_pressed()) { //press inside Control.rect
			_set_finger_index(st->get_index());
			_update_direction_with_point(coord);
		} else if (get_finger_index() == st->get_index()) //on release
			_release();
		return;
	}

	const InputEventScreenDrag *sd = Object::cast_to<InputEventScreenDrag>(*p_event);
	if (sd) {
		Point2 coord = get_global_transform_with_canvas().xform_inv(sd->get_position());
		if (is_passby_press() && get_finger_index() == -1 && Control::has_point(coord)) { //passby press enter Control.rect
			_set_finger_index(sd->get_index());
			_update_direction_with_point(coord);
		} else if (get_finger_index() == sd->get_index()) //dragging dpad direction
			_update_direction_with_point(coord);
	}
}

void TouchScreenDPad::_update_direction_with_point(Point2 p_point) {
	int result = DIR_NEUTRAL;
	p_point -= ((get_size() / 2.0) + get_center_offset());
	Direction xAxis = p_point.x > 0 ? DIR_RIGHT : DIR_LEFT;
	Direction yAxis = p_point.y > 0 ? DIR_DOWN : DIR_UP;
	const Point2 point_abs = p_point.abs();

	const real_t s = MIN(get_size().x, get_size().y) / 2.0;
	const real_t w = get_cardinal_direction_span() * s;
	const real_t l = get_deadzone_extent() * s;
		
	if (point_abs.x >= (l - MIN(MAX(point_abs.y - w, 0.0), w)))
		result |= xAxis;
	if (point_abs.y >= (l - MIN(MAX(point_abs.x - w, 0.0), w)))
		result |= yAxis;
	Direction temp = (Direction)(result);
	if (temp != get_direction()) {
		_set_direction(temp);
		_direction_changed();
	}
}

const bool TouchScreenDPad::_set_deadzone_extent(real_t p_extent) {
	p_extent = MAX(p_extent, get_cardinal_direction_span());
	p_extent = MIN(0.9f, p_extent);
	if(p_extent == get_deadzone_extent())
		return false;
	return TouchScreenPad::_set_deadzone_extent(p_extent);
}

const bool TouchScreenDPad::_set_cardinal_direction_span(real_t p_span) {
	p_span = MAX(p_span, 0.1f);
	p_span = MIN(0.9f, p_span);
	if (p_span == get_cardinal_direction_span())
		return false;
	if(p_span > get_deadzone_extent())
		set_deadzone_extent(p_span);
	return TouchScreenPad::_set_cardinal_direction_span(p_span);
}

Size2 TouchScreenDPad::get_minimum_size() const {
	if (scale_to_rect.length() == 0.0 && texture.is_valid())
		return texture->get_size();
	return Control::get_minimum_size().abs();
}

void TouchScreenDPad::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				return;
			}
			if(is_update_cache())
				_update_cache();
			draw_texture_rect(texture, _position_rect);

#ifdef TOOLS_ENABLED
			if ((Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) && _shape_points.is_valid())
				_draw_shape();
#endif
		} break;
		case NOTIFICATION_RESIZED:
			_update_cache();
		break;
	}
}

void TouchScreenDPad::_update_cache() {
	if(texture.is_null())
		return;
	Size2 size = scale_to_rect.length() == 0.0? texture->get_size() : (get_size() * scale_to_rect);
	_position_rect = Rect2((is_centered() ? (get_size() - size) / 2.0f : Point2(0, 0)), size);
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) 
		_update_shape_points();
#endif
}

Ref<Texture2D> TouchScreenDPad::get_texture() const {
	return texture;
}

void TouchScreenDPad::set_texture(const Ref<Texture2D> p_texture) {
	texture = p_texture;
	_update_cache_dirty();
	update_minimum_size();
	queue_redraw();
}

Point2 TouchScreenDPad::get_scale_to_rect() const {
	return scale_to_rect;
}

void TouchScreenDPad::set_scale_to_rect(Point2 p_scale) {
	p_scale.x = MAX(p_scale.x, 0);
	p_scale.y = MAX(p_scale.y, 0);
	scale_to_rect = p_scale;
	_update_cache_dirty();
	update_minimum_size();
	queue_redraw();
}

void TouchScreenDPad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_texture"), &TouchScreenDPad::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TouchScreenDPad::set_texture);

	ClassDB::bind_method(D_METHOD("get_texture_scale"), &TouchScreenDPad::get_scale_to_rect);
	ClassDB::bind_method(D_METHOD("set_texture_scale", "scale"), &TouchScreenDPad::set_scale_to_rect);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "texture scale"), "set_texture_scale", "get_texture_scale");
}

TouchScreenDPad::TouchScreenDPad()
	: TouchScreenPad(0.65, 0.35)
{}

#ifdef TOOLS_ENABLED
void TouchScreenDPad::_update_shape_points() {
	if (_shape_points.is_null())
		_shape_points.instantiate();

	const real_t s = (real_t)(MIN(get_size().x, get_size().y)) / 2.0;
	const real_t dw = get_cardinal_direction_span() * s;
	const real_t nzl = get_deadzone_extent() * s;

	const Point2 cp = (get_size() / 2.0) + get_center_offset();

	Vector<Vector2> points;
	if (get_cardinal_direction_span() == get_deadzone_extent()) {
		points.resize(4);
		for (int i = 0; i < 4; ++i)
			points.set(i, cp + Point2(dw * (i % 3 ? 1 : -1), nzl * (i < 2 ? 1 : -1)));
	}
	else {
		points.resize(8);
		for (int i = 0; i < 8; ++i) {
			const real_t l = nzl * (i < 4 ? 1 : -1);
			if ((i / 2) % 2)
				points.set(i, cp + Point2(l, dw * (i % 3 ? 1 : -1)));
			else
				points.set(i, cp + Point2(dw * (i % 5 ? 1 : -1), l));
		}
	}
	_shape_points->set_points(points);
}

void TouchScreenDPad::_draw_shape() {
	Color pallete = get_tree()->get_debug_collisions_color();
	draw_rect(Rect2(Point2(0, 0), get_size()), pallete.lightened(0.15));

	const Vector<Vector2> points = _shape_points->get_points();
	if (get_cardinal_direction_span() == get_deadzone_extent()) {
		for (int i = 1; i < 4; i += 2)
			for (int j = 0; j <= 2; j += 2) {
				if (((i / 3) + (j / 2)) % 2) //second [1][2]size.x(right) //third [3][0]pos.x(left)
					draw_rect(Rect2(points[i], Size2((i == 3 ? 0 : get_size().x), points[j].y) - points[i]), pallete); //rect2(DR, size2(size.x, UR.y)) //rect2(UL,size2(pos.x,  DL.y))
				else //first [1][0]size.y(down) //fourth [3][2]pos.y(up)
					draw_rect(Rect2(points[i], Size2(points[j].x, (i == 3 ? 0 : get_size().y)) - points[i]), pallete); //rect2(DR, size2(DL.x , size.y)) //rect2(UL,size2(UR.x, pos.y))
			}
	}
	else {
		for (int i = 1; i < 8; i += 2) { //1 3 5 7
			if ((i / 2) % 2) //{3(right),7(left)}
				draw_rect(Rect2(points[i], Size2((i >= 4 ? 0 : get_size().x), points[i - 1].y) - points[i]), pallete); //rect(point[3], size(size.x, point[2].y)) //rect(point[7], size(pos.x, point[6].y))
			else  //{1(down),5(up)}
				draw_rect(Rect2(points[i], Size2(points[i - 1].x, (i >= 4 ? 0 : get_size().y)) - points[i]), pallete); //rect(point[1], size(point[0].x, size.y)) //rect(point[5], size(point[4].x, pos.y))
		}
	}
	_shape_points->draw(get_canvas_item(), pallete.darkened(0.15));
}
#endif
