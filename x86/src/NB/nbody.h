/*
 * Copyright(C) 2016 Matheus Alcântara Souza <matheusalcantarasouza@gmail.com>
 * 
 * nbody.h - N-Body kernel library.
 */

#ifndef _NB_H_
#define _NB_H_
	/*
	 * Body (particle) with mass.
	 */
	typedef struct {
		double x, y, z;
		double mass;
		double xold, yold, zold;
		double fx, fy, fz;
	} body_t;
	
	/*
	 * Compute velocities (forces)
	 */
	double compute_forces(body_t[], int);

	/*
	 * Compute new positions of the bodies
	 */
	double compute_new_positions(body_t[], int, double);

#endif /* _NB_H_ */
