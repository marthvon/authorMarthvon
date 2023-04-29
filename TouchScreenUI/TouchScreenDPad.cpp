#include "TouchScreenDPad.h"

#include "core/os/os.h"
#include "core/project_settings.h"
#include "core/color.h"
#include "core/vector.h"
#include "core/os/input_event.h"

void TouchScreenDPad::_update_shape_points(const float& size) {
	if(_shape_points.is_null())
		_shape_points.instance();

	const float s = size / 2.0;
	const float dw = get_single_direction_span() * s;
	const float nzl = get_neutral_extent() * s;

	Point2 cp = _get_center_point() + get_center_offset();

	Vector<Vector2> points;
	if(get_single_direction_span() != get_neutral_extent()) {
		points.resize(8);
		for(int i = 0; i < 8; ++i) {
			const float l = nzl * (i < 4? 1 : -1);
			if((i / 2) % 2)
				points.set(i, cp + Point2(l, dw * (i % 3? 1 : -1)));
			else
				points.set(i, cp + Point2(dw * (i % 5? 1 : -1), l));
		}
	} else {
		points.resize(4);
		for(int i = 0; i < 4; ++i)
			points.set(i, cp + Point2(dw * (i % 3? 1 : -1), nzl * (i < 2? 1 : -1)));
	}
	/*
	const bool isOctagon = get_single_direction_span() != get_neutral_extent();
	if (isOctagon)
		points.resize(8);
	else
		points.resize(4);
	
	points.set(0, Point2(-dw, nzl) + cp);
	points.set(1, Point2(dw, nzl) + cp);

	if (isOctagon) {
		points.set(2, Point2(nzl, dw) + cp);
		points.set(3, Point2(nzl, -dw) + cp);
	}

	points.set((isOctagon? 4: 2), Point2(dw, -nzl) + cp);
	points.set((isOctagon? 5: 3), Point2(-dw, -nzl) + cp);

	if (isOctagon) {
		points.set(6, Point2(-nzl, -dw) + cp);
		points.set(7, Point2(-nzl, dw) + cp);
	}
	*/
	_shape_points->set_points(points);
}

void TouchScreenDPad::_draw_shape() {
	Color pallete(0.7, 0.7, 0.7, 0.35);
	if (ProjectSetting::get_singleton()->has_setting("debug/shapes/collision/shape_color"))
		pallete = ProjectSetting::get_singleton()->get_setting("debug/shapes/collision/shape_color");
	const float min_size = MIN(_position_rect.size.x, _position_rect.size.y);
	const Rect2 _rect(_position_rect.position + get_center_offset(), Size2(min_size, min_size));
	draw_rect(_rect, pallete.lightened(0.15));

	const Vector<Vector2> points = _shape_points->get_points();
	if (points.size() == 4) {
		for (int i = 1; i < 4; i += 2)
			for (int j = 0; i <= 2; j += 2) {
				const Vector2& position = i / 3 ? _rect.position : _rect.size; //if 3 then pos else size
				if (((i / 3) + (j / 2)) % 2) //second [1][2]size.x(right) //third [3][0]pos.x(left)
					draw_rect(Rect2(points[i], Size2(position[0], points[j][1]) - points[i]), pallete); //rect2(DR, size2(size.x, UR.y)) //rect2(UL,size2(pos.x,  DL.y))
				else //first [1][0]size.y(down) //fourth [3][2]pos.y(up)
					draw_rect(Rect2(points[i], Size2(points[j][0], position[1]) - points[i]), pallete); //rect2(DR, size2(DL.x , size.y)) //rect2(UL,size2(UR.x, pos.y))
			}
	} else {
		ERR_FAIL_COND(points.size() != 8);
		for (int i = 1; i < 8; i += 2) { //1 3 5 7
			const Vector2& position = (i / 4)? _rect.position : _rect.size; // {1,3: size} {5,7: pos}
			if ((i/2) % 2) //{3(right),7(left)}
				draw_rect(Rect2(points[i], Size2(position[0], points[i - 1][1]) - points[i]), pallete); //rect(point[3], size(size.x, point[2].y)) //rect(point[7], size(pos.x, point[6].y))
			else  //{1(down),5(up)}
				draw_rect(Rect2(points[i], Size2(points[i - 1][0], position[1]) - points[i]), pallete); //rect(point[1], size(point[0].x, size.y)) //rect(point[5], size(point[4].x, pos.y))
		}
	}
	_shape_points->draw(get_canvas_item(), pallete.darkened(0.15));
}

