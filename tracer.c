#include "tracer.h"

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end, int type)
{
	vec3_t	diff, v1, v2, v3, v4, v5, v6;

/*	if(!type) {	// just a line
		//eth.CG_RailTrail2( ci, start, end);
		vec3_t color;
		color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 0.0;
		makeRailTrail( start, end, color);
		return;
	}

*/

	// type '1' (box)
	VectorSubtract(start, end, diff);

	VectorCopy(start, v1);
	VectorCopy(start, v2);
	VectorCopy(start, v3);
	v1[0] -= diff[0];
	v2[1] -= diff[1];
	v3[2] -= diff[2];
	eth.CG_RailTrail2( ci, start, v1);
	eth.CG_RailTrail2( ci, start, v2);
	eth.CG_RailTrail2( ci, start, v3);

	VectorCopy(end, v4);
	VectorCopy(end, v5);
	VectorCopy(end, v6);
	v4[0] += diff[0];
	v5[1] += diff[1];
	v6[2] += diff[2];
	eth.CG_RailTrail2( ci, end, v4);
	eth.CG_RailTrail2( ci, end, v5);
	eth.CG_RailTrail2( ci, end, v6);

	eth.CG_RailTrail2( ci, v2, v6);
	eth.CG_RailTrail2( ci, v6, v1);
	eth.CG_RailTrail2( ci, v1, v5);

	eth.CG_RailTrail2( ci, v2, v4);
	eth.CG_RailTrail2( ci, v4, v3);
	eth.CG_RailTrail2( ci, v3, v5);

}

/* draws expected hitbox on a player */
void debug_head_box( int player )
{
	vec3_t			cv;
	vec3_t			p;
	const hitbox_t 	*hbox;
	qboolean		moving;
	int 			eFlags;

	refEntity_t *head = &eth.entities[player].refHead;

	if( seth.value[VAR_HITBOX] == HITBOX_CUSTOM )
		hbox = &custom_headhitbox;
	else
		hbox = &head_hitboxes[(int)seth.value[VAR_HITBOX]];

	eFlags = eth.cg_entities[player].currentState->eFlags;
	if( VectorCompare(eth.cg_entities[player].currentState->pos.trDelta, vec3_origin) )
		moving = qfalse;
	else
		moving = qtrue;

	if( (eFlags & EF_PRONE) || (eFlags & EF_PRONE_MOVING) )
		VectorCopy( hbox->prone_offset, cv );
	else {
		if( !moving ){
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset, cv );
			else
				VectorCopy( hbox->stand_offset, cv );
		} else {
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset_moving, cv );
			else
				VectorCopy( hbox->stand_offset_moving, cv );
		}
	}

	/* rotate hitbox offset vector with tag (i guess this is so) */
	VectorMA( head->origin, cv[2], head->axis[2], p );
	VectorMA( p, cv[1], head->axis[1], p );
	VectorMA( p, cv[0], head->axis[0], p );

	vec3_t min,max;
	vec3_t x,y,z;

	x[0] = 1.0f; x[1] = 0.0f; x[2] = 0.0f;
	y[0] = 0.0f; y[1] = 1.0f; y[2] = 0.0f;
	z[0] = 0.0f; z[1] = 0.0f; z[2] = 1.0f;
	
	VectorMA( p, -0.5* hbox->size[0], x, min );
	VectorMA( min, -0.5* hbox->size[1], y, min );
	VectorMA( min, -0.5* hbox->size[2], z, min );

	VectorMA( p, 0.5* hbox->size[0], x, max );
	VectorMA( max, 0.5* hbox->size[1], y, max );
	VectorMA( max, 0.5* hbox->size[2], z, max );

	//printf("hitbox on {%.1f, %.1f, %.1f}\n", p[0],p[1],p[2]);

	CG_RailTrail(NULL, min, max, 1);
}

typedef struct {
	vec3_t pt;
	float  d;
} trace_point;

int distance_test(const void *elm1, const void *elm2)
{
	const trace_point *a = elm1;
	const trace_point *b = elm2;
		
	if( a->d > b->d )
		return 1;
	else if( a->d < b->d )
		return -1;
	else
		return 0;
}

int angle_test(const void *elm1, const void *elm2)
{
	const trace_point *a = elm1;
	const trace_point *b = elm2;
		
	if( a->d > b->d )
		return -1;
	else if( a->d < b->d )
		return 1;
	else
		return 0;
}


