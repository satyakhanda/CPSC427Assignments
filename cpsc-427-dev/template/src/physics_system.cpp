// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "ai_system.hpp"

AISystem ai;


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
		
		if (motion.angle != 0) {
			if (motion.velocity.y == 0) {
				//printf("%s", "HERERE");
				motion.position.x += motion.velocity.x * -1 * (cosf(motion.angle+M_PI/2)) * step_seconds;
				motion.position.y += motion.velocity.x * (sinf(motion.angle+M_PI/2)) * step_seconds;
			} else {
				// printf("%s", "SCARY");
				motion.position.x += motion.velocity.x * -1 * cosf(motion.angle) * step_seconds;
				motion.position.y += motion.velocity.y * sinf(motion.angle) * step_seconds;
			}
		} else {
			// printf("%s", "TIME");
			if (motion.scale.x == -BUG_BB_WIDTH && motion.scale.y == BUG_BB_HEIGHT) {
				if (motion.position.x - (BUG_BB_WIDTH/2) <= 0) {
					motion.velocity = vec2{-motion.velocity.x, motion.velocity.y};
					motion.position.x += motion.velocity.x * step_seconds;
					motion.position.y += motion.velocity.y * step_seconds;
				} else if (motion.position.x  >= window_width_px) {
					motion.velocity = vec2{-motion.velocity.x, motion.velocity.y};
					motion.position.x += motion.velocity.x * step_seconds;
					motion.position.y += motion.velocity.y * step_seconds;
				}
			}
			motion.position.x += motion.velocity.x * step_seconds;
			motion.position.y += motion.velocity.y * step_seconds;
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
	(float)window_width_px; 
	(float)window_height_px;

	ComponentContainer<Player>& player_container = registry.players;
	for (uint i = 0; i < player_container.components.size(); i++) {
		Entity player_entity = player_container.entities[i];
		Motion& motion = registry.motions.get(player_entity);
		Mesh* mesh_pointers = registry.meshPtrs.get(player_entity);

		std::vector<ColoredVertex>& vertices = mesh_pointers->vertices;
		for (uint j = 0; j < vertices.size(); j++) {
			float x_angle = cosf(motion.angle);
			float y_angle = sinf(motion.angle);
			mat2 rotation = {{x_angle,-y_angle},{y_angle,x_angle}};
			mat2 scale = {{motion.scale.x,0.f},{0.f,motion.scale.y}};
			vec2 position = {motion.position.x, motion.position.y};
			vec2 vertex = (vertices[j].position) * scale;
			vertex = rotation * vertex;
			vertex += position; 
			if(vertex.x <= 0) {
				motion.position.x += 6;
			} else if (vertex.x >= window_width_px) {
				motion.position.x -= 6;
			}
			
			if(debugging.in_debug_mode){
				Entity meshLines = createLine({vertex.x, vertex.y}, {motion.scale.x/20, motion.scale.y/20});
			}
		}
	}



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

			if (registry.debugComponents.has(entity_i))
				continue;

			if (motion_i.scale.x == -BUG_BB_WIDTH && motion_i.scale.y == BUG_BB_HEIGHT) {
				const vec2 bounding_box = get_bounding_box(motion_i);
				float radius = sqrt(dot(bounding_box / 2.f, bounding_box / 2.f));
				if (motion_i.velocity.y == 0) {
					if (motion_i.velocity.x > 0) {
						vec2 motion_pos = {motion_i.position.x + radius, motion_i.position.y};
						vec2 line_scale = {radius * 2, motion_i.scale.x / 20};
						Entity line = createLine(motion_pos, line_scale);
					}
					if (motion_i.velocity.x < 0) {
						vec2 motion_pos = {motion_i.position.x - radius, motion_i.position.y};
						vec2 line_scale = {radius * 2, motion_i.scale.x / 20};
						Entity line = createLine(motion_pos, line_scale);
					}
				} else if (motion_i.velocity.x == 0) {
						vec2 motion_pos = {motion_i.position.x, motion_i.position.y + radius};
						vec2 line_scale = {motion_i.scale.x / 20, radius * 2};
						Entity line = createLine(motion_pos, line_scale);
				}
			}

			if (registry.players.has(entity_i) && ai.getInProx()) {
				const vec2 bounding_box = get_bounding_box(motion_i);
				float radius = sqrt(dot(bounding_box / 2.f, bounding_box / 2.f));

				vec2 motion_pos1 = {motion_i.position.x + 350, motion_i.position.y};
				vec2 line_scale1 = {motion_i.scale.x / 10, 5.f * radius};
				Entity line1 = createLine(motion_pos1, line_scale1);

				vec2 motion_pos2 = {motion_i.position.x, motion_i.position.y + 200};
				vec2 line_scale2 = {10 * radius, motion_i.scale.x / 10};
				Entity line2 = createLine(motion_pos2, line_scale2);

				vec2 motion_pos3 = {motion_i.position.x - 350, motion_i.position.y};
				vec2 line_scale3 = {motion_i.scale.x / 10, 5.f * radius};
				Entity line3 = createLine(motion_pos3, line_scale3);

				vec2 motion_pos4 = {motion_i.position.x, motion_i.position.y - 200};
				vec2 line_scale4 = {10 * radius, motion_i.scale.x / 10};
				Entity line4 = createLine(motion_pos4, line_scale4);
			}

			const vec2 bounding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bounding_box / 2.f, bounding_box / 2.f));

			vec2 motion_pos1 = {motion_i.position.x + radius, motion_i.position.y};
			vec2 line_scale1 = {motion_i.scale.x / 20, 2 * radius};
			Entity line1 = createLine(motion_pos1, line_scale1);

			vec2 motion_pos2 = {motion_i.position.x, motion_i.position.y + radius};
			vec2 line_scale2 = {2 * radius, motion_i.scale.x / 20};
			Entity line2 = createLine(motion_pos2, line_scale2);

			vec2 motion_pos3 = {motion_i.position.x - radius, motion_i.position.y};
			vec2 line_scale3 = {motion_i.scale.x / 20, 2 * radius};
			Entity line3 = createLine(motion_pos3, line_scale3);

			vec2 motion_pos4 = {motion_i.position.x, motion_i.position.y - radius};
			vec2 line_scale4 = {2 * radius, motion_i.scale.x / 20};
			Entity line4 = createLine(motion_pos4, line_scale4);
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE EGG collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}