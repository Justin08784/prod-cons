# prod-cons


Justin, about this one. Please create 3 different implementation approach of multithreaded producer-consumer in C/C++. Put each version in separate file with its own main function. In the end, we will pick the best (most optimized/fastest) version. You also need to understand and explain to me why certain approach is better than others. Be mindful about the type of data and how do you reuse a pointer. 

I don't care what the consumer does, it just need to consume the queue. The only important details:
The producer is single threaded. The consumer is multi threaded.


- 1P - 1B - NC 


Approach 1:
- Fixed queue; use bitmap style (group 2 adjacent bits to indicate 4 states per 
"element" EMPTY, FULL, EMPTYING, FILLING) utarray with 
- Allows use of bitmap to keep track of available data

Approach 2:
- Dynamic queue; use list of pointers