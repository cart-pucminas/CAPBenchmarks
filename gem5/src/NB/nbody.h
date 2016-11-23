/*
 * Copyright(C) 2016 Matheus Alcântara Souza <matheusalcantarasouza@gmail.com>
 * 
 * nbody.h - N-Body kernel library.
 */

#ifndef _NB_H_
#define _NB_H_
	/* TODO: Join structs in order to optimize data movements */

	/*
	 * Body (particle) with mass.
	 */
	typedef struct {
		double x, y, z;
		double mass;
	} body_t;

	/*
	 * Body (particle) with velocity (forces).
	 */
	typedef struct {
		double xold, yold, zold;
		double fx, fy, fz;
	} body_v_t;
	
	/*
	 * Compute velocities (forces)
	 */
	double compute_forces(body_t[], body_v_t[], int);

	/*
	 * Compute new positions of the bodies
	 */
	double compute_new_positions(body_t[], body_v_t[], int, double);

#endif /* _NB_H_ */