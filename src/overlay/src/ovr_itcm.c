
// https://www.geeksforgeeks.org/program-for-nth-fibonacci-number/
// Fibonacci Series using Space Optimized Method

int itcm1_fibonacci(int n)
{
  int a = 0, b = 1, c, i;

  if( n == 0)
    return a;

  for (i = 2; i <= n; i++) {
     c = a + b;
     a = b;
     b = c;
  }

  return b;
}
