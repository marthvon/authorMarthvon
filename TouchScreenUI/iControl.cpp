
#include "iControl.h"

#include "core/message_queue.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "scene/gui/label.h"
#include "scene/gui/panel.h"
#include "scene/main/canvas_layer.h"
#include "scene/main/viewport.h"
#include "scene/scene_string_names.h"
#include "servers/visual_server.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_settings.h"
#include "editor/plugins/canvas_item_editor_plugin.h"
#endif

#ifdef TOOLS_ENABLED
Dictionary iControl::_edit_get_state() const {

	Dictionary s;
	s["rotation"] = get_rotation();
	s["scale"] = get_scale();
	s["pivot"] = get_pivot_offset();
	Array anchors;
	anchors.push_back(get_anchor(MARGIN_LEFT));
	anchors.push_back(get_anchor(MARGIN_TOP));
	anchors.push_back(get_anchor(MARGIN_RIGHT));
	anchors.push_back(get_anchor(MARGIN_BOTTOM));
	s["anchors"] = anchors;
	Array margins;
	margins.push_back(get_margin(MARGIN_LEFT));
	margins.push_back(get_margin(MARGIN_TOP));
	margins.push_back(get_margin(MARGIN_RIGHT));
	margins.push_back(get_margin(MARGIN_BOTTOM));
	s["margins"] = margins;
	return s;
}

void iControl::_edit_set_state(const Dictionary &p_state) {
	ERR_FAIL_COND((p_state.size() <= 0) ||
				  !p_state.has("rotation") || !p_state.has("scale") ||
				  !p_state.has("pivot") || !p_state.has("anchors") || !p_state.has("margins"));
	Dictionary state = p_state;

	set_rotation(state["rotation"]);
	set_scale(state["scale"]);
	set_pivot_offset(state["pivot"]);
	Array anchors = state["anchors"];
	data.anchor[MARGIN_LEFT] = anchors[0];
	data.anchor[MARGIN_TOP] = anchors[1];
	data.anchor[MARGIN_RIGHT] = anchors[2];
	data.anchor[MARGIN_BOTTOM] = anchors[3];
	Array margins = state["margins"];
	data.margin[MARGIN_LEFT] = margins[0];
	data.margin[MARGIN_TOP] = margins[1];
	data.margin[MARGIN_RIGHT] = margins[2];
	data.margin[MARGIN_BOTTOM] = margins[3];
	_size_changed();
	_change_notify("anchor_left");
	_change_notify("anchor_right");
	_change_notify("anchor_top");
	_change_notify("anchor_bottom");
}

void iControl::_edit_set_position(const Point2 &p_position) {
#ifdef TOOLS_ENABLED
	ERR_FAIL_COND_MSG(!Engine::get_singleton()->is_editor_hint(), "This function can only be used from editor plugins.");
	set_position(p_position, CanvasItemEditor::get_singleton()->is_anchors_mode_enabled());
#else
	// Unlikely to happen. TODO: enclose all _edit_ functions into TOOLS_ENABLED
	set_position(p_position);
#endif
};

Point2 iControl::_edit_get_position() const {
	return get_position();
};

void iControl::_edit_set_scale(const Size2 &p_scale) {
	set_scale(p_scale);
}

Size2 iControl::_edit_get_scale() const {
	return data.scale;
}

void iControl::_edit_set_rect(const Rect2 &p_edit_rect) {
#ifdef TOOLS_ENABLED
	ERR_FAIL_COND_MSG(!Engine::get_singleton()->is_editor_hint(), "This function can only be used from editor plugins.");
	set_position((get_position() + get_transform().basis_xform(p_edit_rect.position)).snapped(Vector2(1, 1)), CanvasItemEditor::get_singleton()->is_anchors_mode_enabled());
	set_size(p_edit_rect.size.snapped(Vector2(1, 1)), CanvasItemEditor::get_singleton()->is_anchors_mode_enabled());
#else
	// Unlikely to happen. TODO: enclose all _edit_ functions into TOOLS_ENABLED
	set_position((get_position() + get_transform().basis_xform(p_edit_rect.position)).snapped(Vector2(1, 1)));
	set_size(p_edit_rect.size.snapped(Vector2(1, 1)));
#endif
}

Rect2 iControl::_edit_get_rect() const {
	return Rect2(Point2(), get_size());
}

bool iControl::_edit_use_rect() const {
	return true;
}

void iControl::_edit_set_rotation(float p_rotation) {
	set_rotation(p_rotation);
}

float iControl::_edit_get_rotation() const {
	return get_rotation();
}

bool iControl::_edit_use_rotation() const {
	return true;
}

void iControl::_edit_set_pivot(const Point2 &p_pivot) {
	Vector2 delta_pivot = p_pivot - get_pivot_offset();
	Vector2 move = Vector2((cos(data.rotation) - 1.0) * delta_pivot.x - sin(data.rotation) * delta_pivot.y, sin(data.rotation) * delta_pivot.x + (cos(data.rotation) - 1.0) * delta_pivot.y);
	set_position(get_position() + move);
	set_pivot_offset(p_pivot);
}

Point2 iControl::_edit_get_pivot() const {
	return get_pivot_offset();
}

bool iControl::_edit_use_pivot() const {
	return true;
}

Size2 iControl::_edit_get_minimum_size() const {
	return get_combined_minimum_size();
}
#endif

void iControl::set_custom_minimum_size(const Size2 &p_custom) {

	if (p_custom == data.custom_minimum_size)
		return;
	data.custom_minimum_size = p_custom;
	minimum_size_changed();
}

Size2 iControl::get_custom_minimum_size() const {

	return data.custom_minimum_size;
}

void iControl::_update_minimum_size_cache() {

	Size2 minsize = get_minimum_size();
	minsize.x = MAX(minsize.x, data.custom_minimum_size.x);
	minsize.y = MAX(minsize.y, data.custom_minimum_size.y);

	bool size_changed = false;
	if (data.minimum_size_cache != minsize)
		size_changed = true;

	data.minimum_size_cache = minsize;
	data.minimum_size_valid = true;

	if (size_changed)
		minimum_size_changed();
}

Size2 iControl::get_combined_minimum_size() const {

	if (!data.minimum_size_valid) {
		const_cast<iControl *>(this)->_update_minimum_size_cache();
	}
	return data.minimum_size_cache;
}

Transform2D iControl::_get_internal_transform() const {

	Transform2D rot_scale;
	rot_scale.set_rotation_and_scale(data.rotation, data.scale);
	Transform2D offset;
	offset.set_origin(-data.pivot_offset);

	return offset.affine_inverse() * (rot_scale * offset);
}

void iControl::_set_finger_index(const int p_finger_pressed) {
	data.finger_pressed = p_finger_pressed;
}

