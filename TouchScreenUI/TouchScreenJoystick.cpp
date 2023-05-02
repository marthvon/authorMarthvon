#include "TouchScreenJoystick.h"

#include "core/os/os.h"
#include "servers/rendering_server.h"
#include "core/math/color.h"
#include "core/templates/vector.h"
#include "core/input/input_event.h"
#include "core/config/project_settings.h"

const bool TouchScreenJoystick::_set_deadzone_extent(real_t p_extent) {
	p_extent = MAX(p_extent, 0.0);
	p_extent = MIN(p_extent, radius);
	if(p_extent == get_deadzone_extent())
		return false;
	return TouchScreenPad::_set_deadzone_extent(p_extent);
}

const bool TouchScreenJoystick::_set_cardinal_direction_span(real_t p_span) {
	p_span = MAX(p_span, 0.0);
	p_span = MIN(p_span, 0.5f * Math_PI);
	if (p_span == get_cardinal_direction_span())
		return false;
	return TouchScreenPad::_set_cardinal_direction_span(p_span);
}

Size2 TouchScreenJoystick::get_minimum_size() const {
	Size2 rscale = Size2();

	if (data.normal.texture.is_valid()) {
		rscale.x = MAX(rscale.x, data.normal.texture->get_size().x * data.normal.scale.x);
		rscale.y = MAX(rscale.y, data.normal.texture->get_size().y * data.normal.scale.y);
	}

	if (data.pressed.texture.is_valid()) {
		rscale.x = MAX(rscale.x, data.pressed.texture->get_size().x * data.pressed.scale.x);
		rscale.y = MAX(rscale.y, data.pressed.texture->get_size().y * data.pressed.scale.y);
	}

	if (data.stick.texture.is_valid()) {
		rscale.x = MAX(rscale.x, data.stick.texture->get_size().x * data.stick.scale.x);
		rscale.y = MAX(rscale.y, data.stick.texture->get_size().y * data.stick.scale.y);
	}

	if (rscale.length() == 0.0)
		return TouchControl::get_minimum_size();

	return rscale;
}

void TouchScreenJoystick::input(const Ref<InputEvent>& p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (!is_visible_in_tree()) {
		return;
	}
	//2 func need update direction with point and update center with point
	const InputEventScreenTouch *st = Object::cast_to<InputEventScreenTouch>(*p_event);
	if (st) {
		Point2 coord = get_global_transform_with_canvas().affine_inverse().xform(st->get_position());
		if (get_finger_index() == -1 && st->is_pressed() && Control::has_point(coord) && _update_center_with_point(coord, normal_moved_to_touch_pos)) { //press inside Control.rect
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
		if (is_passby_press() && get_finger_index() == -1 && Control::has_point(coord) && _update_center_with_point(coord, false)) { //passby press enter Control.rect
			_set_finger_index(sd->get_index());
			_update_direction_with_point(coord);
		} else if (get_finger_index() == sd->get_index()) {//dragging dpad direction
			if(_update_direction_with_point(coord))
				emit_signal("direction_changed_with_speed", get_finger_index(), get_direction(), sd->get_velocity(), coord.angle());
			emit_signal("angle_changed_with_speed", coord.angle(), sd->get_velocity());
		}
	}
}

bool TouchScreenJoystick::_update_center_with_point(Point2 p_point, const bool is_point_center){
	if((p_point - (_get_center_point() + get_center_offset())).length() > radius)
		return false;
	_touch_pos_on_initial_press = is_point_center? p_point : _get_center_point() + get_center_offset();
	return true;
}

bool TouchScreenJoystick::_update_direction_with_point(Point2 p_point) {
	_current_touch_pos = p_point;

	p_point -= normal_moved_to_touch_pos? _touch_pos_on_initial_press : (_get_center_point() + get_center_offset());
	Direction xAxis = p_point.x > 0 ? DIR_RIGHT : DIR_LEFT;
	Direction yAxis = p_point.y > 0 ? DIR_DOWN : DIR_UP;	

	Direction temp = DIR_NEUTRAL;
	if(p_point.length() > get_deadzone_extent()){
		const float theta = (get_cardinal_direction_span() - (Math_PI * 0.5)) / 2.0;
		const float omega = p_point.abs().angle();
		if(omega < theta)
			temp = (Direction)(xAxis);
		else if(omega > ((Math_PI * 0.5) - theta))
			temp = (Direction)(yAxis);
		else
			temp = (Direction)(xAxis | yAxis);
	} 
	if(temp != get_direction()) {
		_set_direction(temp);
		_direction_changed();
		return true;
	}
	return false;
}

void TouchScreenJoystick::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if(is_update_cache())
				_update_cache();
			
			bool is_pressed = get_finger_index() != 1;
			if (data.normal.texture.is_valid() && ((show_mode & SHOW_STICK_AND_NORMAL_ON_TOUCH) || !is_pressed))
				draw_texture_rect(
					data.normal.texture, ( normal_moved_to_touch_pos && is_pressed? 
						data.normal.move_to_position(_touch_pos_on_initial_press) : data.normal._position_rect
					) 
				);

			if (data.pressed.texture.is_valid() && is_pressed)
				draw_texture_rect(
					data.pressed.texture, ( normal_moved_to_touch_pos && is_pressed? 
						data.pressed.move_to_position(_touch_pos_on_initial_press) : data.pressed._position_rect
					) 
				);

			if (data.stick.texture.is_valid() && ((show_mode & SHOW_STICK_WHEN_INACTIVE) || is_pressed))
				draw_texture_rect(
					data.stick.texture, ( is_pressed? 
						data.stick.move_to_position( stick_confined_inside && _current_touch_pos.length() > radius? 
							_current_touch_pos.normalized() * radius : _current_touch_pos
						) : data.stick._position_rect
					) 
				);

			if ((Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) && shape)
				shape->_draw(get_canvas_item());
		} break;
		case NOTIFICATION_RESIZED:
			_update_cache();
		break;
	}
}

