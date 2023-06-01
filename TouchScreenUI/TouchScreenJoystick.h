#ifndef TOUCH_SCREEN_JOYSTICK
#define TOUCH_SCREEN_JOYSTICK

#include "core/object/ref_counted.h"
#include "TouchScreenPad.h"
#include "scene/resources/texture.h"
#include "scene/resources/circle_shape_2d.h"

class TouchScreenJoystick : public TouchScreenPad {
	GDCLASS(TouchScreenJoystick, TouchScreenPad);

public:
	enum ShowMode {
		SHOW_STICK_ON_TOUCH = 0b00,			// stick not visible when inactive, normal not visible on press
		SHOW_STICK_AND_NORMAL_ON_TOUCH = 0b01, // stick not visible when inactive, normal always visible
		SHOW_STICK_WHEN_INACTIVE = 0b10,	// stick always visible, normal not visible on press
		SHOW_ALL_ALWAYS = 0b11				// stick and normal always visible
	};

private:
#ifdef TOOLS_ENABLED
	class Shape {
		Vector<Point2> _deadzone_circle;
		Vector<Vector<Point2>> _direction_zones;

		void _add_to_canvas(const RID p_rid_to, const Vector<Vector2>& p_points, const Vector<Color>& p_color);
	public:
		Shape();

		void _update_shape_points(const Point2 p_center, const real_t p_radius, const real_t p_deadzone, const real_t p_direction_span, const bool is_radius_equal_deadzone);
		void _draw(const RID& p_rid_to, Color pallete, const bool is_radius_equal_deadzone);
	} * shape = nullptr;
#endif

	struct Data {
		struct TextureData {
			Ref<Texture2D> texture = Ref<Texture2D>();
			Rect2 _position_rect = Rect2();
			Vector2 scale = Vector2();

			_FORCE_INLINE_ const Rect2 move_to_position(const Point2& p_point) const;
			void _update_texture_cache(const Size2 parent_size, const bool is_centered);
		} normal, pressed, stick;

		Data();
	} data;
	real_t radius = 1.0;
	
	ShowMode show_mode = SHOW_ALL_ALWAYS;
	bool stick_confined_inside = false; //keep clip content false
	bool normal_moved_to_touch_pos = false;

	Point2 _touch_pos_on_initial_press = Point2();
	Point2 _current_touch_pos = Point2();

	struct SpeedMonitorData {
		Point2 _prev_touch_pos = Point2();
		Vector2 _drag_speed = Vector2();
		real_t _rotation_speed = 0;

		Vector2 update_drag_speed(const Point2 _current_touch_pos, const double delta);
		real_t update_rotation_speed(const Point2 _current_touch_pos, const double delta);
	} * speed_data = nullptr;
protected:
	const bool _set_deadzone_extent(real_t p_extent); // radius of a circle
	const bool _set_cardinal_direction_span(real_t p_span); // a radian of an angle, no more than 90 degrees
	virtual Size2 get_minimum_size() const override;

	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_radius(const real_t p_radius);
	float get_radius() const;

	void set_texture(const Ref<Texture2D> p_texture);
	Ref<Texture2D> get_texture() const;

	void set_texture_scale(Vector2 p_scale);
	Vector2 get_texture_scale() const;

	void set_texture_pressed(const Ref<Texture2D> p_texture_pressed);
	Ref<Texture2D> get_texture_pressed() const;

	void set_texture_pressed_scale(Vector2 p_scale);
	Vector2 get_texture_pressed_scale() const;

	void set_stick_texture(const Ref<Texture2D> p_stick);
	Ref<Texture2D> get_stick_texture() const;

	void set_stick_scale(Vector2 p_scale);
	Vector2 get_stick_scale() const;

	void set_show_mode(const ShowMode p_show_mode);
	ShowMode get_show_mode() const;

	void toggle_normal_moved_to_touch_pos(const bool p_move_normal_to_touch_pos);
	bool is_normal_moved_to_touch_pos() const;

	void toggle_stick_confined_inside(const bool p_confined_inside);
	bool is_stick_confined_inside() const;

	void toggle_monitor_speed(const bool p_monitor_speed);
	bool is_monitoring_speed() const;

	real_t get_angle() const;
	real_t get_rotation_speed() const;
	Vector2 get_drag_speed() const;

	TouchScreenJoystick();
	~TouchScreenJoystick();
private:
	virtual void input(const Ref<InputEvent>& p_event) override;

	void _update_direction_with_point(Point2 p_point);
	inline const bool _is_point_inside(const Point2 p_point);

	void _update_cache();
	inline const real_t _get_radius() const;
};

VARIANT_ENUM_CAST(TouchScreenJoystick::ShowMode);

/**
	internal physics process delta time is used to monitor held time
	should I give an option for either use that or use internal process delta time, instead?
*/

#endif