bool iControl::_set(const StringName &p_name, const Variant &p_value) {

	String name = p_name;
	if (!name.begins_with("custom")) {
		return false;
	}

	if (p_value.get_type() == Variant::NIL) {

		if (name.begins_with("custom_icons/")) {
			String dname = name.get_slicec('/', 1);
			if (data.icon_override.has(dname)) {
				data.icon_override[dname]->disconnect("changed", this, "_override_changed");
			}
			data.icon_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else if (name.begins_with("custom_shaders/")) {
			String dname = name.get_slicec('/', 1);
			if (data.shader_override.has(dname)) {
				data.shader_override[dname]->disconnect("changed", this, "_override_changed");
			}
			data.shader_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else if (name.begins_with("custom_styles/")) {
			String dname = name.get_slicec('/', 1);
			if (data.style_override.has(dname)) {
				data.style_override[dname]->disconnect("changed", this, "_override_changed");
			}
			data.style_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else if (name.begins_with("custom_fonts/")) {
			String dname = name.get_slicec('/', 1);
			if (data.font_override.has(dname)) {
				data.font_override[dname]->disconnect("changed", this, "_override_changed");
			}
			data.font_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else if (name.begins_with("custom_colors/")) {
			String dname = name.get_slicec('/', 1);
			data.color_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else if (name.begins_with("custom_constants/")) {
			String dname = name.get_slicec('/', 1);
			data.constant_override.erase(dname);
			notification(NOTIFICATION_THEME_CHANGED);
		} else
			return false;

	} else {
		if (name.begins_with("custom_icons/")) {
			String dname = name.get_slicec('/', 1);
			add_icon_override(dname, p_value);
		} else if (name.begins_with("custom_shaders/")) {
			String dname = name.get_slicec('/', 1);
			add_shader_override(dname, p_value);
		} else if (name.begins_with("custom_styles/")) {
			String dname = name.get_slicec('/', 1);
			add_style_override(dname, p_value);
		} else if (name.begins_with("custom_fonts/")) {
			String dname = name.get_slicec('/', 1);
			add_font_override(dname, p_value);
		} else if (name.begins_with("custom_colors/")) {
			String dname = name.get_slicec('/', 1);
			add_color_override(dname, p_value);
		} else if (name.begins_with("custom_constants/")) {
			String dname = name.get_slicec('/', 1);
			add_constant_override(dname, p_value);
		} else
			return false;
	}
	return true;
}

void iControl::_update_minimum_size() {

	if (!is_inside_tree())
		return;

	Size2 minsize = get_combined_minimum_size();
	if (minsize.x > data.size_cache.x ||
			minsize.y > data.size_cache.y) {
		_size_changed();
	}

	data.updating_last_minimum_size = false;

	if (minsize != data.last_minimum_size) {
		data.last_minimum_size = minsize;
		emit_signal(SceneStringNames::get_singleton()->minimum_size_changed);
	}
}

bool iControl::_get(const StringName &p_name, Variant &r_ret) const {

	String sname = p_name;

	if (!sname.begins_with("custom")) {
		return false;
	}

	if (sname.begins_with("custom_icons/")) {
		String name = sname.get_slicec('/', 1);

		r_ret = data.icon_override.has(name) ? Variant(data.icon_override[name]) : Variant();
	} else if (sname.begins_with("custom_shaders/")) {
		String name = sname.get_slicec('/', 1);

		r_ret = data.shader_override.has(name) ? Variant(data.shader_override[name]) : Variant();
	} else if (sname.begins_with("custom_styles/")) {
		String name = sname.get_slicec('/', 1);

		r_ret = data.style_override.has(name) ? Variant(data.style_override[name]) : Variant();
	} else if (sname.begins_with("custom_fonts/")) {
		String name = sname.get_slicec('/', 1);

		r_ret = data.font_override.has(name) ? Variant(data.font_override[name]) : Variant();
	} else if (sname.begins_with("custom_colors/")) {
		String name = sname.get_slicec('/', 1);
		r_ret = data.color_override.has(name) ? Variant(data.color_override[name]) : Variant();
	} else if (sname.begins_with("custom_constants/")) {
		String name = sname.get_slicec('/', 1);

		r_ret = data.constant_override.has(name) ? Variant(data.constant_override[name]) : Variant();
	} else
		return false;

	return true;
}
void iControl::_get_property_list(List<PropertyInfo> *p_list) const {

	Ref<Theme> theme = Theme::get_default();
	/* Using the default theme since the properties below are meant for editor only
	if (data.theme.is_valid()) {

		theme = data.theme;
	} else {
		theme = Theme::get_default();

	}*/

	{
		List<StringName> names;
		theme->get_icon_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.icon_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::OBJECT, "custom_icons/" + E->get(), PROPERTY_HINT_RESOURCE_TYPE, "Texture", hint));
		}
	}
	{
		List<StringName> names;
		theme->get_shader_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.shader_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::OBJECT, "custom_shaders/" + E->get(), PROPERTY_HINT_RESOURCE_TYPE, "Shader,VisualShader", hint));
		}
	}
	{
		List<StringName> names;
		theme->get_stylebox_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.style_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::OBJECT, "custom_styles/" + E->get(), PROPERTY_HINT_RESOURCE_TYPE, "StyleBox", hint));
		}
	}
	{
		List<StringName> names;
		theme->get_font_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.font_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::OBJECT, "custom_fonts/" + E->get(), PROPERTY_HINT_RESOURCE_TYPE, "Font", hint));
		}
	}
	{
		List<StringName> names;
		theme->get_color_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.color_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::COLOR, "custom_colors/" + E->get(), PROPERTY_HINT_NONE, "", hint));
		}
	}
	{
		List<StringName> names;
		theme->get_constant_list(get_class_name(), &names);
		for (List<StringName>::Element *E = names.front(); E; E = E->next()) {

			uint32_t hint = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE;
			if (data.constant_override.has(E->get()))
				hint |= PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_CHECKED;

			p_list->push_back(PropertyInfo(Variant::INT, "custom_constants/" + E->get(), PROPERTY_HINT_RANGE, "-16384,16384", hint));
		}
	}
}

iControl *iControl::get_parent_iControl() const {

	return data.parent;
}

void iControl::_resize(const Size2 &p_size) {

	_size_changed();
}

//moved theme configuration here, so iControls can set up even if still not inside active scene

void iControl::add_child_notify(Node *p_child) {

	iControl *child_c = Object::cast_to<iControl>(p_child);
	if (!child_c)
		return;

	if (child_c->data.theme.is_null() && data.theme_owner) {
		_propagate_theme_changed(child_c, data.theme_owner); //need to propagate here, since many iControls may require setting up stuff
	}
}

void iControl::remove_child_notify(Node *p_child) {

	iControl *child_c = Object::cast_to<iControl>(p_child);
	if (!child_c)
		return;

	if (child_c->data.theme_owner && child_c->data.theme.is_null()) {
		_propagate_theme_changed(child_c, NULL);
	}
}

void iControl::_update_canvas_item_transform() {

	Transform2D xform = _get_internal_transform();
	xform[2] += get_position();

	// We use a little workaround to avoid flickering when moving the pivot with _edit_set_pivot()
	if (is_inside_tree() && Math::abs(Math::sin(data.rotation * 4.0f)) < 0.00001f && get_viewport()->is_snap_controls_to_pixels_enabled()) {
		xform[2] = xform[2].round();
	}

	VisualServer::get_singleton()->canvas_item_set_transform(get_canvas_item(), xform);
}

void iControl::_notification(int p_notification) {

	switch (p_notification) {
		case NOTIFICATION_ENTER_TREE: {
			if (Engine::get_singleton()->is_editor_hint() && !OS::get_singleton()->has_touchscreen_ui_hint())
				return;
			update();

			if (Engine::get_singleton()->is_editor_hint())
				set_process_input(is_visible_in_tree());
		} break;
		case NOTIFICATION_POST_ENTER_TREE: {
			data.minimum_size_valid = false;
			_size_changed();
		} break;
		case NOTIFICATION_ENTER_CANVAS: {
			data.parent = Object::cast_to<iControl>(get_parent());

			if (is_set_as_toplevel() && data.theme.is_null() && data.parent && data.parent->data.theme_owner) {
				data.theme_owner = data.parent->data.theme_owner;
				notification(NOTIFICATION_THEME_CHANGED);
			} else {
				Node *parent = this; //meh
				iControl *parent_iControl = NULL;
				bool subwindow = false;

				while (parent) {
					parent = parent->get_parent();
					if (!parent)
						break;
					CanvasItem *ci = Object::cast_to<CanvasItem>(parent);
					if (ci && ci->is_set_as_toplevel()) {
						subwindow = true;
						break;
					}
					parent_iControl = Object::cast_to<iControl>(parent);
					if (parent_iControl) 
						break;
					else if (!ci) 
						break;
				}

				if (parent_iControl) {
					//do nothing, has a parent iControl
					if (data.theme.is_null() && parent_iControl->data.theme_owner) {
						data.theme_owner = parent_iControl->data.theme_owner;
						notification(NOTIFICATION_THEME_CHANGED);
					}
				}
				} else if (subwindow) {
					//is a subwindow (process input before other controls for that canvas)
					data.SI = get_viewport()->_gui_add_subwindow_control(this);
				} else {
					//is a regular root control
					Viewport *viewport = get_viewport();
					ERR_FAIL_COND(!viewport);
				}
				data.parent_canvas_item = get_parent_item();

				if (data.parent_canvas_item) {

					data.parent_canvas_item->connect("item_rect_changed", this, "_size_changed");
				} else {
					//connect viewport
					get_viewport()->connect("size_changed", this, "_size_changed");
				}
			}

			/*
			if (data.theme.is_null() && data.parent && data.parent->data.theme_owner) {
				data.theme_owner=data.parent->data.theme_owner;
				notification(NOTIFICATION_THEME_CHANGED);
			}
			*/
		} break;
		case NOTIFICATION_EXIT_CANVAS: {
			if (data.parent_canvas_item) {

				data.parent_canvas_item->disconnect("item_rect_changed", this, "_size_changed");
				data.parent_canvas_item = NULL;
			} else if (!is_set_as_toplevel()) {
				//disconnect viewport
				get_viewport()->disconnect("size_changed", this, "_size_changed");
			}

			data.parent = NULL;
			data.parent_canvas_item = NULL;
			/*
			if (data.theme_owner && data.theme.is_null()) {
				data.theme_owner=NULL;
				notification(NOTIFICATION_THEME_CHANGED);
			}
			*/
		} break;
		case NOTIFICATION_MOVED_IN_PARENT: {
			// some parents need to know the order of the childrens to draw (like TabContainer)
			// update if necessary
			if (data.parent)
				data.parent->update();
			update();
		} break;
		case NOTIFICATION_RESIZED: {
			emit_signal(SceneStringNames::get_singleton()->resized);
		} break;
		case NOTIFICATION_DRAW: {
			_update_canvas_item_transform();
			VisualServer::get_singleton()->canvas_item_set_custom_rect(get_canvas_item(), !data.disable_visibility_clip, Rect2(Point2(), get_size()));
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), data.clip_contents);
			//emit_signal(SceneStringNames::get_singleton()->draw);
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			minimum_size_changed();
			update();
		} break;
		case NOTIFICATION_MODAL_CLOSE: {
			emit_signal("modal_closed");
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (is_visible_in_tree()) {
				data.minimum_size_valid = false;
				_size_changed();
			}

		} break;
	}
}