void TouchScreenJoystick::_update_cache() {
	if(is_centered())
		_set_center_point(get_size() / 2.0);
	else {
		const float min_size_half = MIN(get_size().x, get_size().y) / 2.0;
		_set_center_point(Point2(min_size_half, min_size_half));
	}

	if(data.normal.texture.is_valid())
		_update_texture_cache(data.normal);
	if(data.pressed.texture.is_valid())
		_update_texture_cache(data.pressed);
	if(data.stick.texture.is_valid()) {
		_update_texture_cache(data.stick);
	}

	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
		if(!shape)
			shape = new TouchScreenJoystick::Shape();
		shape->_update_shape_points(_get_center_point() + get_center_offset(), get_radius(), get_deadzone_extent(), get_cardinal_direction_span());
	}
}

void TouchScreenJoystick::_update_texture_cache(TouchScreenJoystick::Data::TextureData& p_data) {
	if (p_data.texture.is_null())
		return;
	const Size2 size = p_data.texture->get_size() * (p_data.scale.length() == 0? Size2(1, 1) : p_data.scale);
	const Point2 offs = is_centered()? (get_size() - size) / 2.0 : Point2(0, 0);
	
	p_data._position_rect = Rect2(offs, size);
}

void TouchScreenJoystick::Shape::_update_shape_points(const Point2 p_center, const real_t p_radius, const real_t p_deadzone, const real_t p_direction_span) {
	return;
	const float rad90deg = Math_PI * 0.5;
	const float angle = rad90deg - p_direction_span;
	const float new_angle[4] =  { (2.0f * Math_PI) - (angle/2.0f), (0.5f * Math_PI) - (angle/2.0f), -(angle/2.0f) + Math_PI, (1.5f * Math_PI) - (angle/2.0f)}; // (360, 90, 180, 270) - angle
	const float end_angle[4] =  { (angle/2.0f),  (0.5f * Math_PI) + (angle/2.0f), (angle/2.0f) + Math_PI, (1.5f * Math_PI) + (angle/2.0f)}; // (360, 90, 180, 270) + angle
	unsigned int marks[8] = {0,0,0,0,0,0,0,0};
	for (int i = 0, j = 0, k = 1; i < 24; i++) {
		const float theta = Math_PI * i / 12.0f;
		_deadzone_circle.set(i, p_center + (Vector2(Math::cos(theta), Math::sin(theta)) * p_radius * p_deadzone)); // deadzone circle
		if(j < 4 && end_angle[j] <= theta) {
			const int num = i - 1;
			marks[1 + j * 2] = MAX(num, 0);
			j++;
		}
		if(k < 5 && new_angle[k % 4] <= theta) { 
			marks[(k % 4) * 2] = i - 1; 
			k++;
		}
	}

	//[0-3] is U D L R
	if(p_direction_span < (rad90deg - CMP_EPSILON)) {
		unsigned short int size = 2u + 4u * (angle/rad90deg); // outer ring
		const float angle_increments = angle/(float)(size - 1);
		
		Vector<Point2> points;
		points.resize(size);
		for(int i = 0; i < size; ++i) {
			const float theta = (angle_increments * i) - (angle/2.0);
			points.set(i, Vector2(Math::cos(theta), Math::sin(theta)) * p_radius);
		}
		for(int i = 0; i < 4; ++i) {
			Vector<Point2> res;
			const int mark_size = 1 + (i? marks[1 + i * 2] - marks[i * 2] : (marks[1] + 24 - marks[0]) % 24);
			const int new_size = size + mark_size + 2;
			res.resize(new_size);
			for(int j = 0; j < size; ++j) {
				const Vector2& point = points.get(j);
				if(i % 2) 
					res.set(j, p_center + Vector2(point.y, point.x) * (i % 3? Vector2(-1, 1) : Vector2(1, -1)) );
				else 
					res.set(j, p_center + point * (i? Vector2(-1, -1) : Vector2(1, 1)) );
			}
			res.set(size, p_center + (Vector2(Math::cos(end_angle[i]), Math::sin(end_angle[i])) * p_radius * p_deadzone));
			for(int j = marks[i * 2], k = new_size - 1; j != marks[1 + i * 2] + 1; j = (j + 1) % 24) {
				res.set(k, _deadzone_circle.get(j));
				k--;
			}
			res.set(new_size, p_center + (Vector2(Math::cos(new_angle[i]), Math::sin(new_angle[i])) * p_radius * p_deadzone));
			_direction_zones.set(i, res);
		}
	} else 
		_direction_zones.set(0, Vector<Vector2>());
	/*
	if (!(p_direction_span > CMP_EPSILON)) {
		_direction_zones.set(4, Vector<Vector2>());
		return;
	}
	//[4-7] is UR UL DR DL 
	unsigned short int size = 2u + 4u * (p_direction_span/rad90deg);
	const float angle_increments = angle/(float)(size - 1);
	Vector<Point2> points;
	points.resize(size);
	for(int i = 0; i < size; ++i) {
		const float theta = (angle_increments * i) + (angle/2.0);
		points.set(i, Vector2(Math::cos(theta), Math::sin(theta)) * p_radius);
	}
	for(int i = 0; i < 4; ++i) {
		Vector<Point2>& res = _direction_zones.get(i + 4);
		const int mark_size = 1 + marks[(2 + i * 2) % 8] - marks[1 + i * 2];
		const int new_size = size + mark_size + 2;
		res.resize(new_size);
		for(int j = 0; j < size; ++j) {
			const Vector2& point = points.get(j);
			if(i % 2) 
				res.set(j, p_center + Vector2(point.y, point.x) * (i % 3? Vector2(-1, 1) : Vector2(1, -1)) );
			else 
				res.set(j, p_center + point * (i? Vector2(-1, -1) : Vector2(1, 1)) );
		}
		res.set(size, p_center + (Vector2(Math::cos(new_angle[(i + 1) % 4]), Math::sin(new_angle[(i + 1) % 4])) * p_radius * p_deadzone));
		for(int j = marks[1 + i * 2], k = new_size - 1; j != marks[(2 + i * 2) % 8] + 1; j = (j + 1) % 24) { 
			res.set(k, _deadzone_circle.get(j));
			k--;
		}
		res.set(new_size, p_center + (Vector2(Math::cos(end_angle[i]), Math::sin(end_angle[i])) * p_radius * p_deadzone));
	}
	*/
}

