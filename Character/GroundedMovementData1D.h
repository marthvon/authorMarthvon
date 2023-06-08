#ifndef GROUNDED_MOVEMENT_DATA_1D
#define GROUNDED_MOVEMENT_DATA_1D

#include "core/io/resource.h"

class GroundedMovementData1D : public Resource {
	GDCLASS(GroundedMovementData1D, Resource);
	OBJ_SAVE_TYPE(GroundedMovementData1D);

private:
	real_t speed = 0;
	real_t acceleration = 0;
	real_t max_speed = 0;
	real_t min_speed = 0;
	real_t scale_inherited_speed = 0;

protected:
	struct Data {
		const Vector2 previous_speed; const double delta; const bool transitioning; //const Vector2 input;
		Data(const Vector2 p_previous_speed, const double p_delta, const bool p_transitioning);
	};
	virtual Vector2 _get_velocity(const Data& p_data) const;
	inline real_t _update_velocity(const real_t previous_speed, const double delta) const;
	inline real_t _assign_velocity(const real_t previous_speed, const double delta) const;

	static void _bind_methods();
public:
	void set_speed(const real_t p_speed);
	real_t get_speed() const;
	void set_acceleration(const real_t p_acceleration);
	real_t get_acceleration() const;
	void set_max_speed(const real_t p_speed);
	real_t get_max_speed() const;
	void set_min_speed(const real_t p_speed);
	real_t get_min_speed() const;
	void set_scale_inherited_speed(const real_t p_scale);
	real_t get_scale_inherited_speed() const;

	Vector2 get_velocity(const Vector2 previous_speed, const double delta, const bool transitioning = false) const;
};

#endif
