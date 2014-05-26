/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * x86/balance.c - Workload balancer.
 */

/*
 * Balances workload.
 */
void balance(int *work, int n, int k)
{
	int i, j; /* Loop indexes.   */
	
	work[0] = 0;
	
	/* Balance workload. */
	j = 0;
	for (i = 1; i < (n >> 2); i += 2)
	{
		work[i] = j;
		work[n - 1 - i] = j;
		j++;
		
		if (j == k)
			j = 0;
	}
}