bool iControl::clips_input() const {

	if (get_script_instance()) {
		return get_script_instance()->call(SceneStringNames::get_singleton()->_clips_input);
	}
	return false;
}
bool iControl::has_point(const Point2 &p_point) const {
	if (get_script_instance()) {
		Variant v = p_point;
		const Variant *p = &v;
		Variant::CallError ce;
		Variant ret = get_script_instance()->call(SceneStringNames::get_singleton()->has_point, &p, 1, ce);
		if (ce.error == Variant::CallError::CALL_OK) {
			return ret;
		}
	}
	/*if (has_stylebox("mask")) {
		Ref<StyleBox> mask = get_stylebox("mask");
		return mask->test_mask(p_point,Rect2(Point2(),get_size()));
	}*/
	return Rect2(Point2(), get_size()).has_point(p_point);
}

Size2 iControl::get_minimum_size() const {

	ScriptInstance *si = const_cast<iControl *>(this)->get_script_instance();
	if (si) {

		Variant::CallError ce;
		Variant s = si->call(SceneStringNames::get_singleton()->_get_minimum_size, NULL, 0, ce);
		if (ce.error == Variant::CallError::CALL_OK)
			return s;
	}
	return Size2();
}

Ref<Texture> iControl::get_icon(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {

		const Ref<Texture> *tex = data.icon_override.getptr(p_name);
		if (tex)
			return *tex;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_icon(p_name, class_name)) {
				return theme_owner->data.theme->get_icon(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_icon(p_name, type)) {
			return Theme::get_project_default()->get_icon(p_name, type);
		}
	}

	return Theme::get_default()->get_icon(p_name, type);
}

Ref<Shader> iControl::get_shader(const StringName &p_name, const StringName &p_node_type) const {
	if (p_node_type == StringName() || p_node_type == get_class_name()) {

		const Ref<Shader> *sdr = data.shader_override.getptr(p_name);
		if (sdr)
			return *sdr;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_shader(p_name, class_name)) {
				return theme_owner->data.theme->get_shader(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_shader(p_name, type)) {
			return Theme::get_project_default()->get_shader(p_name, type);
		}
	}

	return Theme::get_default()->get_shader(p_name, type);
}

Ref<StyleBox> iControl::get_stylebox(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		const Ref<StyleBox> *style = data.style_override.getptr(p_name);
		if (style)
			return *style;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	StringName class_name = type;

	while (theme_owner) {

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_stylebox(p_name, class_name)) {
				return theme_owner->data.theme->get_stylebox(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		class_name = type;

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	while (class_name != StringName()) {
		if (Theme::get_project_default().is_valid() && Theme::get_project_default()->has_stylebox(p_name, type))
			return Theme::get_project_default()->get_stylebox(p_name, type);

		if (Theme::get_default()->has_stylebox(p_name, class_name))
			return Theme::get_default()->get_stylebox(p_name, class_name);

		class_name = ClassDB::get_parent_class_nocheck(class_name);
	}
	return Theme::get_default()->get_stylebox(p_name, type);
}
Ref<Font> iControl::get_font(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		const Ref<Font> *font = data.font_override.getptr(p_name);
		if (font)
			return *font;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_font(p_name, class_name)) {
				return theme_owner->data.theme->get_font(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		if (theme_owner->data.theme->get_default_theme_font().is_valid())
			return theme_owner->data.theme->get_default_theme_font();
		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	return Theme::get_default()->get_font(p_name, type);
}
Color iControl::get_color(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		const Color *color = data.color_override.getptr(p_name);
		if (color)
			return *color;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();
	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_color(p_name, class_name)) {
				return theme_owner->data.theme->get_color(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_color(p_name, type)) {
			return Theme::get_project_default()->get_color(p_name, type);
		}
	}
	return Theme::get_default()->get_color(p_name, type);
}

int iControl::get_constant(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		const int *constant = data.constant_override.getptr(p_name);
		if (constant)
			return *constant;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();
	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_constant(p_name, class_name)) {
				return theme_owner->data.theme->get_constant(p_name, class_name);
			}

			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_constant(p_name, type)) {
			return Theme::get_project_default()->get_constant(p_name, type);
		}
	}
	return Theme::get_default()->get_constant(p_name, type);
}

bool iControl::has_icon_override(const StringName &p_name) const {

	const Ref<Texture> *tex = data.icon_override.getptr(p_name);
	return tex != NULL;
}

bool iControl::has_shader_override(const StringName &p_name) const {

	const Ref<Shader> *sdr = data.shader_override.getptr(p_name);
	return sdr != NULL;
}

bool iControl::has_stylebox_override(const StringName &p_name) const {

	const Ref<StyleBox> *style = data.style_override.getptr(p_name);
	return style != NULL;
}

bool iControl::has_font_override(const StringName &p_name) const {

	const Ref<Font> *font = data.font_override.getptr(p_name);
	return font != NULL;
}

bool iControl::has_color_override(const StringName &p_name) const {

	const Color *color = data.color_override.getptr(p_name);
	return color != NULL;
}

bool iControl::has_constant_override(const StringName &p_name) const {

	const int *constant = data.constant_override.getptr(p_name);
	return constant != NULL;
}