/* 	multi trace a box and return visible point
	boxes have an origin, and are always world-axis aligned
	Note: box is assumed to be centered on its origin
*/
qboolean trace_box( vec3_t box_origin, vec3_t size, vec3_t tr_origin, int skipent, boxtrace_t trtype,  vec3_t visible )
{
#define MAX_TR	16
	trace_t 	tr;
	trace_point p[MAX_TR];
	vec3_t 		x,y,z;

	if (trtype !=TRACE_BODY){
		/* center trace first */
		eth.CG_Trace(&tr, tr_origin, NULL,NULL, box_origin, skipent, MASK_SHOT );
		/* calling trap_trace() is a lot faster, but it doesn't check for clips by movers :( */
		// 	trap_CM_CapsuleTrace( &tr, tr_origin, box_origin, NULL, NULL, 0, MASK_SHOT );
		if( tr.fraction == 1.f ){
			VectorCopy( tr.endpos, visible );
			return qtrue;
		} else if( trtype == TRACE_CENTER )
			return qfalse;
	}

	if( size[0]*size[1]*size[2] < 1.0 )
		return qfalse;

	int k = 0;
	memset(p, 0, sizeof(p));

	/* Y-axis points north */
	x[0] = 1.0f; x[1] = 0.0f; x[2] = 0.0f;
	y[0] = 0.0f; y[1] = 1.0f; y[2] = 0.0f;
	z[0] = 0.0f; z[1] = 0.0f; z[2] = 1.0f;

	switch( trtype ){
		case TRACE_CONTOUR: {
			/* trace periphery - prioritize plane centers, more chance of hitting */
			VectorMA( box_origin, size[0]*0.5, x, p[k].pt ); k++;
			VectorMA( box_origin, -size[0]*0.5, x, p[k].pt ); k++;
			VectorMA( box_origin, size[1]*0.5, y, p[k].pt ); k++;
			VectorMA( box_origin, -size[0]*0.5, y, p[k].pt ); k++;
			VectorMA( box_origin, size[2]*0.5, z, p[k].pt ); k++;

			/* corners */
			VectorMA( box_origin, size[0]*0.5, x, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, z, p[k].pt );
			k++;

			VectorMA( box_origin, -size[0]*0.5, x, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, z, p[k].pt );
			k++;

			VectorMA( box_origin, size[1]*0.5, y, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, z, p[k].pt );
			k++;

			VectorMA( box_origin, -size[1]*0.5, y, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, z, p[k].pt );
			k++;

			break;
		}
		case TRACE_RANDOM_SURFACE: {
			/* get the 1, 2, or 3 plane(s) facing us */
			vec3_t dir,ndir, dr;
			vec3_t n, a, b;
			int i,j,N;
			float frac;
			VectorSubtract( eth.cg_refdef.vieworg, box_origin, dir );
			VectorCopy( dir, ndir );
			//VectorNormalize2( dir, dir );
			VectorNormalizeFast( ndir );
			for( i=0; i<6; i++ ){
				n[(i+1)%3] = n[(i+2)%3] = 0;
				n[(i % 3)] = (i > 2) ? 1.0 : -1.0;
				frac = DotProduct( n, ndir );
				if( frac > 0 ){
					/* this plane is visible, fill it up with points, we have MAX_TR to distribute
					   so allocate by visible surface area. */
					N = (int)((frac/sqrtf(3))*MAX_TR);
					/* construct orthonormal vecs */
					a[i%3] = a[(i+2)%3] = 0;
					a[((i+1) % 3)] = 1.0;
					b[i%3] = b[(i+1)%3] = 0;
					b[((i+2) % 3)] = 1.0;
					for( j=0;j<N; j++ ){
						VectorMA( box_origin, size[i % 3]*0.5, n, p[k].pt );
						VectorMA( p[k].pt, size[(i+1) % 3]*crandom()*0.5, a, p[k].pt );
						VectorMA( p[k].pt, size[(i+2) % 3]*crandom()*0.5, b, p[k].pt );

						/* sort based on angular distance from center based on our viewdir (to maximize acc) */
						VectorSubtract( eth.cg_refdef.vieworg, p[k].pt, dr );
						VectorNormalizeFast( dr );
						p[k].d = DotProduct( dr, ndir );	// we want smallest angles first so...
						k++;
					}
				}
			}
			qsort( p, MAX_TR, sizeof(trace_point), angle_test );
			break;
		}
		case TRACE_RANDOM_VOLUME: {
			/* trace random points within hbox volume */
			for( k=0; k < MAX_TR; k++ ){
				while( p[k].d < 3.0 ){
					VectorMA( p[k].pt, size[0]*crandom()*0.5, x, p[k].pt );
					VectorMA( p[k].pt, size[1]*crandom()*0.5, y, p[k].pt );
					VectorMA( p[k].pt, size[2]*crandom()*0.5, z, p[k].pt );
					p[k].d = VectorLengthSquared( p[k].pt );
				}
			}
			qsort( p, MAX_TR, sizeof(trace_point), distance_test );

			for( k=0; k < MAX_TR; k++ )
				VectorAdd( box_origin, p[k].pt, p[k].pt );

			break;
		}
		// for capsules size[0] is the radious and size[2] the height
		case TRACE_CAPSULE_VOLUME: {
			float phi, r;
			// trace random points within capsule volume
			for( k=0; k < MAX_TR; k++ ){
				phi = 2.0*M_PI*random();
				r = 0.5*random()*size[0];
				p[k].pt[0] = cosf(phi)*r;
				p[k].pt[1] = sinf(phi)*r;
				p[k].pt[2] = 0.5*crandom()*size[2];
				p[k].d = r*r+p[k].pt[2]*p[k].pt[2];
			}
			qsort( p, MAX_TR, sizeof(trace_point), distance_test );

			for( k=0; k < MAX_TR; k++ )
				VectorAdd( box_origin, p[k].pt, p[k].pt );

			break;
		}

		// for capsules size[0] is the radious and size[2] the height
		case TRACE_CAPSULE_SURFACE: {
			// trace random points within capsule volume
			vec3_t dir, dr;
			float phi;
			
			VectorSubtract( eth.cg_refdef.vieworg, box_origin, dir );
			VectorNormalizeFast( dir );

			for( k=0; k < MAX_TR; k++ ){
				phi = 2.0*M_PI*random();				
				p[k].pt[0] = cosf(phi)*0.5*size[0];
				p[k].pt[1] = sinf(phi)*0.5*size[0];
				p[k].pt[2] = 0.5*crandom()*size[2];

				VectorAdd( box_origin, p[k].pt, p[k].pt );

				VectorSubtract( eth.cg_refdef.vieworg, p[k].pt, dr );
				VectorNormalizeFast( dr );
				p[k].d = DotProduct( dr, dir );

			}
			qsort( p, MAX_TR, sizeof(trace_point), angle_test );

			break;
		}

		// First a few fixed points on the upper torso, when nothing is visible, do so random points
		case TRACE_BODY: {
			size[0] -= 0.7;
			size[1] -= 0.7;
			// create 4 fixed points on the vertices of the top square
			VectorMA( box_origin, 0.5*size[2]-5.0, z, p[0].pt );
			VectorMA( p[0].pt, 0.5*size[0], x, p[0].pt );
			VectorMA( p[0].pt, 0.5*size[1], y, p[0].pt );

			VectorMA( box_origin, 0.5*size[2]-5.0, z, p[1].pt );
			VectorMA( p[1].pt, -0.5*size[0], x, p[1].pt );
			VectorMA( p[1].pt, 0.5*size[1], y, p[1].pt );

			VectorMA( box_origin, 0.5*size[2]-5.0, z, p[2].pt );
			VectorMA( p[2].pt, -0.5*size[0], x, p[2].pt );
			VectorMA( p[2].pt, -0.5*size[1], y, p[2].pt );

			VectorMA( box_origin, 0.5*size[2]-5.0, z, p[3].pt );
			VectorMA( p[3].pt, 0.5*size[0], x, p[3].pt );
			VectorMA( p[3].pt, -0.5*size[1], y, p[3].pt );

			// if these fail to be visible (enemy behind complex object with cracks/holes
			// then create a user-requested amount of random points 
			for( k=4; k < MAX_TR-4; k++ ){
				VectorMA( box_origin, size[0]*crandom()*0.5, x, p[k].pt );
				VectorMA( p[k].pt, size[1]*crandom()*0.5, y, p[k].pt );
				VectorMA( p[k].pt, size[2]*crandom()*0.5, z, p[k].pt );
			}
			break;
		}		

		default:
			/* unknown trace type */
			return qfalse;
	}

	/* trace them all */
	for( k=0; k<MAX_TR; k++ ){
		if( VectorCompare(p[k].pt, vec3_origin) )
			continue;
		memset( &tr, 0, sizeof(tr) );
		eth.CG_Trace(&tr, tr_origin, NULL,NULL, p[k].pt, eth.cg_snap->ps.clientNum, MASK_SHOT );
		if( tr.fraction == 1.f) {
			VectorCopy( tr.endpos, visible );
			return qtrue;
		}
	}

	return qfalse;
}


