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
	p_span = MIN(p_span, Math_PI * 0.5);
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
	
	const InputEventScreenTouch *st = Object::cast_to<InputEventScreenTouch>(*p_event);
	if (st) {
		Point2 coord = get_global_transform_with_canvas().xform_inv(st->get_position());
		if (get_finger_index() == -1 && st->is_pressed() && _is_point_inside(coord)) {
			_set_finger_index(st->get_index());
			_touch_pos_on_initial_press = coord;
			_update_direction_with_point(coord);
		}
		else if (get_finger_index() == st->get_index()) {//on release
			_release();
			_touch_pos_on_initial_press = Point2(0, 0);
			_current_touch_pos = Point2(0, 0);
			if (speed_data) {
				emit_signal("direction_changed_with_speed", -1, get_direction(), speed_data->_drag_speed);
				emit_signal("angle_changed_with_rotation_speed", -1, get_angle(), speed_data->_rotation_speed);
				emit_signal("direction_and_angle_with_speed", -1, get_direction(), speed_data->_drag_speed, get_angle(), speed_data->_rotation_speed);

				speed_data->_prev_touch_pos = Point2(0, 0);
				speed_data->_drag_speed = Point2(0, 0);
				speed_data->_rotation_speed = 0;
			}
		}
		queue_redraw();
		return;
	}

	const InputEventScreenDrag *sd = Object::cast_to<InputEventScreenDrag>(*p_event);
	if (sd) {
		Point2 coord = get_global_transform_with_canvas().xform_inv(sd->get_position());
		if (is_passby_press() && get_finger_index() == -1 && _is_point_inside(coord)) { //passby press enter Control.rect
			_set_finger_index(sd->get_index());
			_touch_pos_on_initial_press = get_size() * 0.5;
			_update_direction_with_point(coord);
		} else if (get_finger_index() == sd->get_index()) //dragging dpad direction
			_update_direction_with_point(coord);
		queue_redraw();
	}
}

const bool TouchScreenJoystick::_is_point_inside(const Point2 p_point){
	return (Control::has_point(p_point) && (((get_size() * 0.5) + get_center_offset() - p_point).length() <= _get_radius()));
}

void TouchScreenJoystick::_update_direction_with_point(Point2 p_point) {
	p_point -= normal_moved_to_touch_pos? _touch_pos_on_initial_press : ((get_size() * 0.5) + get_center_offset());
	_current_touch_pos = p_point;
	Direction xAxis = p_point.x > 0 ? DIR_RIGHT : DIR_LEFT;
	Direction yAxis = p_point.y > 0 ? DIR_DOWN : DIR_UP;	

	Direction temp = DIR_NEUTRAL;
	if(p_point.length() > (get_deadzone_extent() * _get_radius())) {
		const real_t theta = ((Math_PI * 0.5) - get_cardinal_direction_span()) * 0.5;
		const real_t omega = p_point.abs().angle();
		if(omega < theta)
			temp = xAxis;
		else if(omega > ((Math_PI * 0.5) - theta))
			temp = yAxis;
		else
			temp = (Direction)(xAxis | yAxis);
	} 
	if(temp != get_direction()) {
		_set_direction(temp);
		_direction_changed();
	}
	emit_signal("angle_changed", get_finger_index(), p_point.angle());
}

