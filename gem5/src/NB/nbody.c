#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "nbody.h"

/* TODO: Implement tri-dimensional calculations. Using 2D simulation */

double compute_forces(body_t bodies[], body_v_t bodiesV[], int nbodies) {
	double max_f, rx, ry, r, fx, fy, rmin;
	int i, j;
	max_f = 0.0;

	#pragma omp parallel for private(j, rx, ry, r, fx, fy) shared(max_f) schedule(static)
	for (i=0; i<nbodies; i++) {
		rmin = 100.0;
		fx   = 0.0;
		fy   = 0.0;
		for (j=0; j<nbodies; j++) {
			if(i==j)
				continue;
			rx = bodies[i].x - bodies[j].x;
			ry = bodies[i].y - bodies[j].y;
			r  = rx * rx + ry * ry;

			if (r < rmin)
				rmin = r;
			r  = r * sqrt(r);
			fx -= bodies[j].mass * rx / r;
			fy -= bodies[j].mass * ry / r;
		}
		bodiesV[i].fx += fx;
		bodiesV[i].fy += fy;
		fx = sqrt(fx*fx + fy*fy)/rmin;
		if (fx > max_f)
			max_f = fx;
	}
	return max_f;
}

double compute_new_positions(body_t bodies[], body_v_t bodiesV[], int nbodies, double max_f) {
	int i;
	double a0, a1, a2, xi, yi;
	static double dt_old = 0.001, dt = 0.001;
	double dt_new;
	a0 = 2.0 / (dt * (dt + dt_old));
	a2 = 2.0 / (dt_old * (dt + dt_old));
	a1 = -(a0 + a2);

	#pragma omp parallel for private(xi, yi) shared(a0, a1, a2) schedule(static)
	for (i=0; i<nbodies; i++) {
		xi = bodies[i].x;
		yi = bodies[i].y;
		bodies[i].x = (bodiesV[i].fx - a1 * xi - a2 * bodiesV[i].xold) / a0;
		bodies[i].y = (bodiesV[i].fy - a1 * yi - a2 * bodiesV[i].yold) / a0;
		bodiesV[i].xold = xi;
		bodiesV[i].yold = yi;
		bodiesV[i].fx = 0;
		bodiesV[i].fy = 0;
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
