#ifndef TOUCH_SCREEN_PAD
#define TOUCH_SCREEN_PAD

#include "TouchControl.h"

class TouchScreenPad : public TouchControl {
	GDCLASS(TouchScreenPad, TouchControl);

public:
	enum Direction {
		DIR_NEUTRAL = 0,
		DIR_LEFT = 0b0001, // 1
		DIR_RIGHT = 0b0010, // 2

		DIR_DOWN = 0b0100, // 4
		DIR_DOWN_LEFT = DIR_DOWN | DIR_LEFT, // 5
		DIR_DOWN_RIGHT = DIR_DOWN | DIR_RIGHT, // 6

		DIR_UP = 0b1000, // 8
		DIR_UP_LEFT = DIR_UP | DIR_LEFT, // 9
		DIR_UP_RIGHT = DIR_UP | DIR_RIGHT // 10
	};

private:
	Direction direction = DIR_NEUTRAL;
	bool centered = true;
	Point2 offset_center = Point2(0,0); //Doesn't affect texture
	float neutral_extent;
	float single_direction_span;

	Point2 _center_point = Point2();
	bool _propagate_on_unpause = false;
	bool update_pending = false;

protected:
	virtual bool _set_neutral_extent(const real_t p_extent);
	virtual bool _set_single_direction_span(const real_t p_span);

	void _set_center_point(const Point2 p_center_point);
	const Point2 _get_center_point() const;

	void _set_direction(Direction p_direction);
	//get_direction is a public function

	void _update_cache_dirty();
	bool is_update_pending();

	void _notification(int p_what);
	static void _bind_methods();

	void _direction_changed();
	void _release(const bool is_pause_or_exit_tree = false);

public:
	void set_centered(bool p_centered);
	bool is_centered() const;

	void set_center_offset(Point2 p_offset);
	Point2 get_center_offset() const;

	void set_neutral_extent(float p_extent);
	float get_neutral_extent() const;

	void set_single_direction_span(float p_span);
	float get_single_direction_span() const;

	Direction get_direction() const;

	TouchScreenPad(float p_extent, float p_span);
};

VARIANT_ENUM_CAST(TouchScreenPad::Direction);
#endif
