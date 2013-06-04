#include <NTL/ZZ.h>
#include <NTL/vector.h>

using namespace std;
using namespace NTL;

int main()
{
   Vec<ZZ> v;
   cin >> v;

   long n = v.length();
   v.SetLength(2*n);

   long i;
   for (i = 0 ; i < n; i++)
      v[n+i] = v[n-1-i];

   cout << v << "\n";
}
