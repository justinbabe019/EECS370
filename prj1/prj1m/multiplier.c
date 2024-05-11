#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(){
   int mcand=6203, mplier=1429;

   /*
   nor(a,a)=~(a|a)=~(a);
   nor(a,b)=~(a|b)=~a&~b;
   nor(~a,~b)=a&b
   */

   int dig=1, total=0, cur=mcand;

   for(int i=0;i<16;i++){
      if(nor(nor(mplier,mplier), nor(dig,dig))){//extract cur bit
         total += cur;
      }
      dig<<1;
      cur<<1;
   }
   printf("%d\n",total);

   return 0;
}