Vector2 TouchScreenJoystick::SpeedMonitorData::update_drag_speed(const Point2 _current_touch_pos, const double delta) {
	const Vector2 displacement = (_current_touch_pos - _prev_touch_pos);
	Vector2 temp = ((displacement / delta) - _drag_speed) * 2.0;
	if ( (_drag_speed.x > 0? ((displacement.x >= 0) && ((_drag_speed.x + temp.x) < 0)) : ((displacement.x <= 0) && ((_drag_speed.x + temp.x) > 0))) ) // prevent overdamping by adding a damping factor //problem: oscillates when displacement is zero
		temp /= Math_E;
	if ( (_drag_speed.y > 0? ((displacement.y >= 0) && ((_drag_speed.y + temp.y) < 0)) : ((displacement.y <= 0) && ((_drag_speed.y + temp.y) > 0)))) // prevent overdamping by adding a damping factor //problem: oscillates when displacement is zero
		temp /= Math_E;
	_drag_speed += temp;
	return _drag_speed.abs();
}
real_t TouchScreenJoystick::SpeedMonitorData::update_rotation_speed(const Point2 _current_touch_pos, const double delta) {
	if (_prev_touch_pos.length() == 0)
		return 0;
	Transform2D temp(_prev_touch_pos.normalized(), (_prev_touch_pos.x > 0? (_prev_touch_pos.y > 0?
		Vector2(-1, 1): Vector2(1, 1)) : (_prev_touch_pos.y > 0 ? Vector2(-1, -1) : Vector2(1, -1))),
		Vector2(0, 0));
	temp.orthonormalize();
	const real_t delta_angle = (double)(temp.basis_xform_inv(_current_touch_pos.normalized()).angle());
	real_t res = ((delta_angle / delta) - _rotation_speed) * 2.0; 
	if ( (_rotation_speed > 0 ? (delta_angle >= 0 && (_rotation_speed + res) < 0) : (delta_angle <= 0 && (_rotation_speed + res) > 0)) )
		res /= Math_E; //have to add damping factor //problem: oscillates when displacement is zero
	_rotation_speed += res;
	return Math::abs(_rotation_speed);
}

void TouchScreenJoystick::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
			if (get_finger_index() == -1)
				break;
		{
			const double delta = get_physics_process_delta_time();
			emit_signal("direction_changed_with_speed", get_finger_index(), get_direction(), speed_data->update_drag_speed(_current_touch_pos, delta));
			emit_signal("angle_changed_with_rotation_speed", get_finger_index(), get_angle(), speed_data->update_rotation_speed(_current_touch_pos, delta));
			emit_signal("direction_and_angle_with_speed", get_finger_index(), get_direction(), speed_data->_drag_speed, get_angle(), speed_data->_rotation_speed);
			speed_data->_prev_touch_pos = _current_touch_pos;
		} break;
		case NOTIFICATION_DRAW: {
			if(is_update_cache())
				_update_cache();
			
			const bool is_pressed = get_finger_index() != -1;
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
					data.stick.texture, (is_pressed ?
						data.stick.move_to_position(((stick_confined_inside && (_current_touch_pos.length() > _get_radius())) ?
							_current_touch_pos.normalized() * _get_radius() : _current_touch_pos) + (normal_moved_to_touch_pos?
								_touch_pos_on_initial_press : (get_size() * 0.5))
						) : data.stick._position_rect
					) 
				);
#ifdef TOOLS_ENABLED
			if ((Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) && shape)
				shape->_draw(get_canvas_item(), get_tree()->get_debug_collisions_color(), radius == get_deadzone_extent());
#endif
		} break;
		case NOTIFICATION_RESIZED:
			_update_cache();
		break;
	}
}

void TouchScreenJoystick::_update_cache() {
		data.normal._update_texture_cache(get_size(), is_centered());
		data.pressed._update_texture_cache(get_size(), is_centered());
		data.stick._update_texture_cache(get_size(), is_centered());
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) {
		if(!shape)
			shape = new TouchScreenJoystick::Shape();
		shape->_update_shape_points((get_size() * 0.5) + get_center_offset(), _get_radius() , get_deadzone_extent(), get_cardinal_direction_span(), radius == get_deadzone_extent());
	}
#endif
}

void TouchScreenJoystick::Data::TextureData::_update_texture_cache(const Size2 parent_size, const bool is_centered) {
	if (texture.is_null())
		return;
	const Size2 size = (scale.length() == 0 ? texture->get_size() : (parent_size * scale));
	const Point2 offs = is_centered ? (parent_size - size) * 0.5 : Point2(0, 0);

	_position_rect = Rect2(offs, size);
}

