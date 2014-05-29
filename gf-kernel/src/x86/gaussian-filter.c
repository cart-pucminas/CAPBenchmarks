/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

#include <windows.h>

#define IMG_SIZE	8192
#define MASK_SIZE	7
#define SD 			0.8

#define PI 	3.14159265359
#define E	2.71828182845904

static unsigned char img[IMG_SIZE][IMG_SIZE];
static double mask[MASK_SIZE][MASK_SIZE];

typedef struct
{
    LARGE_INTEGER ini;
    LARGE_INTEGER fim;
} crono;

void tique( crono * c )
{
    QueryPerformanceCounter ( &c->ini );
}

double paraSegundos ( LARGE_INTEGER * L) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency ) ;
    return ((double)L->QuadPart /(double)frequency.QuadPart);
}

void taque ( crono * c )
{
    QueryPerformanceCounter ( &c->fim );
}

double getTempo ( crono * c )
{
    LARGE_INTEGER tempo;
    tempo.QuadPart = c->fim.QuadPart - c->ini.QuadPart;
    return paraSegundos ( &tempo );
}


void generate_mask ()
{
	double first = 1.0f / (2.0f * PI * SD * SD);
	int half = 	MASK_SIZE / 2;
	int i, j;

	double total = 0;

	for ( i = -half ; i <= half ; i++ )
	{
		for ( j = -half ; j <= half ; j++ )
		{
			double sec = (double)- ( (double)( i*i + j*j ) / 2.0 * SD * SD );
			sec = pow ( E, sec );

			mask[i + half][j + half] = first * sec;

			total += mask[i + half][j + half];
		}
	}

	for ( i = 0 ; i < MASK_SIZE ; i++ )
	{
		for ( j = 0 ; j < MASK_SIZE ; j++ )
		{
			mask[i][j] = mask[i][j] / total;
		}
	}
}

void filter ()
{
	double new_element = 0.0;
	int half = MASK_SIZE / 2;

	int i = 0, j = 0, cont = 0;
	int imgI, imgJ, maskI, maskJ;

	#pragma omp parallel //shared(cont)
	{
        #pragma omp for collapse(2)
        for ( imgI = 0 ; imgI < IMG_SIZE ; imgI++ )
        {
            for ( imgJ = 0 ; imgJ < IMG_SIZE ; imgJ++ )
            {
                //printf("Thread %d fazendo i=%d, j=%d\n", omp_get_thread_num(), imgI, imgJ);
                for ( maskI = 0 ; maskI < MASK_SIZE ; maskI++ )
                {
                    for ( maskJ = 0 ; maskJ < MASK_SIZE ; maskJ++ )
                    {
                        i = (imgI - half < 0) ? IMG_SIZE - 1 - maskI : imgI - half;
                        j = (imgJ - half < 0) ? IMG_SIZE - 1 - maskJ : imgJ - half;

                        new_element += img[i][j] * mask[maskI][maskJ];
                    }
                }
                //cont++;
                img[imgI][imgJ] = (new_element > 255) ? 255 : (int)new_element;
                new_element = 0.0;
            }
        }
	} // parallel
}



int main()
{
	crono c;
	double ttg = 0.0, ttf = 0.0;
    int i;

    omp_set_num_threads(1);
    printf("Resultados 1 thread:\n\n");

    for (i = 0 ; i < 15 ; i++)
    {
        tique(&c);
            generate_image();
        taque(&c);
        ttg += getTempo(&c);
        printf("Tempo de geracao da imagem: %.5f\n", getTempo(&c));

        generate_mask();

        tique(&c);
            filter();
        taque(&c);
        ttf += getTempo(&c);
        printf("Tempo de filtragem: %.5f\n\n", getTempo(&c));
    }

    printf("Tempo medio de geracao apos %d iteracoes: %.5f\n", i, ttg / (double)i );
    printf("Tempo medio de filtragem apos %d iteracoes: %.5f\n\n*****\n\n", i, ttf / (double)i );

    ttg = 0.0;
    ttf = 0.0;




    omp_set_num_threads(2);
    printf("Resultados 2 threads:\n\n");

    for (i = 0 ; i < 15 ; i++)
    {
        tique(&c);
            generate_image();
        taque(&c);
        ttg += getTempo(&c);
        printf("Tempo de geracao da imagem: %.5f\n", getTempo(&c));

        generate_mask();

        tique(&c);
            filter();
        taque(&c);
        ttf += getTempo(&c);
        printf("Tempo de filtragem: %.5f\n\n", getTempo(&c));
    }

    printf("Tempo medio de geracao apos %d iteracoes: %.5f\n", i, ttg / (double)i );
    printf("Tempo medio de filtragem apos %d iteracoes: %.5f\n\n*****\n\n", i, ttf / (double)i );

    ttg = 0.0;
    ttf = 0.0;






    omp_set_num_threads(4);
    printf("Resultados 4 threads:\n\n");

    for (i = 0 ; i < 15 ; i++)
    {
        tique(&c);
            generate_image();
        taque(&c);
        ttg += getTempo(&c);
        printf("Tempo de geracao da imagem: %.5f\n", getTempo(&c));

        generate_mask();

        tique(&c);
            filter();
        taque(&c);
        ttf += getTempo(&c);
        printf("Tempo de filtragem: %.5f\n\n", getTempo(&c));
    }

    printf("Tempo medio de geracao apos %d iteracoes: %.5f\n", i, ttg / (double)i );
    printf("Tempo medio de filtragem apos %d iteracoes: %.5f\n\n*****\n\n", i, ttf / (double)i );

    ttg = 0.0;
    ttf = 0.0;









    omp_set_num_threads(8);
    printf("Resultados 8 threads:\n\n");

    for (i = 0 ; i < 15 ; i++)
    {
        tique(&c);
            generate_image();
        taque(&c);
        ttg += getTempo(&c);
        printf("Tempo de geracao da imagem: %.5f\n", getTempo(&c));

        generate_mask();

        tique(&c);
            filter();
        taque(&c);
        ttf += getTempo(&c);
        printf("Tempo de filtragem: %.5f\n\n", getTempo(&c));
    }

    printf("Tempo medio de geracao apos %d iteracoes: %.5f\n", i, ttg / (double)i );
    printf("Tempo medio de filtragem apos %d iteracoes: %.5f\n", i, ttf / (double)i );

    ttg = 0.0;
    ttf = 0.0;


	return 0;
}
