#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Seq 139 seg
//4 thread 78 seg
//speedup 1,78

#define BUCKET_SIZE       65536 // 512MB em palavras
#define STRING_LENGTH     4//8 //2
#define NUM_BUCKETS_TYPE  64 // Mapeados de acordo com o primeiro caracter
#define NUM_STRINGS     201326592 // 1,5 GB em palavras //100663296 //67108864//134217728 // 200

int baldes = 0;

char strings[NUM_STRINGS][STRING_LENGTH]; /* Strings.    */

/*
 * Bucket.
 */
struct bucket
{
	int size;                                 /* Bucket size (in strings). */
	char strings[BUCKET_SIZE][STRING_LENGTH]; /* Strings.                  */
	struct bucket *next;                      /* Next bucket a the list.   */
};

/*
 * List of buckets.
 */
struct list
{
	int length;          /* List length. */
	struct bucket *head; /* Head node.   */
};

/* Lists of buckets. */
struct list *todo[NUM_BUCKETS_TYPE];
struct list *done[NUM_BUCKETS_TYPE];

/*
 * Prints an error message and exits.
 */
static void error(const char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

/*
 * Safely allocates memory.
 */
void *smalloc(size_t size)
{
	void *ptr;

	if ((ptr = malloc(size)) == NULL)
		error("cannot malloc()");

	return (ptr);
}

/*
 * Creates an empty bucket.
 */
struct bucket *bucket_create(void)
{
	struct bucket *newbucket;

	/* Create and initialize bucket. */
	newbucket = smalloc(sizeof(struct bucket));
	newbucket->size = 0;

    baldes++;

    fprintf(stderr, "baldes %d\n", baldes);

	return (newbucket);
}


/*
* Destroy the bucket
*/
void bucket_destroy(struct bucket *b)
{
	free(b);
}

/*
 * Asserts if a bucket is full.
 */
#define bucket_full(b) \
	((b)->size == BUCKET_SIZE)

/*
 * Creates an empty list of buckets.
 */
struct list *list_create(void)
{
	struct list *newlist;

	/* Create and initialize list of buckets. */
	newlist = smalloc(sizeof(struct list));
	newlist->length = 0;
	newlist->head = NULL;

	return (newlist);
}

/*
* Destroy the list
*/
void list_destroy(struct list *l)
{
	struct bucket *b;

	/* Empty list. */
	while (l->length > 0){
			b = l->head;
            l->head = b->next;
            l->length--;
            bucket_destroy(b);

	}

	free(l);


}



/*
 * Inserts a bucket in a list of buckets.
 */
void list_insert(struct list *l, struct bucket *b)
{
	b->next = l->head;
	l->head = b;
	l->length++;
}

/*
 * Adds a string to the first available bucket.
 */
void bucket_add(struct list *l, char *str)
{
	struct bucket *b;

	b = l->head;

	/* Create an empty bucket. */
	if (bucket_full(b))
	{
		b = bucket_create();
		list_insert(l, b);
	}

	/* Add string to bucket.*/
	strncpy(&b->strings[b->size++][0], str, STRING_LENGTH);
}

/*
 * Removes the first from a list of buckets.
 */

struct bucket *list_remove(struct list *l)
{
	struct bucket *b;

	/* Empty list. */
	if (l->length == 0)
		return (NULL);

	/* Remove first bucket. */
	b = l->head;
	l->head = b->next;

	/*AMANDA decrementar o tamanho da lista*/
    l->length--;

	return (b);
}

/*
 * Creates a random vector of strings.
 */
void strings_random(char strings[NUM_STRINGS][STRING_LENGTH])
{
	int i, j;

	/* Generate strings. */
	for (i = 0; i < NUM_STRINGS; i++)
	{
		for (j = 0; j < STRING_LENGTH; j++)
			strings[i][j] = rand()%64+63;
	}
}

/*
 * Distributes strings into buckets.
 */
void strings_distribute(struct list **lists, char strings[NUM_STRINGS][STRING_LENGTH])
{
	int i;     /* Loop index.  */
	int btype; /* Bucket type. */

	/* Distribute strings int buckets;. */
	for (i = 0; i < NUM_STRINGS; i++)
	{
		btype = strings[i][0]%64;
		bucket_add(lists[btype], &strings[i][0]);
	}

}

/*
 * Testing unit.
 */

 void intercalaStr( int p, int q, int r, struct bucket *b, struct bucket *aux)
{
   int i, j, k;
   i = p; j = q;
   k = 0;

   int x;

   while (i < q && j < r) {
      //if (v[i] <= v[j])
        if (strncmp(b->strings[i], b->strings[j], STRING_LENGTH) < 0)
        {
            //w[k++] = v[i++];
            for(x=0; x<STRING_LENGTH; x++){
                aux->strings[k][x] = b->strings[i][x];
            }
            k = k+1;
            i = i+1;
        }
        else{
            // w[k++] = v[j++];
            for(x=0; x<STRING_LENGTH; x++){
                aux->strings[k][x] = b->strings[j][x];
            }
            k=k+1;
            j=j+1;
        }
   }

   while (i < q){
        for(x=0; x<STRING_LENGTH; x++){
            aux->strings[k][x] = b->strings[i][x];
        }
        k= k+1;
        i = i+1;
   }

   while (j < r){
        //w[k++] = v[j++];
        for(x=0; x<STRING_LENGTH; x++){
            aux->strings[k][x] = b->strings[j][x];

        }
        k = k+1;
        j=j+1;
   }

   for (i = p; i < r; ++i)
   {
        //v[i] = w[i-p];
        for(x=0; x<STRING_LENGTH; x++){
            b->strings[i][x] = aux->strings[i-p][x];
        }
   }

}

void mergesort_Str( int n, struct bucket *buc, struct bucket *aux)
{
    int p, r;
    int b;


    for(b=1; b<n; b=b+b){

        #pragma omp parallel for private (p, r) shared(b, n, buc, aux)
        for(p =0; p < n - b; p = p + b + b){
            r = p + 2*b;
            if (r > n) r = n;
            intercalaStr( p, p+b, r, buc, aux);

        }
    }

}



int main(int argc, char **argv)
{
	int i, j;                                    /* Loop index. */

	fprintf(stderr, "Generating strings\n");

	strings_random(strings);

	fprintf(stderr, "Creating lists of buckets\n");

	/* Create lists of buckets. */
	for (i = 0; i < NUM_BUCKETS_TYPE; i++)
	{
		todo[i] = list_create();
		done[i] = list_create();
		list_insert(todo[i], bucket_create());
	}

	fprintf(stderr, "Distributing strings into buckets\n");

	strings_distribute(todo, strings);

	//Ordenar os baldes
	struct bucket *bucketOrd;

	struct bucket *aux = bucket_create();


		for (i = 0; i < NUM_BUCKETS_TYPE; i++)
	{
	   //fprintf(stderr, "Ordenar tipo %d\n", i);
	    while(todo[i]->length>0){
            bucketOrd = list_remove(todo[i]);
            mergesort_Str(bucketOrd->size, bucketOrd, aux);
            list_insert(done[i], bucketOrd);

	    }
	}


        fprintf(stderr,"Done\n");
 	   /*
 	   fprintf(stderr, "Tipos Ordenados %d\n", i);
 	   int k;
		for (i = 0; i < NUM_BUCKETS_TYPE; i++)
	{
	   fprintf(stderr, "tipo %d\n", i);
	    while(done[i]->length>0){
            bucketOrd = list_remove(done[i]);

            for(j=0; j<bucketOrd->size; j++){
                for(k=0; k<STRING_LENGTH; k++){

                    fprintf(stderr, "%c",bucketOrd->strings[j][k]);
                }
                fprintf(stderr, "  ");

            }

             fprintf(stderr, "\n\n");
	    }

	    fprintf(stderr, "\n\n\n\n");
	}
*/

    fprintf(stderr, "Destruindo as listas\n");

	for (i = 0; i < NUM_BUCKETS_TYPE; i++)
	{
        list_destroy(todo[i]);
		list_destroy(done[i]);

	}

	 fprintf(stderr, "fim\n");

	return (EXIT_SUCCESS);
}