real_t TouchScreenJoystick::get_angle() const {
	return _current_touch_pos.angle();
}
real_t TouchScreenJoystick::get_rotation_speed() const {
	if (speed_data)
		return speed_data->_rotation_speed;
	return 0;
}
Vector2 TouchScreenJoystick::get_drag_speed() const {
	if (speed_data)
		return speed_data->_drag_speed;
	return Vector2();
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

	ClassDB::bind_method(D_METHOD("toggle_normal_moved_to_touch_pos", "move"), &TouchScreenJoystick::toggle_normal_moved_to_touch_pos);
	ClassDB::bind_method(D_METHOD("is_normal_moved_to_touch_pos"), &TouchScreenJoystick::is_normal_moved_to_touch_pos);

	ClassDB::bind_method(D_METHOD("toggle_stick_confined_inside", "confined_inside"), &TouchScreenJoystick::toggle_stick_confined_inside);
	ClassDB::bind_method(D_METHOD("is_stick_confined_inside"), &TouchScreenJoystick::is_stick_confined_inside);

	ClassDB::bind_method(D_METHOD("toggle_monitor_speed", "monitor"), &TouchScreenJoystick::toggle_monitor_speed);
	ClassDB::bind_method(D_METHOD("is_monitoring_speed"), &TouchScreenJoystick::is_monitoring_speed);

	ClassDB::bind_method(D_METHOD("get_angle"), &TouchScreenJoystick::get_angle);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "show mode", PROPERTY_HINT_ENUM, "Show Stick On Touch,Show Stick & Normal On Touch,Show Stick When Inactive,Show All Always"), "set_show_mode", "get_show_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "move to touch pos"), "toggle_normal_moved_to_touch_pos", "is_normal_moved_to_touch_pos");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stick confined inside"), "toggle_stick_confined_inside", "is_stick_confined_inside");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitor_speed"), "toggle_monitor_speed", "is_monitoring_speed");
	ADD_GROUP("Normal", "normal_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "normal_scale"), "set_texture_scale", "get_texture_scale");
	ADD_GROUP("Pressed", "pressed_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "pressed_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_pressed", "get_texture_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "pressed_scale"), "set_texture_pressed_scale", "get_texture_pressed_scale");
	ADD_GROUP("Stick", "stick_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stick_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_stick_texture", "get_stick_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "stick_scale"), "set_stick_scale", "get_stick_scale");

	ADD_SIGNAL(MethodInfo("direction_and_angle_with_speed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::INT, "direction", PROPERTY_HINT_FLAGS),
		PropertyInfo(Variant::FLOAT, "speed"),
		PropertyInfo(Variant::FLOAT, "angle"),
		PropertyInfo(Variant::FLOAT, "rotation_speed")
	));

	ADD_SIGNAL(MethodInfo("direction_changed_with_speed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::INT, "direction", PROPERTY_HINT_FLAGS),
		PropertyInfo(Variant::FLOAT, "speed")
	));

	ADD_SIGNAL(MethodInfo("angle_changed_with_rotation_speed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::FLOAT, "angle"),
		PropertyInfo(Variant::FLOAT, "speed")
	));

	ADD_SIGNAL(MethodInfo("angle_changed",
		PropertyInfo(Variant::INT, "finger_pressed"),
		PropertyInfo(Variant::FLOAT, "angle")
	));

	BIND_ENUM_CONSTANT(SHOW_STICK_ON_TOUCH);
	BIND_ENUM_CONSTANT(SHOW_STICK_AND_NORMAL_ON_TOUCH);
	BIND_ENUM_CONSTANT(SHOW_STICK_WHEN_INACTIVE);
	BIND_ENUM_CONSTANT(SHOW_ALL_ALWAYS);
}

void TouchScreenJoystick::set_show_mode(const ShowMode p_show_mode) {
	show_mode = p_show_mode;
	_update_cache_dirty();
	queue_redraw();
}
TouchScreenJoystick::ShowMode TouchScreenJoystick::get_show_mode() const {
	return show_mode;
}

void TouchScreenJoystick::toggle_monitor_speed(const bool p_monitor_speed) {
	set_physics_process_internal(p_monitor_speed);
	if (p_monitor_speed && speed_data == nullptr) {
		speed_data = new TouchScreenJoystick::SpeedMonitorData();
		return;
	}
	delete speed_data;
	speed_data = nullptr;
}
bool TouchScreenJoystick::is_monitoring_speed() const {
	return speed_data;
}

