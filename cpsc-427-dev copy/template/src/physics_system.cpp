// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move bug based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		// (void)elapsed_ms; // placeholder to silence unused warning until implemented
		
		// Velocity is movement/second, multiply by number of seconds to get how much it has moved in the time
		if (!motion.advancedMode) {
			if (motion.angle == 0) {
				motion.position.x += motion.velocity.x * step_seconds;
				motion.position.y += motion.velocity.y * step_seconds;
			}
			else {
				if (motion.velocity.y != 0) {
					motion.position.x += motion.velocity.x * step_seconds * -1 * cos(motion.angle);
					motion.position.y += motion.velocity.y * step_seconds * sin(motion.angle);
				}
				else {
					motion.position.x += motion.velocity.x * step_seconds * -1 * cos(motion.angle + M_PI / 2);
					motion.position.y += motion.velocity.x * step_seconds * sin(motion.angle + M_PI / 2);
				}

			}
		}
		else {
			//float forceOfDrag = 0.5f * 1.525f * (motion.velocity.x * motion.velocity.x) * 2.f; // 0.525 is air density at STP in this universe
			// float decelerationDueToDrag = forceOfDrag / 2; // a = F/m
			//float dragDecel = motion.velocity.x * 0.0025f;
			float dragDecelX = .011f * (motion.velocity.x * motion.velocity.x);
			float dragDecelY = .011f * (motion.velocity.y * motion.velocity.y);
			if (motion.velocity.x < 0) {
				dragDecelX *= -1;
			}
			if (motion.velocity.y < 0) {
				dragDecelY *= -1;
			}
			printf("X Coord: Velocity:%.5f Acceleration:%.5f Decelleration:%.5f\n", motion.velocity.x, motion.acceleration.x, dragDecelX);
			printf("Y Coord: Velocity:%.5f Acceleration:%.5f Decelleration:%.5f\n", motion.velocity.y, motion.acceleration.y, dragDecelY);
			if (motion.velocity.x <= motion.terminalVelocity) {
				motion.velocity.x += (motion.acceleration.x * step_seconds) * 10;
			}

			if(motion.velocity.y <= motion.terminalVelocity) {
				motion.velocity.y += (motion.acceleration.y * step_seconds) * 10;
			}
			if (motion.acceleration.x == 0) {
				motion.velocity.x -= dragDecelX * step_seconds;
			}
			if (motion.acceleration.y == 0) {
				motion.velocity.y -= dragDecelY * step_seconds;
			}

			//if (motion.velocity.x <= 0) {
			//	motion.velocity.x = 0;
			//}

			//if (motion.velocity.y <= 0) {
			//	motion.velocity.y = 0;
			//}

//			if (motion.angle == 0) {
				motion.position.x += motion.velocity.x * step_seconds;
				motion.position.y += motion.velocity.y * step_seconds;
//			}
			//else {
			//	if (motion.velocity.y != 0) {
			//		motion.position.x += motion.velocity.x * step_seconds * -1 * cos(motion.angle);
			//		motion.position.y += motion.velocity.y * step_seconds * sin(motion.angle);
			//	}
			//	else {
			//		motion.position.x += motion.velocity.x * step_seconds * -1 * cos(motion.angle + M_PI / 2);
			//		motion.position.y += motion.velocity.x * step_seconds * sin(motion.angle + M_PI / 2);
			//	}

			//}
		}
		
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE CHICKEN - WALL collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// you may need the following quantities to compute wall positions
	(float)window_width_px; (float)window_height_px;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on Chicken mesh collision
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

			// don't draw debugging visuals around debug lines
			if (registry.debugComponents.has(entity_i))
				continue;

			// visualize the radius with two axis-aligned lines
			const vec2 bonding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bonding_box/2.f, bonding_box/2.f));
			vec2 line_scale1 = { motion_i.scale.x / 10, 2*radius };
			vec2 line_scale2 = { 2*radius, motion_i.scale.x / 10};
			vec2 position = motion_i.position;
			Entity line1 = createLine(motion_i.position, line_scale1);
			Entity line2 = createLine(motion_i.position, line_scale2);

			// !!! TODO A2: implement debug bounding boxes instead of crosses
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}