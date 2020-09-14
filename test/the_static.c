#include<stdio.h> 
int counterFunction() 
{ 
  static int count = 0; 
  count++; 
  return count; 
} 

int main() 
{ 
  printf("First Counter Output = %d\n", counterFunction()); 
  printf("Second Counter Output = %d ", counterFunction()); 
  return 0; 
}