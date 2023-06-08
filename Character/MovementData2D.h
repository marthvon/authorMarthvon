#ifndef MOVEMENT_DATA_2D
#define MOVEMENT_DATA_2D

#include "GroundedMovementData1D.h"

class MovementData2D : public GroundedMovementData1D {
	GDCLASS(MovementData2D, GroundedMovementData1D);
	OBJ_SAVE_TYPE(MovementData2D);

private:
	real_t initial_jump_velocity = 0;
	real_t jump_duration = 0;
	real_t jump_height = 0;
	real_t gravity = 0;
	real_t xVel_to_yVel_ratio = 0;
	real_t scale_inherited_ySpeed = 0;
protected:
	Vector2 _get_velocity(const Data& p_data) const override;

	static void _bind_methods();
public:
	void set_jump_height(const real_t p_height);
	real_t get_jump_height() const;
	void set_jump_duration(const real_t p_duration);
	real_t get_jump_duration() const;
	void set_gravity(const real_t p_gravity, const bool keep_jump_duration = false);
	real_t get_gravity() const;
	real_t get_initial_jump_velocity() const;
	void set_xVel2yVel_ratio(const real_t p_ratio);
	real_t get_xVel2yVel_ratio() const;
	void set_scale_inherited_ySpeed(const real_t p_scale);
	real_t get_scale_inherited_ySpeed() const;
private:
	void _update_jump_cache();
};

#endif
