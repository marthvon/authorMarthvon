#ifndef ICONTROL
#define ICONTROL

#include "core/math/transform_2d.h"
#include "core/template/rid.h"
#include "scene/main/canvas_item.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/resources/theme.h"

class Viewport;

class iControl : public CanvasItem {
	GDCLASS(iControl, CanvasItem);
	OBJ_CATEGORY("iGUI Nodes");

public:
	enum Anchor {

		ANCHOR_BEGIN = 0,
		ANCHOR_END = 1
	};

	enum GrowDirection {
		GROW_DIRECTION_BEGIN,
		GROW_DIRECTION_END,
		GROW_DIRECTION_BOTH
	};

	enum SizeFlags {

		SIZE_FILL = 1,
		SIZE_EXPAND = 2,
		SIZE_EXPAND_FILL = SIZE_EXPAND | SIZE_FILL,
		SIZE_SHRINK_CENTER = 4, //ignored by expand or fill
		SIZE_SHRINK_END = 8, //ignored by expand or fill

	};

	enum LayoutPreset {
		PRESET_TOP_LEFT,
		PRESET_TOP_RIGHT,
		PRESET_BOTTOM_LEFT,
		PRESET_BOTTOM_RIGHT,
		PRESET_CENTER_LEFT,
		PRESET_CENTER_TOP,
		PRESET_CENTER_RIGHT,
		PRESET_CENTER_BOTTOM,
		PRESET_CENTER,
		PRESET_LEFT_WIDE,
		PRESET_TOP_WIDE,
		PRESET_RIGHT_WIDE,
		PRESET_BOTTOM_WIDE,
		PRESET_VCENTER_WIDE,
		PRESET_HCENTER_WIDE,
		PRESET_WIDE
	};

	enum LayoutPresetMode {
		PRESET_MODE_MINSIZE,
		PRESET_MODE_KEEP_WIDTH,
		PRESET_MODE_KEEP_HEIGHT,
		PRESET_MODE_KEEP_SIZE
	};

private:
	struct CComparator {

		bool operator()(const iControl *p_a, const iControl *p_b) const {
			if (p_a->get_canvas_layer() == p_b->get_canvas_layer())
				return p_b->is_greater_than(p_a);

			return p_a->get_canvas_layer() < p_b->get_canvas_layer();
		}
	};

	struct Data {

		int finger_pressed = -1;
		bool passby_press = false;

		Point2 pos_cache;
		Size2 size_cache;
		Size2 minimum_size_cache;
		bool minimum_size_valid;

		Size2 last_minimum_size;
		bool updating_last_minimum_size;

		float margin[4];
		float anchor[4];
		GrowDirection h_grow;
		GrowDirection v_grow;

		float rotation;
		Vector2 scale;
		Vector2 pivot_offset;

		bool pending_resize;

		int h_size_flags;
		int v_size_flags;
		float expand;
		Point2 custom_minimum_size;

		bool clip_contents;

		bool block_minimum_size_adjust;
		bool disable_visibility_clip;

		iControl *parent;
		Ref<Theme> theme;
		iControl *theme_owner;

		CanvasItem *parent_canvas_item;

		HashMap<StringName, Ref<Texture> > icon_override;
		HashMap<StringName, Ref<Shader> > shader_override;
		HashMap<StringName, Ref<StyleBox> > style_override;
		HashMap<StringName, Ref<Font> > font_override;
		HashMap<StringName, Color> color_override;
		HashMap<StringName, int> constant_override;

	} data;

	// used internally
	iControl *_find_iControl_at_pos(CanvasItem *p_node, const Point2 &p_pos, const Transform2D &p_xform, Transform2D &r_inv_xform);

	void _set_anchor(Margin p_margin, float p_anchor);
	void _set_position(const Point2 &p_point);
	void _set_global_position(const Point2 &p_point);
	void _set_size(const Size2 &p_size);

	void _propagate_theme_changed(CanvasItem *p_at, iControl *p_owner, bool p_assign = true);
	void _theme_changed();

	void _change_notify_margins();
	void _update_minimum_size();

	void _update_scroll();
	void _resize(const Size2 &p_size);

	void _compute_margins(Rect2 p_rect, const float p_anchors[4], float (&r_margins)[4]);
	void _compute_anchors(Rect2 p_rect, const float p_margins[4], float (&r_anchors)[4]);

	void _size_changed();

	void _override_changed();

	void _update_canvas_item_transform();

	Transform2D _get_internal_transform() const;

	friend class Viewport;

	void _update_minimum_size_cache();

protected:
	virtual void add_child_notify(Node *p_child);
	virtual void remove_child_notify(Node *p_child);

	//virtual void _window_gui_input(InputEvent p_event);

	void _set_finger_index(const int p_finger_pressed);
	//get_finger_index is a public function

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_notification);

	static void _bind_methods();

	//bind helpers

public:
	enum {

		/*		NOTIFICATION_DRAW=30,
		NOTIFICATION_VISIBILITY_CHANGED=38*/
		NOTIFICATION_RESIZED = 40,
		NOTIFICATION_THEME_CHANGED = 45,
		NOTIFICATION_MODAL_CLOSE = 46,
		NOTIFICATION_SCROLL_BEGIN = 47,
		NOTIFICATION_SCROLL_END = 48,

	};

	/* EDITOR */
