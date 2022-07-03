#include <iostream>
#include <iomanip>
void PrintKey(const uint8_t* array, uint32_t len, uint8_t line_items = 8)
{
    using namespace std;
    ios_base::fmtflags f(cout.flags());
    cout<< "Len = " << len <<endl;
    for(uint32_t it = 0;it < len; it++)
    {
        cout << setw(2) << setfill('0') << hex << (int)array[it] <<  " ";
        if ((it+1) % line_items == 0) cout <<endl;
    }
    cout.flags(f);
}