void TouchScreenDPad::_input(const Ref<InputEvent>& p_event) {
	ERR_FAIL_COND(p_event.is_null());
	ERR_FAIL_COND(!is_visible_in_tree());

	if (!is_inside_tree() || p_event->get_device() != 0)
		return;

	const InputEventScreenTouch *st = Object::cast_to<InputEventScreenTouch>(*p_event);
	if (st) {
		Point2 coord = get_global_transform_with_canvas().xform_inv(st->get_position());
		if (iControl::has_point(coord) && get_finger_index() == -1 && st->is_pressed()) { //press inside Control.rect
			_set_finger_index(st->get_index());
			_update_direction_with_point(coord);
			if(get_direction() == DIR_NEUTRAL)
				_direction_changed();
		} else if (get_finger_index() == st->get_index()) //on release
			_release();
		return;
	}

	const InputEventScreenDrag *sd = Object::cast_to<InputEventScreenDrag>(*p_event);
	if (sd) {
		Point2 coord = get_global_transform_with_canvas().affine_inverse().xform(sd->get_position());
		if (is_passby_press() && get_finger_index() == -1 && iControl::has_point(coord)) { //passby press enter Control.rect
			_set_finger_index(sd->get_index());
			_update_direction_with_point(coord);
		} else if (get_finger_index() == sd->get_index()) //dragging dpad direction
			_update_direction_with_point(coord);
	}
}

void TouchScreenDPad::_update_direction_with_point(Point2& p_point) {
	int result = DIR_NEUTRAL;
	p_point -= (_get_center_point() + get_center_offset());
	Direction xAxis = p_point.x > 0 ? DIR_RIGHT : DIR_LEFT;
	Direction yAxis = p_point.y > 0 ? DIR_DOWN : DIR_UP;
	const Point2 point_abs = p_point.abs();

	const float s = MIN(_position_rect.size.x, _position_rect.size.y) / 2.0;
	const float w = get_single_direction_span() * s;
	const float l = get_neutral_extent() * s;
		
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

bool TouchScreenDPad::_set_neutral_extent(real_t p_extent) {
	p_extent = MAX(p_extent, get_single_direction_span());
	const float new_extent = MIN(0.9, p_extent);
	if(new_extent == get_neutral_extent())
		return false;
	return TouchScreenPad::_set_neutral_extent(new_extent);
}

bool TouchScreenDPad::_set_single_direction_span(real_t p_span) {
	p_span = MAX(p_span, 0.1);
	const float new_span = MIN(0.9, p_span);
	if (new_span == get_single_direction_span())
		return false;
	TouchScreenPad::_set_single_direction_span(new_span);
	set_neutral_extent( MAX(get_neutral_extent(), get_single_direction_span()) );
	return true;
}

Size2 TouchScreenDPad::get_minimum_size() const {
	if (scale_to_rect <= 0.0 && texture.is_valid())
		return texture->get_size();
	return iControl::get_minimum_size().abs();
}

void TouchScreenDPad::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				return;
			}
			if(is_update_pending())
				_update_cache();
			draw_texture_rect(texture, _position_rect);

			if (Engine::get_singleton()->is_editor_hint() && get_tree()->is_debugging_collisions_hint())
				_draw_shape();
		} break;
		case NOTIFICATION_RESIZED:
			_update_cache();
		break;
	}
}

void TouchScreenDPad::_update_cache() {
	if(texture.is_null())
		return;
	const Size2 size = scale_to_rect > 0.0?  get_size() * scale_to_rect : texture->get_size();
	const Point2 offs = is_centered()? (get_size() - size) / 2.0 : Point2(0, 0);

	_set_center_point((size / 2.0) + offs);
	_position_rect = Rect2(offs, size);

	if (Engine::get_singleton()->is_editor_hint() && get_tree()->is_debugging_collisions_hint()) 
		_update_shape_points(MIN(size.x, size.y));
}

Ref<Texture> TouchScreenDPad::get_texture() const {
	return texture;
}

void TouchScreenDPad::set_texture(Ref<Texture>& p_texture) {
	if(texture == p_texture)
		return;
	texture = p_texture;
	_update_cache_dirty();
	update();
	minimum_size_changed();
}

const float TouchScreenDPad::get_scale_to_rect() const {
	return scale_to_rect;
}

void TouchScreenDPad::set_scale_to_rect(const float p_scale) {
	if(scale_to_rect == p_scale)
		return
	scale_to_rect = MAX(p_scale, 0);
	_update_cache_dirty();
	update();
	minimum_size_changed();
}

void TouchScreenDPad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_input", "event"), &TouchScreenDPad::_input);

	ClassDB::bind_method(D_METHOD("get_texture"), &TouchScreenDPad::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TouchScreenDPad::set_texture);

	ClassDB::bind_method(D_METHOD("get_texture_scale"), &TouchScreenDPad::get_scale_to_rect);
	ClassDB::bind_method(D_METHOD("set_texture_scale", "scale"), &TouchScreenDPad::set_scale_to_rect);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "texture scale"), "set_texture_scale", "get_texture_scale");
}

TouchScreenDPad::TouchScreenDPad()
	: TouchScreenPad(0.65, 0.35)
{
	if (Engine::get_singleton()->is_editor_hint() && get_tree()->is_debugging_collisions_hint())
		_shape_points = Ref<ConvexPolygonShape2D>(memnew(ConvexPolygonShape2D));
}
