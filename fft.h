#include <fstream>
#include <complex>
using namespace std;
class FFT{
    private:
        int log2(int N);
        int check(int n);
        int reverse(int N, int n);
        void ordina(complex<float>* f1, int N);
        void transform(complex<float>* f, int N);
    public:
        FFT(complex<float>* f, int N, float d);
};
