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
	real_t deadzone_extent;
	real_t cardinal_direction_span;

	bool _propagate_on_unpause = false;
	bool update_cache = false;

protected:
	virtual const bool _set_deadzone_extent(real_t p_extent);
	virtual const bool _set_cardinal_direction_span(real_t p_span);

	void _set_direction(Direction p_direction);
	//get_direction is a public function

	void _update_cache_dirty();
	bool is_update_cache();

	void _notification(int p_what);
	static void _bind_methods();

	void _direction_changed();
	void _release();

public:
	void set_centered(bool p_centered);
	bool is_centered() const;

	void set_center_offset(Point2 p_offset);
	Point2 get_center_offset() const;
	
	void set_deadzone_extent(real_t p_extent);
	real_t get_deadzone_extent() const;

	void set_cardinal_direction_span(real_t p_span);
	real_t get_cardinal_direction_span() const;

	Direction get_direction() const;

	TouchScreenPad(real_t p_extent, real_t p_span);
};

VARIANT_ENUM_CAST(TouchScreenPad::Direction);
#endif
