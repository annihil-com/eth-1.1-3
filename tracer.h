#ifndef _TRACER_H
#define _TRACER_H

// stupid include hell workaround
#ifndef ETH_H_
#include "eth.h"
#endif

#define RAILTRAIL_INTERVAL	13

void debug_head_box( int player );
qboolean trace_body( int player, vec3_t hitpoint );
qboolean trace_head( refEntity_t *head, vec3_t hitpoint, int player );

typedef enum {
	HITBOX_OFF,
	HITBOX_CUSTOM
} hitbox_type_t;

typedef enum {
	MS_STANDING,
	MS_CROUCHING,
	MS_PRONE
} model_state_t;

typedef struct {
	vec3_t stand_offset;
	vec3_t crouch_offset;
	vec3_t prone_offset;

	vec3_t stand_offset_moving;
	vec3_t crouch_offset_moving;
	
	vec3_t size;

	// for body boxes
	vec3_t size2;
} hitbox_t;

/* these define the head hitboxes. format is { fwd, right, up }, relative to the tag's orientation
	matched them as good as possibly with server head hitboxes using hacked servercode

	hitboxes don't rotate, but their origin does! it rotates with tag_head

	etpub/etmain boxes are really based on centity position, not on tag_head position, so need
	different aimbot code to handle those mods... for now use everything tag_head based

	always use correct hitbox mode.. wrong vecs == suck4ge
*/

const static hitbox_t head_hitboxes[] =
{
// stand               crouch         prone              stand moving      crouch moving    hitbox size (x,y,z)
{ { 0.0, 0.0, 0.0}, { 0.0, 0.0, 0.0}, { 0.0, 0.0, 0.0}, { 0.0, 0.0, 0.0}, { 0.0, 0.0, 0.0}, { 0.0, 0.0, 0.0} },	// OFF
{ { 0.5, 0.2, 5.5}, { 0.5, 0.2, 5.5}, { 0.5, 0.2, 5.5}, { 0.5, 0.0, 5.5}, { 0.5, 0.2, 5.5}, {10.0,10.0,13.0} },	// ETMAIN
{ { 0.3, 0.3, 7.0}, { -0.3, -0.8, 7.0}, { 0.0,  0.3, 6.7}, {0.0, -0.5, 7.0}, { 0.0, -0.7, 7.0}, {11.0,11.0,12.0} },	// ETPUB FIXED
{ { 0.3,-0.6, 7.5}, { 0.0, 0.6, 7.8}, { 1.5, -1.0, 7.0}, { 0.0,  0.0, 7.5}, { 0.9,  0.7, 6.8}, {12.5,12.5,12.0} },	// ETPRO b_realhead 1
{ { 0.3, 0.3, 7.0}, {-0.3, 0.8, 7.0}, { 0.0,  0.3, 6.9}, { 0.5,  0.0, 6.5}, { 0.0, -0.7, 7.0}, {11.0,11.0,12.0} },    // GENERIC realhead
{ { 0.4, 0.2, 5.7}, { 9.6, 5.5, 5.4}, { 0.6, 2.1, 5.9}, { 0.1, 1.2, 4.7}, { 0.1, -0.1, 7.1}, { 15.0, 15.0, 15.0} },              // Sil2
{ { 1.10, 0.65, 6.65}, { 0.70, 0.52, 6.40}, { 0.60, 0.50, 5.60}, { 0.80, 0.52, 6.10}, { 0.70, 0.52, 6.40}, { 6.0, 6.0, 3.0} },	// icebreak
{ { -1.211, -1.3, 5.426}, { 0.1, -0.1, 6.1}, { 0.08, 0.10, 7.0}, { 0.05, 1.2, 5.97}, { 0.01, -0.02, 4.99}, { 6.996, 7.197, 6.928} },	// Mesa
{ { 1.11, 1.3, 4.3}, { -0.58, 1.31, 2.19}, { 0.208, 0.417, 3.611}, { 0.0, 0.0, 4.097}, { 0.27, 1.18, 4.16}, { 0.729, 0.833, 5.448} },	// new
{ { -1.12, 0.20, 4.91}, { -1.12, 0.69, 5.28}, { -1.18, 0.374, 5.909}, { 0.202, 0.1, 6.9}, { -1.12, 0.33, 5.8}, { 15.0, 15.0, 15.0} },	// pwn
{ { -1.12, 0.20, 4.909}, { -1.12, 0.69, 5.28}, { -1.179, 0.374, 5.909}, { 0.202, 0.1, 6.9}, { -1.12, 0.33, 5.8}, { 9.68, 9.75, 9.75} },// pwn2
{ { 1.35, 1.18, 5.64}, { 1.3, 0.89, 5.6}, { 0.9, 1.1, 4.46}, { 1.3, 1.4, 5.66}, { 0.9, -1.2, 7.4}, { 15.0, 15.0, 15.0} },	// mp40
{ { 1.03, 0.49, 7.3}, { 1.03, -0.04, 6.78}, { 1.57, 0.94, 5.42}, { 0.49, 1.65, 5.69}, { 0.11, 0.12, 8.1}, { 15.0, 15.0, 15.0} },// sten
{ { 1.38, 0.74, 6.8}, { 1.38, 0.74, 6.8}, { 1.38, 0.74, 6.8}, { 1.38, 0.74, 6.8}, { 1.38, 0.74, 6.82}, { 15.0, 15.0, 15.0} },	// sniper
{ { -1.121, 0.40, 6.68}, { -1.12, 0.40, 6.68}, { 0.0, 0.0, 5.5}, { 0.0, 0.0, 5.5}, { -1.12, 0.4, 6.68}, { 15.0, 15.0, 15.0} },	// monster
{ { 0.55, -0.48, 5.62}, { 0.0, -0.14, 5.07}, { 0.42, 0.35, 5.556}, { -0.556, 0.208, 5.069}, { -0.69, -0.9, 5.9}, { 6.0, 7.0, 9.0} },//test
{ { 0.41, -0.6, 6.1}, { 0.4, -0.6, 6.1}, { 0.41, -0.6, 6.1}, { 0.41, -0.6, 6.1}, { 0.41, -0.6, 6.1}, { 15.0, 15.0, 15.0} },	// test2
{ { -1.12, 0.20, 7.2}, { -1.12, 0.69, 7.2}, { -1.18, 0.374, 7.2}, { 0.202, 0.1, 7.2}, { -1.12, 0.33, 7.2}, { 15.0, 15.0, 15.0} },	// fg

};

// seems to be the same for every mod      stand              crouch              prone         stand moving       prone moving      stand bounds      prone bounds
const static hitbox_t body_hitbox = { {0.0,  0.0, 0.0}, {0.0,  0.0, 0.0}, {0.0,  0.0, 0.0}, {0.0,  0.0, 0.0}, {0.0,  0.0, 0.0}, {24.0,24.0,36.0}, {24.0,24.0,36.0} };

hitbox_t custom_headhitbox;

#endif
