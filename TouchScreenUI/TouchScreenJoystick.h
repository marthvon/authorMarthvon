#ifndef TOUCH_SCREEN_JOYSTICK
#define TOUCH_SCREEN_JOYSTICK

#include "core/reference.h"
#include "TouchScreenPad.h"
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
	class Shape {
		Vector<Point2> _deadzone_circle;
		Vector<Vector<Point2>> _direction_zones;

		void _add_to_canvas(const RID& p_rid_to, const Vector<Vector2>& p_points, const Vector<Color>& p_color);
	public:
		Shape();

		void _update_shape_points(const Point2& p_center, const real_t p_radius, const real_t p_deadzone, const real_t p_direction_span);
		void _draw(const RID& p_rid_to);
	} * const shape;

	struct Data {
		struct TextureData {
			Ref<Texture> texture = Ref<Texture>();
			Rect2 _position_rect = Rect2();
			Vector2 scale = Vector2();

			_FORCE_INLINE_ const Rect2 move_to_position(const Point2& p_point) const;
		} normal, pressed, stick;

		Data();
	} data;
	float radius = 0.0;
	
	ShowMode show_mode = SHOW_ALL_ALWAYS;
	bool stick_confined_inside = false; //keep clip content false
	bool normal_moved_to_touch_pos = false;
	bool monitor_speed = false;

	Point2 _touch_pos_on_initial_press = Point2();
	Point2 _current_touch_pos = Point2();
protected:
	bool _set_neutral_extent(real_t p_extent); // radius of a circle
	bool _set_single_direction_span(real_t p_span); // a radian of an angle, no more than 90 degrees
	virtual Size2 get_minimum_size() const override;

	void _notifications(int p_what);
	static void _bind_methods();

public:
	void set_radius(const float p_radius);
	const float get_radius() const;

	void set_texture(Ref<Texture>& p_texture);
	Ref<Texture> get_texture() const;

	void set_texture_scale(const Vector2& p_scale);
	const Vector2 get_texture_scale() const;

	void set_texture_pressed(Ref<Texture> &p_texture_pressed);
	Ref<Texture> get_texture_pressed() const;

	void set_texture_pressed_scale(const Vector2& p_scale);
	const Vector2 get_texture_pressed_scale() const;

	void set_stick_texture(Ref<Texture> &p_stick);
	Ref<Texture> get_stick_texture() const;

	void set_stick_scale(const Vector2& p_scale);
	Vector2 get_stick_scale() const;

	void set_show_mode(const ShowMode p_show_mode);
	ShowMode get_show_mode() const;

	void set_normal_moved_to_touch_pos(const bool p_move_normal_to_touch_pos);
	const bool is_normal_moved_to_touch_pos() const;

	void set_stick_confined_inside(const bool p_confined_inside);
	bool is_stick_confined_inside() const;

	TouchScreenJoystick();
	~TouchScreenJoystick();
private:
	void _input(Ref<InputEvent> &p_event);

	bool _update_direction_with_point(Point2& p_point);
	bool _update_center_with_point(Point2& p_point, const bool is_point_center);

	void _update_cache();
	void _update_texture_cache(Data::TextureData& p_data);
};

VARIANT_ENUM_CAST(TouchScreenJoystick::ShowMode);
#endif