void TouchScreenJoystick::Shape::_draw(const RID& p_rid_to) {
	return;
	Color pallete(0.7, 0.7, 0.7, 0.35); //get_tree()->get_debug_collisions_color();

	Vector<Color> a_pallete; a_pallete.push_back(pallete.darkened(0.15));
	_add_to_canvas(p_rid_to, _deadzone_circle, a_pallete);

	if(_direction_zones[0].size() != 0) {
		Vector<Color> a_pallete2; a_pallete2.push_back(pallete);
		for (int i = 0; i < 4; ++i)
			_add_to_canvas(p_rid_to, _direction_zones[i], a_pallete2);
	}

	if(_direction_zones[4].size() != 0) {
		Vector<Color> a_pallete3; a_pallete3.push_back(pallete.lightened(0.15));
		for(int i = 4; i < 8; ++i)
			_add_to_canvas(p_rid_to, _direction_zones[i], a_pallete3); 
	}
}

void TouchScreenJoystick::Shape::_add_to_canvas(const RID p_rid_to, const Vector<Vector2>& p_points, const Vector<Color>& p_color) {
	RenderingServer::get_singleton()->canvas_item_add_polygon(p_rid_to, p_points, p_color);
	RenderingServer::get_singleton()->canvas_item_add_polyline(p_rid_to, p_points, p_color, 1.0, true);
	// Draw the last segment as it's not drawn by `canvas_item_add_polyline()`.
	RenderingServer::get_singleton()->canvas_item_add_line(p_rid_to, p_points[p_points.size() - 1], p_points[0], p_color[0], 1.0, true);
}