bool iControl::has_icon(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_icon_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_icon(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_color(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_icon(p_name, type);
}

bool iControl::has_shader(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_shader_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_shader(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_shader(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_shader(p_name, type);
}
bool iControl::has_stylebox(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_stylebox_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_stylebox(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_stylebox(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_stylebox(p_name, type);
}
bool iControl::has_font(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_font_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_font(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_font(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_font(p_name, type);
}

bool iControl::has_color(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_color_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_color(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_color(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_color(p_name, type);
}

bool iControl::has_constant(const StringName &p_name, const StringName &p_node_type) const {

	if (p_node_type == StringName() || p_node_type == get_class_name()) {
		if (has_constant_override(p_name))
			return true;
	}

	StringName type = p_node_type ? p_node_type : get_class_name();

	// try with custom themes
	iControl *theme_owner = data.theme_owner;

	while (theme_owner) {

		StringName class_name = type;

		while (class_name != StringName()) {
			if (theme_owner->data.theme->has_constant(p_name, class_name)) {
				return true;
			}
			class_name = ClassDB::get_parent_class_nocheck(class_name);
		}

		iControl *parent = Object::cast_to<iControl>(theme_owner->get_parent());

		if (parent)
			theme_owner = parent->data.theme_owner;
		else
			theme_owner = NULL;
	}

	if (Theme::get_project_default().is_valid()) {
		if (Theme::get_project_default()->has_constant(p_name, type)) {
			return true;
		}
	}
	return Theme::get_default()->has_constant(p_name, type);
}

Rect2 iControl::get_parent_anchorable_rect() const {
	if (!is_inside_tree())
		return Rect2();

	Rect2 parent_rect;
	if (data.parent_canvas_item) {
		parent_rect = data.parent_canvas_item->get_anchorable_rect();
	} else {
		parent_rect = get_viewport()->get_visible_rect();
	}

	return parent_rect;
}

Size2 iControl::get_parent_area_size() const {

	return get_parent_anchorable_rect().size;
}

void iControl::_size_changed() {

	Rect2 parent_rect = get_parent_anchorable_rect();

	float margin_pos[4];

	for (int i = 0; i < 4; i++) {

		float area = parent_rect.size[i & 1];
		margin_pos[i] = data.margin[i] + (data.anchor[i] * area);
	}

	Point2 new_pos_cache = Point2(margin_pos[0], margin_pos[1]);
	Size2 new_size_cache = Point2(margin_pos[2], margin_pos[3]) - new_pos_cache;

	Size2 minimum_size = get_combined_minimum_size();

	if (minimum_size.width > new_size_cache.width) {
		if (data.h_grow == GROW_DIRECTION_BEGIN) {
			new_pos_cache.x += new_size_cache.width - minimum_size.width;
		} else if (data.h_grow == GROW_DIRECTION_BOTH) {
			new_pos_cache.x += 0.5 * (new_size_cache.width - minimum_size.width);
		}

		new_size_cache.width = minimum_size.width;
	}

	if (minimum_size.height > new_size_cache.height) {
		if (data.v_grow == GROW_DIRECTION_BEGIN) {
			new_pos_cache.y += new_size_cache.height - minimum_size.height;
		} else if (data.v_grow == GROW_DIRECTION_BOTH) {
			new_pos_cache.y += 0.5 * (new_size_cache.height - minimum_size.height);
		}

		new_size_cache.height = minimum_size.height;
	}

	bool pos_changed = new_pos_cache != data.pos_cache;
	bool size_changed = new_size_cache != data.size_cache;

	data.pos_cache = new_pos_cache;
	data.size_cache = new_size_cache;

	if (is_inside_tree()) {
		if (size_changed) {
			notification(NOTIFICATION_RESIZED);
		}
		if (pos_changed || size_changed) {
			item_rect_changed(size_changed);
			_change_notify_margins();
			_notify_transform();
		}

		if (pos_changed && !size_changed) {
			_update_canvas_item_transform(); //move because it won't be updated
		}
	}
}

void iControl::set_anchor(Margin p_margin, float p_anchor, bool p_keep_margin, bool p_push_opposite_anchor) {

	ERR_FAIL_INDEX((int)p_margin, 4);

	Rect2 parent_rect = get_parent_anchorable_rect();
	float parent_range = (p_margin == MARGIN_LEFT || p_margin == MARGIN_RIGHT) ? parent_rect.size.x : parent_rect.size.y;
	float previous_margin_pos = data.margin[p_margin] + data.anchor[p_margin] * parent_range;
	float previous_opposite_margin_pos = data.margin[(p_margin + 2) % 4] + data.anchor[(p_margin + 2) % 4] * parent_range;

	data.anchor[p_margin] = p_anchor;

	if (((p_margin == MARGIN_LEFT || p_margin == MARGIN_TOP) && data.anchor[p_margin] > data.anchor[(p_margin + 2) % 4]) ||
			((p_margin == MARGIN_RIGHT || p_margin == MARGIN_BOTTOM) && data.anchor[p_margin] < data.anchor[(p_margin + 2) % 4])) {
		if (p_push_opposite_anchor) {
			data.anchor[(p_margin + 2) % 4] = data.anchor[p_margin];
		} else {
			data.anchor[p_margin] = data.anchor[(p_margin + 2) % 4];
		}
	}

	if (!p_keep_margin) {
		data.margin[p_margin] = previous_margin_pos - data.anchor[p_margin] * parent_range;
		if (p_push_opposite_anchor) {
			data.margin[(p_margin + 2) % 4] = previous_opposite_margin_pos - data.anchor[(p_margin + 2) % 4] * parent_range;
		}
	}
	if (is_inside_tree()) {
		_size_changed();
	}

	update();
	_change_notify("anchor_left");
	_change_notify("anchor_right");
	_change_notify("anchor_top");
	_change_notify("anchor_bottom");
}

void iControl::_set_anchor(Margin p_margin, float p_anchor) {
	set_anchor(p_margin, p_anchor);
}

void iControl::set_anchor_and_margin(Margin p_margin, float p_anchor, float p_pos, bool p_push_opposite_anchor) {

	set_anchor(p_margin, p_anchor, false, p_push_opposite_anchor);
	set_margin(p_margin, p_pos);
}

void iControl::set_anchors_preset(LayoutPreset p_preset, bool p_keep_margins) {

	ERR_FAIL_INDEX((int)p_preset, 16);

	//Left
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_BOTTOM_LEFT:
		case PRESET_CENTER_LEFT:
		case PRESET_TOP_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_LEFT_WIDE:
		case PRESET_HCENTER_WIDE:
		case PRESET_WIDE:
			set_anchor(MARGIN_LEFT, ANCHOR_BEGIN, p_keep_margins);
			break;

		case PRESET_CENTER_TOP:
		case PRESET_CENTER_BOTTOM:
		case PRESET_CENTER:
		case PRESET_VCENTER_WIDE:
			set_anchor(MARGIN_LEFT, 0.5, p_keep_margins);
			break;

		case PRESET_TOP_RIGHT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_RIGHT:
		case PRESET_RIGHT_WIDE:
			set_anchor(MARGIN_LEFT, ANCHOR_END, p_keep_margins);
			break;
	}

	// Top
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_TOP_RIGHT:
		case PRESET_CENTER_TOP:
		case PRESET_LEFT_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_TOP_WIDE:
		case PRESET_VCENTER_WIDE:
		case PRESET_WIDE:
			set_anchor(MARGIN_TOP, ANCHOR_BEGIN, p_keep_margins);
			break;

		case PRESET_CENTER_LEFT:
		case PRESET_CENTER_RIGHT:
		case PRESET_CENTER:
		case PRESET_HCENTER_WIDE:
			set_anchor(MARGIN_TOP, 0.5, p_keep_margins);
			break;

		case PRESET_BOTTOM_LEFT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_BOTTOM:
		case PRESET_BOTTOM_WIDE:
			set_anchor(MARGIN_TOP, ANCHOR_END, p_keep_margins);
			break;
	}

	// Right
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_BOTTOM_LEFT:
		case PRESET_CENTER_LEFT:
		case PRESET_LEFT_WIDE:
			set_anchor(MARGIN_RIGHT, ANCHOR_BEGIN, p_keep_margins);
			break;

		case PRESET_CENTER_TOP:
		case PRESET_CENTER_BOTTOM:
		case PRESET_CENTER:
		case PRESET_VCENTER_WIDE:
			set_anchor(MARGIN_RIGHT, 0.5, p_keep_margins);
			break;

		case PRESET_TOP_RIGHT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_RIGHT:
		case PRESET_TOP_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_HCENTER_WIDE:
		case PRESET_WIDE:
			set_anchor(MARGIN_RIGHT, ANCHOR_END, p_keep_margins);
			break;
	}

	// Bottom
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_TOP_RIGHT:
		case PRESET_CENTER_TOP:
		case PRESET_TOP_WIDE:
			set_anchor(MARGIN_BOTTOM, ANCHOR_BEGIN, p_keep_margins);
			break;

		case PRESET_CENTER_LEFT:
		case PRESET_CENTER_RIGHT:
		case PRESET_CENTER:
		case PRESET_HCENTER_WIDE:
			set_anchor(MARGIN_BOTTOM, 0.5, p_keep_margins);
			break;

		case PRESET_BOTTOM_LEFT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_BOTTOM:
		case PRESET_LEFT_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_VCENTER_WIDE:
		case PRESET_WIDE:
			set_anchor(MARGIN_BOTTOM, ANCHOR_END, p_keep_margins);
			break;
	}
}

void iControl::set_margins_preset(LayoutPreset p_preset, LayoutPresetMode p_resize_mode, int p_margin) {

	ERR_FAIL_INDEX((int)p_preset, 16);
	ERR_FAIL_INDEX((int)p_resize_mode, 4);

	// Calculate the size if the node is not resized
	Size2 min_size = get_minimum_size();
	Size2 new_size = get_size();
	if (p_resize_mode == PRESET_MODE_MINSIZE || p_resize_mode == PRESET_MODE_KEEP_HEIGHT) {
		new_size.x = min_size.x;
	}
	if (p_resize_mode == PRESET_MODE_MINSIZE || p_resize_mode == PRESET_MODE_KEEP_WIDTH) {
		new_size.y = min_size.y;
	}

	Rect2 parent_rect = get_parent_anchorable_rect();

	//Left
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_BOTTOM_LEFT:
		case PRESET_CENTER_LEFT:
		case PRESET_TOP_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_LEFT_WIDE:
		case PRESET_HCENTER_WIDE:
		case PRESET_WIDE:
			data.margin[0] = parent_rect.size.x * (0.0 - data.anchor[0]) + p_margin + parent_rect.position.x;
			break;

		case PRESET_CENTER_TOP:
		case PRESET_CENTER_BOTTOM:
		case PRESET_CENTER:
		case PRESET_VCENTER_WIDE:
			data.margin[0] = parent_rect.size.x * (0.5 - data.anchor[0]) - new_size.x / 2 + parent_rect.position.x;
			break;

		case PRESET_TOP_RIGHT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_RIGHT:
		case PRESET_RIGHT_WIDE:
			data.margin[0] = parent_rect.size.x * (1.0 - data.anchor[0]) - new_size.x - p_margin + parent_rect.position.x;
			break;
	}

	// Top
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_TOP_RIGHT:
		case PRESET_CENTER_TOP:
		case PRESET_LEFT_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_TOP_WIDE:
		case PRESET_VCENTER_WIDE:
		case PRESET_WIDE:
			data.margin[1] = parent_rect.size.y * (0.0 - data.anchor[1]) + p_margin + parent_rect.position.y;
			break;

		case PRESET_CENTER_LEFT:
		case PRESET_CENTER_RIGHT:
		case PRESET_CENTER:
		case PRESET_HCENTER_WIDE:
			data.margin[1] = parent_rect.size.y * (0.5 - data.anchor[1]) - new_size.y / 2 + parent_rect.position.y;
			break;

		case PRESET_BOTTOM_LEFT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_BOTTOM:
		case PRESET_BOTTOM_WIDE:
			data.margin[1] = parent_rect.size.y * (1.0 - data.anchor[1]) - new_size.y - p_margin + parent_rect.position.y;
			break;
	}

	// Right
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_BOTTOM_LEFT:
		case PRESET_CENTER_LEFT:
		case PRESET_LEFT_WIDE:
			data.margin[2] = parent_rect.size.x * (0.0 - data.anchor[2]) + new_size.x + p_margin + parent_rect.position.x;
			break;

		case PRESET_CENTER_TOP:
		case PRESET_CENTER_BOTTOM:
		case PRESET_CENTER:
		case PRESET_VCENTER_WIDE:
			data.margin[2] = parent_rect.size.x * (0.5 - data.anchor[2]) + new_size.x / 2 + parent_rect.position.x;
			break;

		case PRESET_TOP_RIGHT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_RIGHT:
		case PRESET_TOP_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_HCENTER_WIDE:
		case PRESET_WIDE:
			data.margin[2] = parent_rect.size.x * (1.0 - data.anchor[2]) - p_margin + parent_rect.position.x;
			break;
	}

	// Bottom
	switch (p_preset) {
		case PRESET_TOP_LEFT:
		case PRESET_TOP_RIGHT:
		case PRESET_CENTER_TOP:
		case PRESET_TOP_WIDE:
			data.margin[3] = parent_rect.size.y * (0.0 - data.anchor[3]) + new_size.y + p_margin + parent_rect.position.y;
			break;

		case PRESET_CENTER_LEFT:
		case PRESET_CENTER_RIGHT:
		case PRESET_CENTER:
		case PRESET_HCENTER_WIDE:
			data.margin[3] = parent_rect.size.y * (0.5 - data.anchor[3]) + new_size.y / 2 + parent_rect.position.y;
			break;

		case PRESET_BOTTOM_LEFT:
		case PRESET_BOTTOM_RIGHT:
		case PRESET_CENTER_BOTTOM:
		case PRESET_LEFT_WIDE:
		case PRESET_RIGHT_WIDE:
		case PRESET_BOTTOM_WIDE:
		case PRESET_VCENTER_WIDE:
		case PRESET_WIDE:
			data.margin[3] = parent_rect.size.y * (1.0 - data.anchor[3]) - p_margin + parent_rect.position.y;
			break;
	}

	_size_changed();
}

void iControl::set_anchors_and_margins_preset(LayoutPreset p_preset, LayoutPresetMode p_resize_mode, int p_margin) {
	set_anchors_preset(p_preset);
	set_margins_preset(p_preset, p_resize_mode, p_margin);
}

float iControl::get_anchor(Margin p_margin) const {

	ERR_FAIL_INDEX_V(int(p_margin), 4, 0.0);

	return data.anchor[p_margin];
}

void iControl::_change_notify_margins() {

	// this avoids sending the whole object data again on a change
	_change_notify("margin_left");
	_change_notify("margin_top");
	_change_notify("margin_right");
	_change_notify("margin_bottom");
	_change_notify("rect_position");
	_change_notify("rect_size");
}

void iControl::set_margin(Margin p_margin, float p_value) {

	ERR_FAIL_INDEX((int)p_margin, 4);

	data.margin[p_margin] = p_value;
	_size_changed();
}

void iControl::set_begin(const Size2 &p_point) {

	data.margin[0] = p_point.x;
	data.margin[1] = p_point.y;
	_size_changed();
}

void iControl::set_end(const Size2 &p_point) {

	data.margin[2] = p_point.x;
	data.margin[3] = p_point.y;
	_size_changed();
}

float iControl::get_margin(Margin p_margin) const {

	ERR_FAIL_INDEX_V((int)p_margin, 4, 0);

	return data.margin[p_margin];
}

Size2 iControl::get_begin() const {

	return Size2(data.margin[0], data.margin[1]);
}
Size2 iControl::get_end() const {

	return Size2(data.margin[2], data.margin[3]);
}

Point2 iControl::get_global_position() const {

	return get_global_transform().get_origin();
}

void iControl::_set_global_position(const Point2 &p_point) {
	set_global_position(p_point);
}

void iControl::set_global_position(const Point2 &p_point, bool p_keep_margins) {

	Transform2D inv;

	if (data.parent_canvas_item) {

		inv = data.parent_canvas_item->get_global_transform().affine_inverse();
	}

	set_position(inv.xform(p_point), p_keep_margins);
}

void iControl::_compute_anchors(Rect2 p_rect, const float p_margins[4], float (&r_anchors)[4]) {

	Size2 parent_rect_size = get_parent_anchorable_rect().size;
	ERR_FAIL_COND(parent_rect_size.x == 0.0);
	ERR_FAIL_COND(parent_rect_size.y == 0.0);

	r_anchors[0] = (p_rect.position.x - p_margins[0]) / parent_rect_size.x;
	r_anchors[1] = (p_rect.position.y - p_margins[1]) / parent_rect_size.y;
	r_anchors[2] = (p_rect.position.x + p_rect.size.x - p_margins[2]) / parent_rect_size.x;
	r_anchors[3] = (p_rect.position.y + p_rect.size.y - p_margins[3]) / parent_rect_size.y;
}

void iControl::_compute_margins(Rect2 p_rect, const float p_anchors[4], float (&r_margins)[4]) {

	Size2 parent_rect_size = get_parent_anchorable_rect().size;
	r_margins[0] = p_rect.position.x - (p_anchors[0] * parent_rect_size.x);
	r_margins[1] = p_rect.position.y - (p_anchors[1] * parent_rect_size.y);
	r_margins[2] = p_rect.position.x + p_rect.size.x - (p_anchors[2] * parent_rect_size.x);
	r_margins[3] = p_rect.position.y + p_rect.size.y - (p_anchors[3] * parent_rect_size.y);
}

void iControl::_set_position(const Size2 &p_point) {
	set_position(p_point);
}

void iControl::set_position(const Size2 &p_point, bool p_keep_margins) {
	if (p_keep_margins) {
		_compute_anchors(Rect2(p_point, data.size_cache), data.margin, data.anchor);
		_change_notify("anchor_left");
		_change_notify("anchor_right");
		_change_notify("anchor_top");
		_change_notify("anchor_bottom");
	} else {
		_compute_margins(Rect2(p_point, data.size_cache), data.anchor, data.margin);
	}
	_size_changed();
}

void iControl::_set_size(const Size2 &p_size) {
	set_size(p_size);
}

void iControl::set_size(const Size2 &p_size, bool p_keep_margins) {

	Size2 new_size = p_size;
	Size2 min = get_combined_minimum_size();
	if (new_size.x < min.x)
		new_size.x = min.x;
	if (new_size.y < min.y)
		new_size.y = min.y;

	if (p_keep_margins) {
		_compute_anchors(Rect2(data.pos_cache, new_size), data.margin, data.anchor);
		_change_notify("anchor_left");
		_change_notify("anchor_right");
		_change_notify("anchor_top");
		_change_notify("anchor_bottom");
	} else {
		_compute_margins(Rect2(data.pos_cache, new_size), data.anchor, data.margin);
	}
	_size_changed();
}

Size2 iControl::get_position() const {

	return data.pos_cache;
}

Size2 iControl::get_size() const {

	return data.size_cache;
}

Rect2 iControl::get_global_rect() const {

	return Rect2(get_global_position(), get_size());
}

Rect2 iControl::get_window_rect() const {
	ERR_FAIL_COND_V(!is_inside_tree(), Rect2());
	Rect2 gr = get_global_rect();
	gr.position += get_viewport()->get_visible_rect().position;
	return gr;
}

Rect2 iControl::get_rect() const {

	return Rect2(get_position(), get_size());
}

Rect2 iControl::get_anchorable_rect() const {

	return Rect2(Point2(), get_size());
}

void iControl::add_icon_override(const StringName &p_name, const Ref<Texture> &p_icon) {

	if (data.icon_override.has(p_name)) {
		data.icon_override[p_name]->disconnect("changed", this, "_override_changed");
	}

	// clear if "null" is passed instead of a icon
	if (p_icon.is_null()) {
		data.icon_override.erase(p_name);
	} else {
		data.icon_override[p_name] = p_icon;
		if (data.icon_override[p_name].is_valid()) {
			data.icon_override[p_name]->connect("changed", this, "_override_changed", Vector<Variant>(), CONNECT_REFERENCE_COUNTED);
		}
	}
	notification(NOTIFICATION_THEME_CHANGED);
}

void iControl::add_shader_override(const StringName &p_name, const Ref<Shader> &p_shader) {

	if (data.shader_override.has(p_name)) {
		data.shader_override[p_name]->disconnect("changed", this, "_override_changed");
	}

	// clear if "null" is passed instead of a shader
	if (p_shader.is_null()) {
		data.shader_override.erase(p_name);
	} else {
		data.shader_override[p_name] = p_shader;
		if (data.shader_override[p_name].is_valid()) {
			data.shader_override[p_name]->connect("changed", this, "_override_changed", Vector<Variant>(), CONNECT_REFERENCE_COUNTED);
		}
	}
	notification(NOTIFICATION_THEME_CHANGED);
}
void iControl::add_style_override(const StringName &p_name, const Ref<StyleBox> &p_style) {

	if (data.style_override.has(p_name)) {
		data.style_override[p_name]->disconnect("changed", this, "_override_changed");
	}

	// clear if "null" is passed instead of a style
	if (p_style.is_null()) {
		data.style_override.erase(p_name);
	} else {
		data.style_override[p_name] = p_style;
		if (data.style_override[p_name].is_valid()) {
			data.style_override[p_name]->connect("changed", this, "_override_changed", Vector<Variant>(), CONNECT_REFERENCE_COUNTED);
		}
	}
	notification(NOTIFICATION_THEME_CHANGED);
}

void iControl::add_font_override(const StringName &p_name, const Ref<Font> &p_font) {

	if (data.font_override.has(p_name)) {
		data.font_override[p_name]->disconnect("changed", this, "_override_changed");
	}

	// clear if "null" is passed instead of a font
	if (p_font.is_null()) {
		data.font_override.erase(p_name);
	} else {
		data.font_override[p_name] = p_font;
		if (data.font_override[p_name].is_valid()) {
			data.font_override[p_name]->connect("changed", this, "_override_changed", Vector<Variant>(), CONNECT_REFERENCE_COUNTED);
		}
	}
	notification(NOTIFICATION_THEME_CHANGED);
}
void iControl::add_color_override(const StringName &p_name, const Color &p_color) {

	data.color_override[p_name] = p_color;
	notification(NOTIFICATION_THEME_CHANGED);
}
void iControl::add_constant_override(const StringName &p_name, int p_constant) {

	data.constant_override[p_name] = p_constant;
	notification(NOTIFICATION_THEME_CHANGED);
}

static iControl *_next_iControl(iControl *p_from) {

	if (p_from->is_set_as_toplevel())
		return NULL; // can't go above

	iControl *parent = Object::cast_to<iControl>(p_from->get_parent());

	if (!parent) {

		return NULL;
	}

	int next = p_from->get_position_in_parent();
	ERR_FAIL_INDEX_V(next, parent->get_child_count(), NULL);
	for (int i = (next + 1); i < parent->get_child_count(); i++) {

		iControl *c = Object::cast_to<iControl>(parent->get_child(i));
		if (!c || !c->is_visible_in_tree() || c->is_set_as_toplevel())
			continue;

		return c;
	}

	//no next in parent, try the same in parent
	return _next_iControl(parent);
}

static iControl *_prev_iControl(iControl *p_from) {

	iControl *child = NULL;
	for (int i = p_from->get_child_count() - 1; i >= 0; i--) {

		iControl *c = Object::cast_to<iControl>(p_from->get_child(i));
		if (!c || !c->is_visible_in_tree() || c->is_set_as_toplevel())
			continue;

		child = c;
		break;
	}

	if (!child)
		return p_from;

	//no prev in parent, try the same in parent
	return _prev_iControl(child);
}

void iControl::_propagate_theme_changed(CanvasItem *p_at, iControl *p_owner, bool p_assign) {

	iControl *c = Object::cast_to<iControl>(p_at);

	if (c && c != p_owner && c->data.theme.is_valid()) // has a theme, this can't be propagated
		return;

	for (int i = 0; i < p_at->get_child_count(); i++) {

		CanvasItem *child = Object::cast_to<CanvasItem>(p_at->get_child(i));
		if (child) {
			_propagate_theme_changed(child, p_owner, p_assign);
		}
	}

	if (c) {

		if (p_assign) {
			c->data.theme_owner = p_owner;
		}
		c->notification(NOTIFICATION_THEME_CHANGED);
	}
}

void iControl::_theme_changed() {

	_propagate_theme_changed(this, this, false);
}

void iControl::set_theme(const Ref<Theme> &p_theme) {

	if (data.theme == p_theme)
		return;

	if (data.theme.is_valid()) {
		data.theme->disconnect("changed", this, "_theme_changed");
	}

	data.theme = p_theme;
	if (!p_theme.is_null()) {

		data.theme_owner = this;
		_propagate_theme_changed(this, this);
	} else {

		iControl *parent = cast_to<iControl>(get_parent());
		if (parent && parent->data.theme_owner) {
			_propagate_theme_changed(this, parent->data.theme_owner);
		} else {

			_propagate_theme_changed(this, NULL);
		}
	}

	if (data.theme.is_valid()) {
		data.theme->connect("changed", this, "_theme_changed", varray(), CONNECT_DEFERRED);
	}
}

Ref<Theme> iControl::get_theme() const {

	return data.theme;
}

Transform2D iControl::get_transform() const {

	Transform2D xform = _get_internal_transform();
	xform[2] += get_position();
	return xform;
}

void iControl::set_h_size_flags(int p_flags) {

	if (data.h_size_flags == p_flags)
		return;
	data.h_size_flags = p_flags;
	emit_signal(SceneStringNames::get_singleton()->size_flags_changed);
}

int iControl::get_h_size_flags() const {
	return data.h_size_flags;
}
void iControl::set_v_size_flags(int p_flags) {

	if (data.v_size_flags == p_flags)
		return;
	data.v_size_flags = p_flags;
	emit_signal(SceneStringNames::get_singleton()->size_flags_changed);
}

void iControl::set_stretch_ratio(float p_ratio) {

	if (data.expand == p_ratio)
		return;

	data.expand = p_ratio;
	emit_signal(SceneStringNames::get_singleton()->size_flags_changed);
}

float iControl::get_stretch_ratio() const {

	return data.expand;
}

void iControl::minimum_size_changed() {

	if (!is_inside_tree() || data.block_minimum_size_adjust)
		return;

	iControl *invalidate = this;

	//invalidate cache upwards
	while (invalidate && invalidate->data.minimum_size_valid) {
		invalidate->data.minimum_size_valid = false;
		if (invalidate->is_set_as_toplevel())
			break; // do not go further up
		invalidate = invalidate->data.parent;
	}

	if (!is_visible_in_tree())
		return;

	if (data.updating_last_minimum_size)
		return;

	data.updating_last_minimum_size = true;

	MessageQueue::get_singleton()->push_call(this, "_update_minimum_size");
}

int iControl::get_v_size_flags() const {
	return data.v_size_flags;
}

int iControl::get_finger_index() const {
	return data.finger_pressed;
}

const bool iControl::is_passby_press() const {
	return data.passby_press;
}

void iControl::set_passby_press(const bool p_passby_press) {
	data.passby_press = p_passby_press;
	update();
}

void iControl::set_rotation(float p_radians) {

	data.rotation = p_radians;
	update();
	_notify_transform();
	_change_notify("rect_rotation");
}

float iControl::get_rotation() const {

	return data.rotation;
}

void iControl::set_rotation_degrees(float p_degrees) {
	set_rotation(Math::deg2rad(p_degrees));
}

float iControl::get_rotation_degrees() const {
	return Math::rad2deg(get_rotation());
}

void iControl::_override_changed() {

	notification(NOTIFICATION_THEME_CHANGED);
	minimum_size_changed(); // overrides are likely to affect minimum size
}

void iControl::set_pivot_offset(const Vector2 &p_pivot) {

	data.pivot_offset = p_pivot;
	update();
	_notify_transform();
	_change_notify("rect_pivot_offset");
}

Vector2 iControl::get_pivot_offset() const {

	return data.pivot_offset;
}

void iControl::set_scale(const Vector2 &p_scale) {

	data.scale = p_scale;
	// Avoid having 0 scale values, can lead to errors in physics and rendering.
	if (data.scale.x == 0)
		data.scale.x = CMP_EPSILON;
	if (data.scale.y == 0)
		data.scale.y = CMP_EPSILON;
	update();
	_notify_transform();
}
Vector2 iControl::get_scale() const {

	return data.scale;
}

void iControl::set_block_minimum_size_adjust(bool p_block) {
	data.block_minimum_size_adjust = p_block;
}

bool iControl::is_minimum_size_adjust_blocked() const {

	return data.block_minimum_size_adjust;
}

void iControl::set_disable_visibility_clip(bool p_ignore) {

	data.disable_visibility_clip = p_ignore;
	update();
}

bool iControl::is_visibility_clip_disabled() const {

	return data.disable_visibility_clip;
}

void iControl::get_argument_options(const StringName &p_function, int p_idx, List<String> *r_options) const {

#ifdef TOOLS_ENABLED
	const String quote_style = EDITOR_DEF("text_editor/completion/use_single_quotes", 0) ? "'" : "\"";
#else
	const String quote_style = "\"";
#endif

	Node::get_argument_options(p_function, p_idx, r_options);

	if (p_idx == 0) {
		List<StringName> sn;
		String pf = p_function;
		if (pf == "add_color_override" || pf == "has_color" || pf == "has_color_override" || pf == "get_color") {
			Theme::get_default()->get_color_list(get_class(), &sn);
		} else if (pf == "add_style_override" || pf == "has_style" || pf == "has_style_override" || pf == "get_style") {
			Theme::get_default()->get_stylebox_list(get_class(), &sn);
		} else if (pf == "add_font_override" || pf == "has_font" || pf == "has_font_override" || pf == "get_font") {
			Theme::get_default()->get_font_list(get_class(), &sn);
		} else if (pf == "add_constant_override" || pf == "has_constant" || pf == "has_constant_override" || pf == "get_constant") {
			Theme::get_default()->get_constant_list(get_class(), &sn);
		}

		sn.sort_custom<StringName::AlphCompare>();
		for (List<StringName>::Element *E = sn.front(); E; E = E->next()) {
			r_options->push_back(quote_style + E->get() + quote_style);
		}
	}
}

void iControl::set_clip_contents(bool p_clip) {

	data.clip_contents = p_clip;
	update();
}

bool iControl::is_clipping_contents() {

	return data.clip_contents;
}

void iControl::set_h_grow_direction(GrowDirection p_direction) {

	ERR_FAIL_INDEX((int)p_direction, 3);

	data.h_grow = p_direction;
	_size_changed();
}

iControl::GrowDirection iControl::get_h_grow_direction() const {

	return data.h_grow;
}

void iControl::set_v_grow_direction(GrowDirection p_direction) {

	ERR_FAIL_INDEX((int)p_direction, 3);

	data.v_grow = p_direction;
	_size_changed();
}
iControl::GrowDirection iControl::get_v_grow_direction() const {

	return data.v_grow;
}

void iControl::_bind_methods() {

	//ClassDB::bind_method(D_METHOD("_window_resize_event"),&iControl::_window_resize_event);
	ClassDB::bind_method(D_METHOD("_size_changed"), &iControl::_size_changed);
	ClassDB::bind_method(D_METHOD("_update_minimum_size"), &iControl::_update_minimum_size);

	ClassDB::bind_method(D_METHOD("get_minimum_size"), &iControl::get_minimum_size);
	ClassDB::bind_method(D_METHOD("get_combined_minimum_size"), &iControl::get_combined_minimum_size);
	ClassDB::bind_method(D_METHOD("set_anchors_preset", "preset", "keep_margins"), &iControl::set_anchors_preset, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("set_margins_preset", "preset", "resize_mode", "margin"), &iControl::set_margins_preset, DEFVAL(PRESET_MODE_MINSIZE), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_anchors_and_margins_preset", "preset", "resize_mode", "margin"), &iControl::set_anchors_and_margins_preset, DEFVAL(PRESET_MODE_MINSIZE), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("_set_anchor", "margin", "anchor"), &iControl::_set_anchor);
	ClassDB::bind_method(D_METHOD("set_anchor", "margin", "anchor", "keep_margin", "push_opposite_anchor"), &iControl::set_anchor, DEFVAL(false), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("get_anchor", "margin"), &iControl::get_anchor);
	ClassDB::bind_method(D_METHOD("set_margin", "margin", "offset"), &iControl::set_margin);
	ClassDB::bind_method(D_METHOD("set_anchor_and_margin", "margin", "anchor", "offset", "push_opposite_anchor"), &iControl::set_anchor_and_margin, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("set_begin", "position"), &iControl::set_begin);
	ClassDB::bind_method(D_METHOD("set_end", "position"), &iControl::set_end);
	ClassDB::bind_method(D_METHOD("set_position", "position", "keep_margins"), &iControl::set_position, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("_set_position", "margin"), &iControl::_set_position);
	ClassDB::bind_method(D_METHOD("set_size", "size", "keep_margins"), &iControl::set_size, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("_set_size", "size"), &iControl::_set_size);
	ClassDB::bind_method(D_METHOD("set_custom_minimum_size", "size"), &iControl::set_custom_minimum_size);
	ClassDB::bind_method(D_METHOD("set_global_position", "position", "keep_margins"), &iControl::set_global_position, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("_set_global_position", "position"), &iControl::_set_global_position);
	ClassDB::bind_method(D_METHOD("set_rotation", "radians"), &iControl::set_rotation);
	ClassDB::bind_method(D_METHOD("set_rotation_degrees", "degrees"), &iControl::set_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &iControl::set_scale);
	ClassDB::bind_method(D_METHOD("set_pivot_offset", "pivot_offset"), &iControl::set_pivot_offset);
	ClassDB::bind_method(D_METHOD("get_margin", "margin"), &iControl::get_margin);
	ClassDB::bind_method(D_METHOD("get_begin"), &iControl::get_begin);
	ClassDB::bind_method(D_METHOD("get_end"), &iControl::get_end);
	ClassDB::bind_method(D_METHOD("get_position"), &iControl::get_position);
	ClassDB::bind_method(D_METHOD("get_size"), &iControl::get_size);
	ClassDB::bind_method(D_METHOD("get_rotation"), &iControl::get_rotation);
	ClassDB::bind_method(D_METHOD("get_rotation_degrees"), &iControl::get_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_scale"), &iControl::get_scale);
	ClassDB::bind_method(D_METHOD("get_pivot_offset"), &iControl::get_pivot_offset);
	ClassDB::bind_method(D_METHOD("get_custom_minimum_size"), &iControl::get_custom_minimum_size);
	ClassDB::bind_method(D_METHOD("get_parent_area_size"), &iControl::get_parent_area_size);
	ClassDB::bind_method(D_METHOD("get_global_position"), &iControl::get_global_position);
	ClassDB::bind_method(D_METHOD("get_rect"), &iControl::get_rect);
	ClassDB::bind_method(D_METHOD("get_global_rect"), &iControl::get_global_rect);

	ClassDB::bind_method(D_METHOD("set_h_size_flags", "flags"), &iControl::set_h_size_flags);
	ClassDB::bind_method(D_METHOD("get_h_size_flags"), &iControl::get_h_size_flags);

	ClassDB::bind_method(D_METHOD("set_stretch_ratio", "ratio"), &iControl::set_stretch_ratio);
	ClassDB::bind_method(D_METHOD("get_stretch_ratio"), &iControl::get_stretch_ratio);

	ClassDB::bind_method(D_METHOD("set_v_size_flags", "flags"), &iControl::set_v_size_flags);
	ClassDB::bind_method(D_METHOD("get_v_size_flags"), &iControl::get_v_size_flags);

	ClassDB::bind_method(D_METHOD("set_theme", "theme"), &iControl::set_theme);
	ClassDB::bind_method(D_METHOD("get_theme"), &iControl::get_theme);

	ClassDB::bind_method(D_METHOD("add_icon_override", "name", "texture"), &iControl::add_icon_override);
	ClassDB::bind_method(D_METHOD("add_shader_override", "name", "shader"), &iControl::add_shader_override);
	ClassDB::bind_method(D_METHOD("add_stylebox_override", "name", "stylebox"), &iControl::add_style_override);
	ClassDB::bind_method(D_METHOD("add_font_override", "name", "font"), &iControl::add_font_override);
	ClassDB::bind_method(D_METHOD("add_color_override", "name", "color"), &iControl::add_color_override);
	ClassDB::bind_method(D_METHOD("add_constant_override", "name", "constant"), &iControl::add_constant_override);

	ClassDB::bind_method(D_METHOD("get_icon", "name", "node_type"), &iControl::get_icon, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_stylebox", "name", "node_type"), &iControl::get_stylebox, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_font", "name", "node_type"), &iControl::get_font, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_color", "name", "node_type"), &iControl::get_color, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_constant", "name", "node_type"), &iControl::get_constant, DEFVAL(""));

	ClassDB::bind_method(D_METHOD("has_icon_override", "name"), &iControl::has_icon_override);
	ClassDB::bind_method(D_METHOD("has_shader_override", "name"), &iControl::has_shader_override);
	ClassDB::bind_method(D_METHOD("has_stylebox_override", "name"), &iControl::has_stylebox_override);
	ClassDB::bind_method(D_METHOD("has_font_override", "name"), &iControl::has_font_override);
	ClassDB::bind_method(D_METHOD("has_color_override", "name"), &iControl::has_color_override);
	ClassDB::bind_method(D_METHOD("has_constant_override", "name"), &iControl::has_constant_override);

	ClassDB::bind_method(D_METHOD("has_icon", "name", "node_type"), &iControl::has_icon, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("has_stylebox", "name", "node_type"), &iControl::has_stylebox, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("has_font", "name", "node_type"), &iControl::has_font, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("has_color", "name", "node_type"), &iControl::has_color, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("has_constant", "name", "node_type"), &iControl::has_constant, DEFVAL(""));

	ClassDB::bind_method(D_METHOD("get_parent_iControl"), &iControl::get_parent_iControl);

	ClassDB::bind_method(D_METHOD("get_finger_index"), &iControl::get_finger_index);

	ClassDB::bind_method(D_METHOD("is_passby_press"), &iControl::is_passby_press);
	ClassDB::bind_method(D_METHOD("set_passby_press", "passby_press"), &iControl::set_passby_press);

	ClassDB::bind_method(D_METHOD("set_h_grow_direction", "direction"), &iControl::set_h_grow_direction);
	ClassDB::bind_method(D_METHOD("get_h_grow_direction"), &iControl::get_h_grow_direction);

	ClassDB::bind_method(D_METHOD("set_v_grow_direction", "direction"), &iControl::set_v_grow_direction);
	ClassDB::bind_method(D_METHOD("get_v_grow_direction"), &iControl::get_v_grow_direction);

	ClassDB::bind_method(D_METHOD("set_clip_contents", "enable"), &iControl::set_clip_contents);
	ClassDB::bind_method(D_METHOD("is_clipping_contents"), &iControl::is_clipping_contents);

	ClassDB::bind_method(D_METHOD("minimum_size_changed"), &iControl::minimum_size_changed);

	ClassDB::bind_method(D_METHOD("_theme_changed"), &iControl::_theme_changed);

	ClassDB::bind_method(D_METHOD("_override_changed"), &iControl::_override_changed);

	BIND_VMETHOD(MethodInfo("_gui_input", PropertyInfo(Variant::OBJECT, "event", PROPERTY_HINT_RESOURCE_TYPE, "InputEvent")));
	BIND_VMETHOD(MethodInfo(Variant::VECTOR2, "_get_minimum_size"));

	MethodInfo get_drag_data = MethodInfo("get_drag_data", PropertyInfo(Variant::VECTOR2, "position"));
	get_drag_data.return_val.usage |= PROPERTY_USAGE_NIL_IS_VARIANT;
	BIND_VMETHOD(get_drag_data);

	BIND_VMETHOD(MethodInfo(Variant::BOOL, "can_drop_data", PropertyInfo(Variant::VECTOR2, "position"), PropertyInfo(Variant::NIL, "data")));
	BIND_VMETHOD(MethodInfo("drop_data", PropertyInfo(Variant::VECTOR2, "position"), PropertyInfo(Variant::NIL, "data")));
	BIND_VMETHOD(MethodInfo(
			PropertyInfo(Variant::OBJECT, "iControl", PROPERTY_HINT_RESOURCE_TYPE, "iControl"),
			"_make_custom_tooltip", PropertyInfo(Variant::STRING, "for_text")));
	BIND_VMETHOD(MethodInfo(Variant::BOOL, "_clips_input"));

	ADD_GROUP("Anchor", "anchor_");
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anchor_left", PROPERTY_HINT_RANGE, "0,1,0.001,or_lesser,or_greater"), "_set_anchor", "get_anchor", MARGIN_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anchor_top", PROPERTY_HINT_RANGE, "0,1,0.001,or_lesser,or_greater"), "_set_anchor", "get_anchor", MARGIN_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anchor_right", PROPERTY_HINT_RANGE, "0,1,0.001,or_lesser,or_greater"), "_set_anchor", "get_anchor", MARGIN_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::REAL, "anchor_bottom", PROPERTY_HINT_RANGE, "0,1,0.001,or_lesser,or_greater"), "_set_anchor", "get_anchor", MARGIN_BOTTOM);

	ADD_GROUP("Margin", "margin_");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "margin_left", PROPERTY_HINT_RANGE, "-4096,4096"), "set_margin", "get_margin", MARGIN_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "margin_top", PROPERTY_HINT_RANGE, "-4096,4096"), "set_margin", "get_margin", MARGIN_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "margin_right", PROPERTY_HINT_RANGE, "-4096,4096"), "set_margin", "get_margin", MARGIN_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "margin_bottom", PROPERTY_HINT_RANGE, "-4096,4096"), "set_margin", "get_margin", MARGIN_BOTTOM);

	ADD_GROUP("Grow Direction", "grow_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "grow_horizontal", PROPERTY_HINT_ENUM, "Begin,End,Both"), "set_h_grow_direction", "get_h_grow_direction");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "grow_vertical", PROPERTY_HINT_ENUM, "Begin,End,Both"), "set_v_grow_direction", "get_v_grow_direction");

	ADD_GROUP("Rect", "rect_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_position", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "_set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_global_position", PROPERTY_HINT_NONE, "", 0), "_set_global_position", "get_global_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_size", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "_set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_min_size"), "set_custom_minimum_size", "get_custom_minimum_size");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "rect_rotation", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater"), "set_rotation_degrees", "get_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_scale"), "set_scale", "get_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rect_pivot_offset"), "set_pivot_offset", "get_pivot_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rect_clip_content"), "set_clip_contents", "is_clipping_contents");

	ADD_GROUP("Size Flags", "size_flags_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size_flags_horizontal", PROPERTY_HINT_FLAGS, "Fill,Expand,Shrink Center,Shrink End"), "set_h_size_flags", "get_h_size_flags");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size_flags_vertical", PROPERTY_HINT_FLAGS, "Fill,Expand,Shrink Center,Shrink End"), "set_v_size_flags", "get_v_size_flags");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "size_flags_stretch_ratio", PROPERTY_HINT_RANGE, "0,20,0.01,or_greater"), "set_stretch_ratio", "get_stretch_ratio");

	ADD_GROUP("Touch", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "passby press"), "set_passby_press", "is_passby_press");

	ADD_GROUP("Theme", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "theme", PROPERTY_HINT_RESOURCE_TYPE, "Theme"), "set_theme", "get_theme");
	ADD_GROUP("", "");

	BIND_CONSTANT(NOTIFICATION_RESIZED);
	BIND_CONSTANT(NOTIFICATION_THEME_CHANGED);
	BIND_CONSTANT(NOTIFICATION_MODAL_CLOSE);
	BIND_CONSTANT(NOTIFICATION_SCROLL_BEGIN);
	BIND_CONSTANT(NOTIFICATION_SCROLL_END);

	BIND_ENUM_CONSTANT(PRESET_TOP_LEFT);
	BIND_ENUM_CONSTANT(PRESET_TOP_RIGHT);
	BIND_ENUM_CONSTANT(PRESET_BOTTOM_LEFT);
	BIND_ENUM_CONSTANT(PRESET_BOTTOM_RIGHT);
	BIND_ENUM_CONSTANT(PRESET_CENTER_LEFT);
	BIND_ENUM_CONSTANT(PRESET_CENTER_TOP);
	BIND_ENUM_CONSTANT(PRESET_CENTER_RIGHT);
	BIND_ENUM_CONSTANT(PRESET_CENTER_BOTTOM);
	BIND_ENUM_CONSTANT(PRESET_CENTER);
	BIND_ENUM_CONSTANT(PRESET_LEFT_WIDE);
	BIND_ENUM_CONSTANT(PRESET_TOP_WIDE);
	BIND_ENUM_CONSTANT(PRESET_RIGHT_WIDE);
	BIND_ENUM_CONSTANT(PRESET_BOTTOM_WIDE);
	BIND_ENUM_CONSTANT(PRESET_VCENTER_WIDE);
	BIND_ENUM_CONSTANT(PRESET_HCENTER_WIDE);
	BIND_ENUM_CONSTANT(PRESET_WIDE);

	BIND_ENUM_CONSTANT(PRESET_MODE_MINSIZE);
	BIND_ENUM_CONSTANT(PRESET_MODE_KEEP_WIDTH);
	BIND_ENUM_CONSTANT(PRESET_MODE_KEEP_HEIGHT);
	BIND_ENUM_CONSTANT(PRESET_MODE_KEEP_SIZE);

	BIND_ENUM_CONSTANT(SIZE_FILL);
	BIND_ENUM_CONSTANT(SIZE_EXPAND);
	BIND_ENUM_CONSTANT(SIZE_EXPAND_FILL);
	BIND_ENUM_CONSTANT(SIZE_SHRINK_CENTER);
	BIND_ENUM_CONSTANT(SIZE_SHRINK_END);

	BIND_ENUM_CONSTANT(GROW_DIRECTION_BEGIN);
	BIND_ENUM_CONSTANT(GROW_DIRECTION_END);
	BIND_ENUM_CONSTANT(GROW_DIRECTION_BOTH);

	BIND_ENUM_CONSTANT(ANCHOR_BEGIN);
	BIND_ENUM_CONSTANT(ANCHOR_END);

	ADD_SIGNAL(MethodInfo("resized"));
	ADD_SIGNAL(MethodInfo("gui_input", PropertyInfo(Variant::OBJECT, "event", PROPERTY_HINT_RESOURCE_TYPE, "InputEvent")));
	ADD_SIGNAL(MethodInfo("size_flags_changed"));
	ADD_SIGNAL(MethodInfo("minimum_size_changed"));
	ADD_SIGNAL(MethodInfo("modal_closed"));

	BIND_VMETHOD(MethodInfo(Variant::BOOL, "has_point", PropertyInfo(Variant::VECTOR2, "point")));
}
iControl::iControl() {

	data.parent = NULL;

	data.theme_owner = NULL;
	data.h_size_flags = SIZE_FILL;
	data.v_size_flags = SIZE_FILL;
	data.expand = 1;
	data.rotation = 0;
	data.parent_canvas_item = NULL;
	data.scale = Vector2(1, 1);
	data.block_minimum_size_adjust = false;
	data.disable_visibility_clip = false;
	data.h_grow = GROW_DIRECTION_END;
	data.v_grow = GROW_DIRECTION_END;
	data.minimum_size_valid = false;
	data.updating_last_minimum_size = false;

	data.clip_contents = false;
	for (int i = 0; i < 4; i++) {
		data.anchor[i] = ANCHOR_BEGIN;
		data.margin[i] = 0;
	}
}
