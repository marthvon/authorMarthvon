#ifndef TOUCH_SCREEN_DPAD
#define TOUCH_SCREEN_DPAD

#include "TouchScreenPad.h"
#include "core/object/ref_counted.h"
#include "scene/resources/texture.h"
#include "scene/resources/convex_polygon_shape_2d.h"

class TouchScreenDPad : public TouchScreenPad {
	GDCLASS(TouchScreenDPad, TouchScreenPad);

private:
	Ref<Texture2D> texture;
	Point2 scale_to_rect = Point2(1, 1);

	Rect2 _position_rect = Rect2();
#ifdef TOOLS_ENABLED
	Ref<ConvexPolygonShape2D> _shape_points; // put in a struct then ifdef with tools enabled
#endif
protected:
	const bool _set_deadzone_extent(real_t p_extent) override; //an Octagon
	const bool _set_cardinal_direction_span(real_t p_span) override; //a width for rect
	virtual Size2 get_minimum_size() const override;

	void _notification(int p_what);
	static void _bind_methods();

public:
	Ref<Texture2D> get_texture() const;
	void set_texture(const Ref<Texture2D> p_texture);

	Point2 get_scale_to_rect() const;
	void set_scale_to_rect(Point2 p_scale);

	TouchScreenDPad();
private:
	virtual void input(const Ref<InputEvent>& p_event) override;
	void _update_direction_with_point(Point2 p_point);

	void _update_cache();
#ifdef TOOLS_ENABLED
	void _update_shape_points(); //put in a struct const RID& p_rid_to, const real_t p_extent, const real_t p_span
	void _draw_shape();
#endif
};

#endif