void TouchScreenJoystick::toggle_normal_moved_to_touch_pos(const bool p_move_normal_to_touch_pos) {
	normal_moved_to_touch_pos = p_move_normal_to_touch_pos;
	_update_cache_dirty();
	if (get_finger_index() != -1) 
		queue_redraw();	
}
bool TouchScreenJoystick::is_normal_moved_to_touch_pos() const {
	return normal_moved_to_touch_pos;
}

void TouchScreenJoystick::set_radius(const real_t p_radius) {
	radius = MAX(p_radius, get_deadzone_extent());
#ifdef TOOLS_ENABLED
	if((Engine::get_singleton()->is_editor_hint() || (is_inside_tree() && get_tree()->is_debugging_collisions_hint())) && shape) {
		shape->_update_shape_points((get_size() * 0.5) + get_center_offset(), _get_radius(), get_deadzone_extent(), get_cardinal_direction_span(), radius == get_deadzone_extent());
		queue_redraw();
	}
#endif
}
real_t TouchScreenJoystick::get_radius() const {
	return radius;
}

void TouchScreenJoystick::toggle_stick_confined_inside(const bool p_confined_inside) {
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

void TouchScreenJoystick::set_texture(const Ref<Texture2D> p_texture) {
	data.normal.texture = p_texture;
	update_minimum_size();
	data.normal._update_texture_cache(get_size(), is_centered());
	if (get_finger_index() == -1 || (show_mode & 0b01))
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_texture() const {
	return data.normal.texture;
}
void TouchScreenJoystick::set_texture_scale(Vector2 p_scale) {
	p_scale.x = MAX(p_scale.x, 0);
	p_scale.y = MAX(p_scale.y, 0);
	data.normal.scale = p_scale;
	update_minimum_size();
	data.normal._update_texture_cache(get_size(), is_centered());
	if (data.normal.texture.is_valid() && (get_finger_index() == -1 || (show_mode & 0b01)))
		queue_redraw();
}
Vector2 TouchScreenJoystick::get_texture_scale() const {
	return data.normal.scale;
}
void TouchScreenJoystick::set_texture_pressed(const Ref<Texture2D> p_texture_pressed) {
	data.pressed.texture = p_texture_pressed;
	update_minimum_size();
	data.pressed._update_texture_cache(get_size(), is_centered());
	if (get_finger_index() != -1)
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_texture_pressed() const {
	return data.pressed.texture;
}
void TouchScreenJoystick::set_texture_pressed_scale(Vector2 p_scale) {
	p_scale.x = MAX(p_scale.x, 0);
	p_scale.y = MAX(p_scale.y, 0);
	data.pressed.scale = p_scale;
	update_minimum_size();
	data.pressed._update_texture_cache(get_size(), is_centered());
	if (data.pressed.texture.is_valid() && get_finger_index() != -1)
		queue_redraw();
}
Vector2 TouchScreenJoystick::get_texture_pressed_scale() const {
	return data.pressed.scale;
}
void TouchScreenJoystick::set_stick_texture(const Ref<Texture2D> p_stick) {
	data.stick.texture = p_stick;
	update_minimum_size();
	data.stick._update_texture_cache(get_size(), is_centered());
	if ((show_mode & 0b10) || get_finger_index() != -1)
		queue_redraw();
}
Ref<Texture2D> TouchScreenJoystick::get_stick_texture() const {
	return data.stick.texture;
}
void TouchScreenJoystick::set_stick_scale(Vector2 p_scale) {
	p_scale.x = MAX(p_scale.x, 0);
	p_scale.y = MAX(p_scale.y, 0);
	data.stick.scale = p_scale;
	update_minimum_size();
	data.stick._update_texture_cache(get_size(), is_centered());
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
	return Rect2(p_point - (_position_rect.size * 0.5), _position_rect.size);
}

TouchScreenJoystick::TouchScreenJoystick()
	: TouchScreenPad(0.4, 0.575), data()
{}

#ifdef TOOLS_ENABLED
TouchScreenJoystick::~TouchScreenJoystick() {
	if(shape)
		delete shape;
	if (speed_data)
		delete speed_data;
}

TouchScreenJoystick::Shape::Shape()
	: _deadzone_circle(), _direction_zones()
{
	_deadzone_circle.resize(24);
	_direction_zones.resize(8);
}
#else
TouchScreenJoystick::~TouchScreenJoystick() {
	if (speed_data)
		delete speed_data;
}
#endif

const real_t TouchScreenJoystick::_get_radius() const {
	return radius * MIN(get_size().x, get_size().y) * 0.5;
}

#ifdef TOOLS_ENABLED
void TouchScreenJoystick::Shape::_update_shape_points(const Point2 p_center, const real_t p_radius, const real_t p_deadzone, const real_t p_direction_span, const bool is_radius_equal_deadzone) {
	//draw points for circle in the middle and 8 or 4 quarter disks in the outer edges
	const real_t rad90deg = Math_PI * 0.5;
	const real_t angle = (p_direction_span + CMP_EPSILON < rad90deg ? rad90deg - p_direction_span : 0.0);
	const real_t half_angle = angle * 0.5;
	const real_t start_angle[4] = { (2.0f * Math_PI) - half_angle, (0.5f * Math_PI) - half_angle, -half_angle + Math_PI, (1.5f * Math_PI) - half_angle }; // (360, 90, 180, 270) - angle
	const real_t end_angle[4] = { half_angle,  (0.5f * Math_PI) + half_angle, half_angle + Math_PI, (1.5f * Math_PI) + half_angle }; // (0, 90, 180, 270) + angle
	unsigned int marked_edges[8] = { 0,0,0,0,0,0,0,0 }; // { upStart, upEnd, rightStart, rightEnd, downStart, downEnd, leftStart, leftEnd }
	for (int i = 0, j = 0, k = 1; i < 24; i++) {
		const real_t theta = Math_PI * i / 12.0f;
		_deadzone_circle.set(i, p_center + (Vector2(Math::cos(theta), Math::sin(theta)) * p_radius * p_deadzone)); // deadzone circle
		if (j < 4 && end_angle[j] <= theta) { // up, right, down, left
			marked_edges[1 + j * 2] = MAX(i - 1, 0);
			j++;
		}
		if (k < 5 && start_angle[k % 4] <= theta) { // right, down, left, up
			marked_edges[(k % 4) * 2] = i - 1;
			k++;
		}
	}

	if (is_radius_equal_deadzone)
		return;

	//[0-3] is Up Down Left Right
	if (p_direction_span + CMP_EPSILON < rad90deg) {
		const unsigned short int size = 2u + 4u * (angle / rad90deg); // outer ring // 2 for the start and end edges of the outer ring
		const real_t angle_increments = angle / (real_t)(size - 1);

		Vector<Point2> points;
		points.resize(size);
		for (int i = 0; i < size; ++i) {
			const real_t theta = (angle_increments * i) - (half_angle); // outer ring points
			points.set(i, Vector2(Math::cos(theta), Math::sin(theta)) * p_radius);
		}
		for (int i = 0; i < 4; ++i) { // outer ring of circle
			Vector<Point2> res;
			res.resize(3 + size + (i ? marked_edges[1 + i * 2] - marked_edges[i * 2] : ((marked_edges[1] + 24 - marked_edges[0]) % 24))); // +2 for start and end edges of the inner ring and +1 for extra size
			for (int j = 0; j < size; ++j) {
				const Vector2& point = points.get(j);
				if (i % 2)  //outer ring points plus center and if up, down, left, right
					res.set(j, p_center + Vector2(point.y, point.x) * (i % 3 ? Vector2(-1, 1) : Vector2(1, -1))); // i is 1 or 3; 1 is left; 3 is right
				else
					res.set(j, p_center + point * (i ? Vector2(-1, -1) : Vector2(1, 1))); // i is 0 or 2; 2 is down; 0 is up
			}
			res.set(size, p_center + (Vector2(Math::cos(end_angle[i]), Math::sin(end_angle[i])) * p_radius * p_deadzone)); // inner ring of the circle end edge
			for (int index = marked_edges[i * 2], res_rend = res.size() - 2; index != marked_edges[1 + i * 2] + 1; --res_rend) { // j is startEdge and condition is if j != endEdge + 1
				res.set(res_rend, _deadzone_circle.get(index));
				++index %= 24;
			}
			res.set(res.size() - 1, p_center + (Vector2(Math::cos(start_angle[i]), Math::sin(start_angle[i])) * p_radius * p_deadzone)); // inner ring of the circle start edge
			_direction_zones.set(i, res);
		}
	}
	else
		_direction_zones.set(0, Vector<Vector2>());

	if (!(p_direction_span > CMP_EPSILON)) {
		_direction_zones.set(4, Vector<Vector2>());
		return;
	}
	//[4-7] is UpRight UpLeft DownRight DownLeft
	const unsigned short int size = 2u + 4u * (p_direction_span / rad90deg);
	const real_t angle_increments = p_direction_span / (real_t)(size - 1);
	Vector<Point2> points;
	points.resize(size);
	for (int i = 0; i < size; ++i) {
		const real_t theta = (angle_increments * i) + half_angle;
		points.set(i, Vector2(Math::cos(theta), Math::sin(theta)) * p_radius);
	}
	for (int i = 0; i < 4; ++i) {
		Vector<Point2> res;
		res.resize(3 + size + (i != 3 ? marked_edges[(2 + i * 2) % 8] - marked_edges[1 + i * 2] : (marked_edges[0] + 24 - marked_edges[7]) % 24)); // end edge of up subtracted by start edge of right... and so on so forth for the other edges
		for (int j = 0; j < size; ++j) {
			const Vector2& point = points.get(j);
			if (i % 2) //downright, downleft
				res.set(j, p_center + Vector2(point.y, point.x) * (i % 3 ? Vector2(-1, 1) : Vector2(1, -1)));
			else  //upright, downleft
				res.set(j, p_center + point * (i ? Vector2(-1, -1) : Vector2(1, 1)));
		}
		res.set(size, p_center + (Vector2(Math::cos(start_angle[(i + 1) % 4]), Math::sin(start_angle[(i + 1) % 4])) * p_radius * p_deadzone));
		for (int index = marked_edges[1 + i * 2], res_rend = res.size() - 2; index != marked_edges[(2 + i * 2) % 8] + 1; --res_rend) {
			res.set(res_rend, _deadzone_circle.get(index));
			++index %= 24;
		}
		res.set(res.size() - 1, p_center + (Vector2(Math::cos(end_angle[i]), Math::sin(end_angle[i])) * p_radius * p_deadzone));
		_direction_zones.set(i + 4, res);
	}
}

void TouchScreenJoystick::Shape::_draw(const RID& p_rid_to, Color pallete, const bool is_radius_equal_deadzone) {
	Vector<Color> a_pallete; a_pallete.push_back(pallete.darkened(0.15));
	_add_to_canvas(p_rid_to, _deadzone_circle, a_pallete);

	if (is_radius_equal_deadzone)
		return;

	if (_direction_zones[0].size() != 0) {
		Vector<Color> a_pallete2; a_pallete2.push_back(pallete);
		for (int i = 0; i < 4; ++i)
			_add_to_canvas(p_rid_to, _direction_zones[i], a_pallete2);
	}

	if (_direction_zones[4].size() != 0) {
		Vector<Color> a_pallete3; a_pallete3.push_back(pallete.lightened(0.15));
		for (int i = 4; i < 8; ++i)
			_add_to_canvas(p_rid_to, _direction_zones[i], a_pallete3);
	}
}

void TouchScreenJoystick::Shape::_add_to_canvas(const RID p_rid_to, const Vector<Vector2>& p_points, const Vector<Color>& p_color) {
	RenderingServer::get_singleton()->canvas_item_add_polygon(p_rid_to, p_points, p_color);
	RenderingServer::get_singleton()->canvas_item_add_polyline(p_rid_to, p_points, p_color, 1.0, true);
	// Draw the last segment as it's not drawn by `canvas_item_add_polyline()`.
	RenderingServer::get_singleton()->canvas_item_add_line(p_rid_to, p_points[p_points.size() - 1], p_points[0], p_color[0], 1.0, true);
}
#endif