/* build a head hitbox and trace its volume */
qboolean trace_head( refEntity_t *head, vec3_t hitpoint, int player )
{
	vec3_t			cv;
	vec3_t			p;
	const hitbox_t 	*hbox;
	qboolean		moving;
	int 			eFlags;
	vec3_t			vel;
	vec3_t			size;
	float			speed;
//little gay but who cares
//just won't notice it's change so reconnect or smtg
 	int serverSpeed = eth.cg_snap->ps.speed;
	if (serverSpeed <= 0) serverSpeed = 320;
	

	if( seth.value[VAR_HITBOX] == HITBOX_CUSTOM )
		hbox = &custom_headhitbox;
	else
		hbox = &head_hitboxes[(int)seth.value[VAR_HITBOX]];

	eFlags = eth.cg_entities[player].currentState->eFlags;

	VectorCopy( eth.cg_entities[player].currentState->pos.trDelta, vel );
	/* this is not really movement detector, but animation detector
		only use pos.trDelta since that handles user-intended velocities */
	if( VectorCompare(vel, vec3_origin) )
		moving = qfalse;
	else
		moving = qtrue;

	if( (eFlags & EF_PRONE) || (eFlags & EF_PRONE_MOVING) )
		VectorCopy( hbox->prone_offset, cv );
	else {
		if( !moving ){
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset, cv );
			else
				VectorCopy( hbox->stand_offset, cv );
		} else {
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset_moving, cv );
			else
				VectorCopy( hbox->stand_offset_moving, cv );
		}
	}

	/* dynamic hitbox - speed perpendicular to our viewdirection */
	speed = VectorLength(vel) - DotProduct( vel, eth.cg_refdef.viewaxis[0] );

	if( speed < 0 )
		printf("ERROR! vel={%.5f,%.5f, %.5f} dir={%.5f,%.5f, %.5f}\n", vel[0],vel[1],vel[2], eth.cg_refdef.viewaxis[0][0], eth.cg_refdef.viewaxis[0][1], eth.cg_refdef.viewaxis[0][2]);
	else {
		// lets try 12x12 boxes first, @ 320 -> 16x16 so...
		VectorCopy( hbox->size, size );
		size[0] += seth.value[VAR_BOXADD]*speed/(float)serverSpeed;
		size[1] += seth.value[VAR_BOXADD]*speed/(float)serverSpeed;
	}

	/* rotate hitbox offset vector with tag (hitboxes themselves dont rotate) */
	VectorMA( head->origin, cv[2], head->axis[2], p );
	VectorMA( p, cv[1], head->axis[1], p );
	VectorMA( p, cv[0], head->axis[0], p );

	vec3_t hit;

	/* you can play a bit with the random settings... doing randomized trace gives me best results
		only do complicated trace if it is enemy, if friend do simple center check
		note: i suggest always do center trace first, for max acc
	*/
	if( trace_box( p, size, eth.muzzle, player, IS_PLAYER_ENEMY(player) ? TRACE_MULTI : TRACE_CENTER, hit ) ){
		VectorCopy( hit, hitpoint );
		return qtrue;
	} else
		return qfalse;
}

/* build a body hitbox and trace it */
qboolean trace_body( int player, vec3_t hitpoint )
{
	vec3_t			cv;
	const hitbox_t 	*bbox;
	vec3_t			size;
	int				eFlags;

	/* for friends dont body trace, use head instead to save cpu*/
	if( !IS_PLAYER_ENEMY(player) )
		return eth.entities[player].isHeadVisible;
	
	// for now just standard, maybe later extend for mod tweaking
	bbox = &body_hitbox;
	eFlags = eth.cg_entities[player].currentState->eFlags;

	if( eFlags & EF_CROUCHING )
		VectorCopy( bbox->size2, size );
	else
		VectorCopy( bbox->size, size );
	
	VectorCopy( vec3_origin, cv );

	vec3_t start,hit;
	VectorCopy( eth.entities[player].origin, start );
	VectorAdd( start, cv, start );

	if( trace_box( start, size, eth.muzzle, player, TRACE_MULTI, hit ) ){
		VectorCopy( hit, hitpoint );
		return qtrue;
	} else
		return qfalse;
}
