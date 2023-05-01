#ifndef TOUCH_SCREEN_BUTTON
#define TOUCH_SCREEN_BUTTON

#include "core/object/ref_counted.h"
#include "TouchControl.h"
#include "scene/resources/texture.h"
#include "scene/resources/circle_shape_2d.h"

class TouchButton : public TouchControl {
	GDCLASS(TouchButton, TouchControl);

	StringName action;
	real_t radius;
	//real_t accum_t; //should I keep this
	Ref<Texture2D> normal;
	Ref<Texture2D> pressed;

	bool _propagate_on_unpause = false;
	bool signal_only_when_released_inside = true;
	//bool isAccumulate = false;
	bool isHeld = false;
protected:
	void _notification(int p_what);
	static void _bind_methods();
public:
	void set_action(StringName p_name);
	StringName get_action() const;

	void set_texture(Ref<Texture2D> p_texture);
	Ref<Texture2D> get_texture() const;

	void set_pressed_texture(Ref<Texture2D> p_texture);
	Ref<Texture2D> get_pressed_texture() const;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void toggle_signal_release_inside(bool p_bool);
	bool is_signal_release_inside() const;

	//void toggle_accumulate_time(const bool p_accumulate);
	//const bool is_accumulate() const;

	//const real_t get_held_delta_time() const;
	bool is_held() const;
private:
	virtual void input(const Ref<InputEvent>& p_event) override;
	void _press(int p_index);
	void _release(bool p_exiting_tree = false);
};
#endif