void TouchScreenJoystick::_bind_methods(){
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TouchScreenJoystick::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &TouchScreenJoystick::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture_scale", "scale"), &TouchScreenJoystick::set_texture_scale);
	ClassDB::bind_method(D_METHOD("get_texture_scale"), &TouchScreenJoystick::get_texture_scale);

	ClassDB::bind_method(D_METHOD("set_texture_pressed", "texture"), &TouchScreenJoystick::set_texture_pressed);
	ClassDB::bind_method(D_METHOD("get_texture_pressed"), &TouchScreenJoystick::get_texture_pressed);
	ClassDB::bind_method(D_METHOD("set_texture_pressed_scale", "scale"), &TouchScreenJoystick::set_texture_pressed_scale);
	ClassDB::bind_method(D_METHOD("get_texture_pressed_scale"), &TouchScreenJoystick::get_texture_pressed_scale);

	ClassDB::bind_method(D_METHOD("set_stick_texture", "texture"), &TouchScreenJoystick::set_stick_texture);
	ClassDB::bind_method(D_METHOD("get_stick_texture"), &TouchScreenJoystick::get_stick_texture);
	ClassDB::bind_method(D_METHOD("set_stick_scale", "scale"), &TouchScreenJoystick::set_stick_scale);
	ClassDB::bind_method(D_METHOD("get_stick_scale"), &TouchScreenJoystick::get_stick_scale);

	ClassDB::bind_method(D_METHOD("set_show_mode", "show_mode"), &TouchScreenJoystick::set_show_mode);
	ClassDB::bind_method(D_METHOD("get_show_mode"), &TouchScreenJoystick::get_show_mode);

	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &TouchScreenJoystick::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &TouchScreenJoystick::get_radius);

	ClassDB::bind_method(D_METHOD("set_normal_moved_to_touch_pos", "move"), &TouchScreenJoystick::set_normal_moved_to_touch_pos);
	ClassDB::bind_method(D_METHOD("is_normal_moved_to_touch_pos"), &TouchScreenJoystick::is_normal_moved_to_touch_pos);

	ClassDB::bind_method(D_METHOD("set_stick_confined_inside", "confined_inside"), &TouchScreenJoystick::set_stick_confined_inside);
	ClassDB::bind_method(D_METHOD("is_stick_confined_inside"), &TouchScreenJoystick::is_stick_confined_inside);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "show mode", PROPERTY_HINT_ENUM, "Show Stick On Touch,Show Stick & Normal On Touch,Show Stick When Inactive,Show All Always"), "set_show_mode", "get_show_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "move to touch pos"), "set_normal_moved_to_touch_pos", "is_normal_moved_to_touch_pos");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stick confined inside"), "set_stick_confined_inside", "is_stick_confined_inside");
	ADD_GROUP("Normal", "normal_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "normal_scale"), "set_texture_scale", "get_texture_scale");
	ADD_GROUP("Pressed", "pressed_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "pressed_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_pressed", "get_texture_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "pressed_scale"), "set_texture_pressed_scale", "get_texture_pressed_scale");
	ADD_GROUP("Stick", "stick_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stick_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_stick_texture", "get_stick_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "stick_scale"), "set_stick_scale", "get_stick_scale");

	ADD_SIGNAL(MethodInfo("direction_changed_with_speed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::INT, "direction", PROPERTY_HINT_FLAGS),
		PropertyInfo(Variant::FLOAT, "speed"),
		PropertyInfo(Variant::FLOAT, "angle")
	));

	ADD_SIGNAL(MethodInfo("angle_changed_with_speed",
		PropertyInfo(Variant::FLOAT, "angle"),
		PropertyInfo(Variant::FLOAT, "speed")
	));

	BIND_ENUM_CONSTANT(SHOW_STICK_ON_TOUCH);
	BIND_ENUM_CONSTANT(SHOW_STICK_AND_NORMAL_ON_TOUCH);
	BIND_ENUM_CONSTANT(SHOW_STICK_WHEN_INACTIVE);
	BIND_ENUM_CONSTANT(SHOW_ALL_ALWAYS);
}

void TouchScreenJoystick::set_show_mode(ShowMode p_show_mode) {
	show_mode = p_show_mode;
	_update_cache_dirty();
	queue_redraw();
}
TouchScreenJoystick::ShowMode TouchScreenJoystick::get_show_mode() const {
	return show_mode;
}

