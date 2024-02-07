static int global = 88;

extern "C" int GetSum(int a, int b)
{
    return a + b + global;
}