#ifdef TOOLS_ENABLED
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_position(const Point2 &p_position);
	virtual Point2 _edit_get_position() const;

	virtual void _edit_set_scale(const Size2 &p_scale);
	virtual Size2 _edit_get_scale() const;

	virtual void _edit_set_rect(const Rect2 &p_edit_rect);
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

	virtual void _edit_set_rotation(float p_rotation);
	virtual float _edit_get_rotation() const;
	virtual bool _edit_use_rotation() const;

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;

	virtual Size2 _edit_get_minimum_size() const;
#endif

	virtual Size2 get_minimum_size() const;
	virtual Size2 get_combined_minimum_size() const;
	virtual bool has_point(const Point2 &p_point) const;
	virtual bool clips_input() const;

	void set_custom_minimum_size(const Size2 &p_custom);
	Size2 get_custom_minimum_size() const;

	iControl *get_parent_iControl() const;

	/* POSITIONING */

	void set_anchors_preset(LayoutPreset p_preset, bool p_keep_margins = true);
	void set_margins_preset(LayoutPreset p_preset, LayoutPresetMode p_resize_mode = PRESET_MODE_MINSIZE, int p_margin = 0);
	void set_anchors_and_margins_preset(LayoutPreset p_preset, LayoutPresetMode p_resize_mode = PRESET_MODE_MINSIZE, int p_margin = 0);

	void set_anchor(Margin p_margin, float p_anchor, bool p_keep_margin = true, bool p_push_opposite_anchor = true);
	float get_anchor(Margin p_margin) const;

	void set_margin(Margin p_margin, float p_value);
	float get_margin(Margin p_margin) const;

	void set_anchor_and_margin(Margin p_margin, float p_anchor, float p_pos, bool p_push_opposite_anchor = true);

	void set_begin(const Point2 &p_point); // helper
	void set_end(const Point2 &p_point); // helper

	Point2 get_begin() const;
	Point2 get_end() const;

	void set_position(const Point2 &p_point, bool p_keep_margins = false);
	void set_global_position(const Point2 &p_point, bool p_keep_margins = false);
	Point2 get_position() const;
	Point2 get_global_position() const;

	void set_size(const Size2 &p_size, bool p_keep_margins = false);
	Size2 get_size() const;

	Rect2 get_rect() const;
	Rect2 get_global_rect() const;
	Rect2 get_window_rect() const; ///< use with care, as it blocks waiting for the visual server
	Rect2 get_anchorable_rect() const;

	void set_rotation(float p_radians);
	void set_rotation_degrees(float p_degrees);
	float get_rotation() const;
	float get_rotation_degrees() const;

	void set_h_grow_direction(GrowDirection p_direction);
	GrowDirection get_h_grow_direction() const;

	void set_v_grow_direction(GrowDirection p_direction);
	GrowDirection get_v_grow_direction() const;

	void set_pivot_offset(const Vector2 &p_pivot);
	Vector2 get_pivot_offset() const;

	void set_scale(const Vector2 &p_scale);
	Vector2 get_scale() const;

	void set_theme(const Ref<Theme> &p_theme);
	Ref<Theme> get_theme() const;

	void set_h_size_flags(int p_flags);
	int get_h_size_flags() const;

	void set_v_size_flags(int p_flags);
	int get_v_size_flags() const;

	void set_stretch_ratio(float p_ratio);
	float get_stretch_ratio() const;

	void minimum_size_changed();

	int get_finger_index() const;

	const bool is_passby_press() const;
	void set_passby_press(const bool p_passby_press);

	/* SKINNING */

	void add_icon_override(const StringName &p_name, const Ref<Texture> &p_icon);
	void add_shader_override(const StringName &p_name, const Ref<Shader> &p_shader);
	void add_style_override(const StringName &p_name, const Ref<StyleBox> &p_style);
	void add_font_override(const StringName &p_name, const Ref<Font> &p_font);
	void add_color_override(const StringName &p_name, const Color &p_color);
	void add_constant_override(const StringName &p_name, int p_constant);

	Ref<Texture> get_icon(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	Ref<Shader> get_shader(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	Ref<StyleBox> get_stylebox(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	Ref<Font> get_font(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	Color get_color(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	int get_constant(const StringName &p_name, const StringName &p_node_type = StringName()) const;

	bool has_icon_override(const StringName &p_name) const;
	bool has_shader_override(const StringName &p_name) const;
	bool has_stylebox_override(const StringName &p_name) const;
	bool has_font_override(const StringName &p_name) const;
	bool has_color_override(const StringName &p_name) const;
	bool has_constant_override(const StringName &p_name) const;

	bool has_icon(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	bool has_shader(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	bool has_stylebox(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	bool has_font(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	bool has_color(const StringName &p_name, const StringName &p_node_type = StringName()) const;
	bool has_constant(const StringName &p_name, const StringName &p_node_type = StringName()) const;

	virtual Transform2D get_transform() const;

	Size2 get_parent_area_size() const;
	Rect2 get_parent_anchorable_rect() const;

	void set_clip_contents(bool p_clip);
	bool is_clipping_contents();

	void set_block_minimum_size_adjust(bool p_block);
	bool is_minimum_size_adjust_blocked() const;

	void set_disable_visibility_clip(bool p_ignore);
	bool is_visibility_clip_disabled() const;

	virtual void get_argument_options(const StringName &p_function, int p_idx, List<String> *r_options) const;

	iControl() {}
};
#endif