void TouchScreenJoystick::set_normal_moved_to_touch_pos(bool p_move_normal_to_touch_pos) {
	normal_moved_to_touch_pos = p_move_normal_to_touch_pos;
	_update_cache_dirty();
	if (get_finger_index() != -1) 
		queue_redraw();	
}
bool TouchScreenJoystick::is_normal_moved_to_touch_pos() const {
	return normal_moved_to_touch_pos;
}

void TouchScreenJoystick::set_radius(float p_radius) {
	radius = p_radius;
	if((Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) && shape) {
		shape->_update_shape_points(_get_center_point() + get_center_offset(), get_radius(), get_deadzone_extent(), get_cardinal_direction_span());
		queue_redraw();
	}
}
float TouchScreenJoystick::get_radius() const {
	return radius;
}

void TouchScreenJoystick::set_stick_confined_inside(bool p_confined_inside) {
	stick_confined_inside = p_confined_inside;
	_update_cache_dirty();
	if(!stick_confined_inside)
		set_clip_contents(false);
	if (data.stick.texture.is_valid() && ((show_mode & SHOW_STICK_AND_NORMAL_ON_TOUCH) || get_finger_index() != -1)) 
		queue_redraw();
}
bool TouchScreenJoystick::is_stick_confined_inside() const {
	return stick_confined_inside;
}

void TouchScreenJoystick::set_texture(Ref<Texture2D> p_texture) {
	data.normal.texture = p_texture;
	update_minimum_size();
	_update_texture_cache(data.normal);
	if (p_texture.is_valid() && (get_finger_index() == -1 || (show_mode & 0b01))) 
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_texture() const {
	return data.normal.texture;
}
void TouchScreenJoystick::set_texture_scale(Vector2 p_scale) {
	data.normal.scale = p_scale;
	update_minimum_size();
	_update_texture_cache(data.normal);
	if (data.normal.texture.is_valid() && (get_finger_index() == -1 || (show_mode & 0b01)))
		queue_redraw();
}
Vector2 TouchScreenJoystick::get_texture_scale() const {
	return data.normal.scale;
}
void TouchScreenJoystick::set_texture_pressed(Ref<Texture2D> p_texture_pressed) {
	data.pressed.texture = p_texture_pressed;
	update_minimum_size();
	_update_texture_cache(data.pressed);
	if (p_texture_pressed.is_valid() && get_finger_index() != -1)
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_texture_pressed() const {
	return data.pressed.texture;
}
void TouchScreenJoystick::set_texture_pressed_scale(Vector2 p_scale) {
	data.pressed.scale = p_scale;
	update_minimum_size();
	_update_texture_cache(data.pressed);
	if (data.pressed.texture.is_valid() && get_finger_index() != -1)
		queue_redraw();
}
Vector2 TouchScreenJoystick::get_texture_pressed_scale() const {
	return data.pressed.scale;
}
void TouchScreenJoystick::set_stick_texture(Ref<Texture2D> p_stick) {
	data.stick.texture = p_stick;
	update_minimum_size();
	_update_texture_cache(data.stick);
	if (p_stick.is_valid() && ((show_mode & 0b10) || get_finger_index() != -1))
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_stick_texture() const {
	return data.stick.texture;
}
void TouchScreenJoystick::set_stick_scale(Vector2 p_scale) {
	data.stick.scale = p_scale;
	update_minimum_size();
	_update_texture_cache(data.stick);
	if (data.stick.texture.is_valid() && ((show_mode & 0b10) || get_finger_index() != -1)) 
		queue_redraw();
}
Vector2 TouchScreenJoystick::get_stick_scale() const {
	return data.stick.scale;
}

TouchScreenJoystick::Data::Data() 
	: normal(), pressed(), stick()
{
	normal.scale = Vector2(1.0, 1.0);
	pressed.scale = Vector2(1.4, 1.4);
	stick.scale = Vector2(0.3, 0.3);
}
			
_FORCE_INLINE_ const Rect2 TouchScreenJoystick::Data::TextureData::move_to_position(const Point2& p_point) const {
	return Rect2(p_point - (_position_rect.size / 2.0), _position_rect.size);
}

TouchScreenJoystick::TouchScreenJoystick()
	: TouchScreenPad(0.4, 0.575), data(), shape(nullptr)
{}

TouchScreenJoystick::~TouchScreenJoystick() {
	if(shape)
		delete shape;
}

TouchScreenJoystick::Shape::Shape() 
	: _deadzone_circle(), _direction_zones()
{
	_deadzone_circle.resize(24);
	_deadzone_circle.resize(8);
}
