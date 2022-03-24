// internal
#include "ai_system.hpp"

Advanced advanceMode;
void AISystem::step(float elapsed_ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE BUG AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Bug AI.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (registry.advanced.components[0].adv) {
	
		printf("ENTER AI\n");

		for (Entity eagle : registry.deadlys.entities) {
			eagle_AI(eagle, elapsed_ms);
		}
	}

	for (Entity player : registry.players.entities) {
		Motion player_motion = registry.motions.get(player);
		ComponentContainer<AI> &ai_container = registry.ai;
		for (uint i = 0; i < ai_container.components.size(); i++) {
			bug_AI(i, player_motion, elapsed_ms);
		}
	}


}
float eagle_counter = 200.f;
void AISystem::eagle_AI(Entity eagle_entity, float elapsed_ms) {
	printf("ENTER AI\n");
	Motion& eagle_motion = registry.motions.get(eagle_entity);

	Entity player = registry.players.entities[0];
	Motion player_motion = registry.motions.get(player);

	vec2 dist_vector = {eagle_motion.position.x - player_motion.position.x, eagle_motion.position.y - player_motion.position.y};
	float dist = distance(player_motion.position, eagle_motion.position);
	dist_vector = {dist_vector.x/dist, dist_vector.y/dist};

	ComponentContainer<AI> &ai_container = registry.ai;
	for (uint i = 0; i < ai_container.components.size(); i++) {
		Entity bug_entity = registry.ai.entities[i];
		Motion& bug_motion = registry.motions.get(bug_entity);

		// vec2 v = eagle_motion.position;
		// vec2 w = player_motion.position;
		// vec2 p = bug_motion.position;

		// float l2 = distance(v,w);
		// float temp = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
		// float min = 1 < temp ? 1 : temp;
		// float t = 0 > min ? 0 : min;
		// vec2 projection = v + t * (w - v);
		// float bug_line_dist = distance(p, projection);
		// printf("BUG DIST%f\n", bug_line_dist);

		if (eagle_counter > 0) {
			if (dist <= 500) {
				eagle_motion.velocity.x = -(dist_vector.x) * 100.f;
				eagle_motion.velocity.y = -dist_vector.y * 100.f;
			}
		} else {
			eagle_counter = 200.f;
		}
		// if (eagle_counter > 0) {
		// 	eagle_counter -= elapsed_ms;
		// 	if (bug_line_dist < 300) {
		// 		if (projection.x > bug_motion.position.x) {
		// 			dist_vector = {eagle_motion.position.x - projection.x - 50, eagle_motion.position.y - projection.y};
		// 			dist = distance(player_motion.position, eagle_motion.position);
		// 			dist_vector = {dist_vector.x/dist, dist_vector.y/dist};
		// 			eagle_motion.velocity.x = -(dist_vector.x) * 75.f;
		// 			eagle_motion.velocity.y = -dist_vector.y * 75.f;
		// 		} else {
		// 			dist_vector = {eagle_motion.position.x - projection.x + 50, eagle_motion.position.y - projection.y};
		// 			dist = distance(player_motion.position, eagle_motion.position);
		// 			dist_vector = {dist_vector.x/dist, dist_vector.y/dist};
		// 			eagle_motion.velocity.x = -(dist_vector.x) * 75.f;
		// 			eagle_motion.velocity.y = -dist_vector.y * 75.f;
		// 		}
		// 	} else {
		// 		if (dist <= 500) {
		// 			eagle_motion.velocity.x = -(dist_vector.x) * 75.f;
		// 			eagle_motion.velocity.y = -dist_vector.y * 75.f;
		// 		}
		// 	}
		// } else {
		// 	eagle_counter = 200.f;
		// }
	}

}

float counter = 150.f;
bool in_prox = false;

void AISystem::bug_AI(int index, Motion player_motion, float elapsed_ms) {
	AI& ai = registry.ai.components[index];
	Entity bug_entity = registry.ai.entities[index];
	Motion& bug_motion = registry.motions.get(bug_entity);

	ai.frames_counter -= elapsed_ms;

	if (ai.frames_counter <= 0) {
		ai.frames_counter = counter;
		if (distance(player_motion.position, bug_motion.position) <= ai.epsilon) {
			ai.been_inprox = true;
			in_prox = true;
			if (player_motion.position.x > bug_motion.position.x) {
				bug_motion.velocity.x = -50; 
			}
			else {
				bug_motion.velocity.x = 50; 
			}

			bug_motion.velocity.y = 0;
		} else {
			in_prox = false;
			if (ai.been_inprox) {
				bug_motion.velocity.x = 0;
				bug_motion.velocity.y = 60;
			}
		}

		if (debugging.in_debug_mode) {

		}
		debugging.in_freeze_mode = true;
	}
}

bool AISystem::getInProx()
{
	return in_prox;
} 