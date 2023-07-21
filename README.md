# prod-cons

All tests were run with the following settings:
- #define PROD_LIMIT 1000 // max # of production cycles
- #define CONS_TIME 10000 // in microseconds
- #define PROD_TIME 1000 // in microseconds
- size_t num_producers = 1;
- size_t num_consumers = 10;

Approach 1:
- Dynamically allocated buffer blocks
- Free space management through queue of free blocks
- Time: 1.054421866s

Approach 2:
- Pre-allocated buffer blocks
- Free space management through array of free blocks
- Time: 1.064258219s

Approach 3:
- Pre-allocated buffer blocks
- Free space management through (pseudo-)bit array of free blocks
- Time: 1.064304521s