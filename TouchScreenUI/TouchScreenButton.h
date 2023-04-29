#ifndef TOUCH_SCREEN_BUTTON
#define TOUCH_SCREEN_BUTTON

#include "core/reference.h"
#include "iControl.h"
#include "scene/resources/circle_shape_2d.h"

class TouchScreenButton : public iControl {
	GDCLASS(TouchScreenButton, iControl);

	StringName action;
	real_t radius;
	//real_t accum_t; //should I keep this
	Ref<Texture> normal;
	Ref<Texture> pressed;

	bool _propagate_on_unpause = false;
	bool signal_only_when_released_inside = true;
	//bool isAccumulate = false;
	bool isHeld = false;
protected:
	void _notifications(int p_what);
	static void _bind_methods();
public:
	void set_action(const StringName& p_name);
	const StringName get_action() const;

	void set_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_texture() const;

	void set_pressed_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_pressed_texture() const;

	void set_radius(const real_t p_radius);
	const real_t get_radius() const;

	bool toggle_signal_release_inside(const bool p_bool);
	const bool is_signal_release_inside() const;

	//void toggle_accumulate_time(const bool p_accumulate);
	//const bool is_accumulate() const;

	//const real_t get_held_delta_time() const;
	const bool is_held() const;
private:
	virtual void _input(const Ref<InputEvent>& p_event);
	void _press(const int p_index);
	void _release(const bool p_exiting_tree = false);
};
#endif
