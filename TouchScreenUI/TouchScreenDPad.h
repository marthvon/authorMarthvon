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
	float scale_to_rect = 1; // if 0 then take texture as size (texture is always squared)

	Rect2 _position_rect = Rect2();
	Ref<ConvexPolygonShape2D> _shape_points; // put in a struct then ifdef with tools enabled
protected:
	bool _set_neutral_extent(real_t p_extent) override; //an Octagon
	bool _set_single_direction_span(real_t p_span) override; //a width for rect
	virtual Size2 get_minimum_size() const override;

	void _notification(int p_what);
	static void _bind_methods();

public:
	Ref<Texture2D> get_texture() const;
	void set_texture(Ref<Texture2D> &p_texture);

	const float get_scale_to_rect() const;
	void set_scale_to_rect(const float p_scale);

	TouchScreenDPad();
private:
	void _input(const Ref<InputEvent>& p_event);
	void _update_direction_with_point(Point2& p_point);

	void _update_cache();

	void _update_shape_points(const float &size); //put in a struct const RID& p_rid_to, const real_t p_extent, const real_t p_span
	void _draw_shape();
};

#endif
