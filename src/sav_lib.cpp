static int global = 0;

//#pragma data_seg(".shared") -- as i understood this is only necessary if sharing between processes
// To share between processes it needs the segment to have shared permissions.
// Other than that it just needs write/read permissions. Both of which are already included in .data segment.
// https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2008/sf9b18xk(v=vs.90)
// https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2008/h90dkhs0(v=vs.90)?redirectedfrom=MSDN
// dumpbin /headers ...; add /Fmmap_name.map to compiler options to see in which segment what is.
__declspec(dllexport) int anotherVar = 0;
//#pragma data_seg()

extern "C" void SetGlobal(int a)
{
    global = a;
}

extern "C" int GetSum(int a, int b)
{
    return a + b + global;
}

extern "C" void SetAnotherVar(int a)
{
    anotherVar = a;
}

extern "C" int GetAnotherVar()
{
    return anotherVar;
}
    
