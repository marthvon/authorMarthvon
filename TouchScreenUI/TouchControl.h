#ifndef TOUCH_CONTROL
#define TOUCH_CONTROL

#include "scene/gui/control.h"

class TouchControl : public Control {
	GDCLASS(TouchControl, Control);
private:
	int finger_pressed = -1;
	bool passby_press = false;
protected:
	void _set_finger_index(const int p_finger_pressed);

	static void _bind_methods();
public:
	int get_finger_index() const;

	const bool is_passby_press() const;
	void set_passby_press(const bool p_passby_press);

	/**
		Don't use the following functions of Control
		MouseFilter
		CursorShape
		Tooltip
		FocusMode
	*/
};

#endif
