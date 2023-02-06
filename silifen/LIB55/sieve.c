/* eratosthenes sieve benchmark, Silicon Fen Software 30/1/85 */
/* approx 30 seconds on 1MHz 6809 for 10 iterations */


#include "stdio.h"

char flags[8191];       /* globals zeroed automatically */
int i,prime,k,count,iter;

main()
        {
        printf("\n10 iterations\n");
        while(iter++ < 10)
                {
                count = 0;
                fill(flags,1,8191);
                for(i = 0;i<8191;++i)
                        {
                        if (flags[i])
                                {
                                k = i + ( prime = i + i + 3 );
                                while(k<8191)
                                        {
                                        flags[k] = 0;
                                        k = k + prime;
                                        }
                                ++count;
                                }
                        }
                }
        printf("%d primes found\n", count);
        }   
