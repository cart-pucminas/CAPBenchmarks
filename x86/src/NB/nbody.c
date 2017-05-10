#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "nbody.h"

double compute_forces(body_t bodies[], int nbodies) {
	double max_f, rx, ry, rz, r, fx, fy, fz, rmin;
	int i, j;
	max_f = 0.0;

	#pragma omp parallel for private(j, rx, ry, rz, r, fx, fy, fz) shared(max_f) schedule(static)
	for (i=0; i<nbodies; i++) {
		rmin = 100.0;
		fx   = 0.0;
		fy   = 0.0;
		fz   = 0.0;
		for (j=0; j<nbodies; j++) {
			if(i==j)
				continue;
			rx = bodies[i].x - bodies[j].x;
			ry = bodies[i].y - bodies[j].y;
			rz = bodies[i].z - bodies[j].z;
			r  = rx*rx + ry*ry + rz*rz;

			if (r < rmin)
				rmin = r;
			r  = r * sqrt(r);
			fx -= bodies[j].mass * rx / r;
			fy -= bodies[j].mass * ry / r;
			fz -= bodies[j].mass * rz / r;
		}
		bodies[i].fx += fx;
		bodies[i].fy += fy;
		bodies[i].fz += fz;
		fx = sqrt(fx*fx + fy*fy + fz*fz)/rmin;
		if (fx > max_f)
			max_f = fx;
	}
	return max_f;
}

double compute_new_positions(body_t bodies[], int nbodies, double max_f) {
	int i;
	double a0, a1, a2, xi, yi, zi;
	static double dt_old = 0.001, dt = 0.001;
	double dt_new;
	a0 = 2.0 / (dt * (dt + dt_old));
	a2 = 2.0 / (dt_old * (dt + dt_old));
	a1 = -(a0 + a2);

	#pragma omp parallel for private(xi, yi, zi)  shared(a0, a1, a2) schedule(static)
	for (i=0; i<nbodies; i++) {
		xi = bodies[i].x;
		yi = bodies[i].y;
		zi = bodies[i].z;
		bodies[i].x = (bodies[i].fx - a1 * xi - a2 * bodies[i].xold) / a0;
		bodies[i].y = (bodies[i].fy - a1 * yi - a2 * bodies[i].yold) / a0;
		bodies[i].z = (bodies[i].fz - a1 * zi - a2 * bodies[i].zold) / a0;
		bodies[i].xold = xi;
		bodies[i].yold = yi;
		bodies[i].zold = zi;
		bodies[i].fx = 0.0;
		bodies[i].fy = 0.0;
		bodies[i].fz = 0.0;
	}
	dt_new = 1.0/sqrt(max_f);
	
	if (dt_new < 1.0e-6)
		dt_new = 1.0e-6;
	
	if (dt_new < dt) {
		dt_old = dt;
		dt = dt_new;
	}
	else if (dt_new > 4.0 * dt) {
		dt_old = dt;
		dt *= 2.0;
	}
	return dt_old;
}
