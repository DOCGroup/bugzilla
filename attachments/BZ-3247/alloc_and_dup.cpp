#include "ace/Message_Block.h"

int main(int argc, char* argv[])
   {
   if(argc != 3)
      return printf("Usage %s cnt size\n", argv[0]);

   int how_many = atoi(argv[1]);
   int sz = atoi(argv[2]);

   printf("%d blocks %d bytes each\n", how_many, sz);

   char blk[sz];

   for(size_t j = 0 ; j != sz; j++)
      blk[j] = 'A';
   blk[sz-1] = 0;

   ACE_Message_Block* mb_top = new ACE_Message_Block();

   ACE_Message_Block* mb = mb_top;

   for(size_t j = 0; j != how_many; ++j)
      {
      ACE_Message_Block* next = new ACE_Message_Block(blk, sz);

      next->copy(blk);

      mb->cont(next);
      mb = next;
      }

   ACE_Message_Block* mb_dup = mb_top->duplicate();

   printf("dup'ed: %p %d %d\n",
          mb_dup, mb_dup->total_size(), mb_dup->total_length());
   }
