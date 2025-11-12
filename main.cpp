// Набор функций для сравнения стратегий компиляции: x86 vs Эльбрус
// флаги компиляции для Эльбруса:
// -O3             -     оптимизации компилятора
// -DNOINLINE=     -     замена вызовов функций вставками
// -ffast          -     оптимизация конвееризации циклов и пр.

int simple(int a, int b, int c)
{
    int x = a * b + c<<3;
    int y = b * c + a<<4;
return x + y;
}



double inlined(double x, double y)
{
double r;
if ( x > y )
r = ( x - y );
else
r = ( x + y );
return r;
}

void g(double *p, int n)
{
int i;
for (i=0; i<n; i++)
p[i]= inlined(p[i],p[i+1]);
return;
}

double conv(double s, double *a, int N)
{
    int i = 0;
do
{
s += 1.0/(a[i]*a[i] + 1.0);
}
while (i++ < N);
return s;
}







////////////////////// Дополнительные примеры

// 1. Цепочка зависимостей — последовательное исполнение
double dependent_chain(double x, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i)
        x = x * 1.000001 + 0.000001;
    return x;
}

// 2. Независимые операции
void independent_ops(double* a, double* b, double* c, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i)
    {
        a[i] = b[i] + 1.1;
        c[i] = b[i] * 2.2;
    }
}

// 3. Смешанные зависимости — частичный ILP
void mixed_dependencies(double* a, double* b, unsigned long n)
{
    double t1, t2;
    for (unsigned long i = 0; i < n; ++i)
    {
        t1 = a[i] * 1.001;
        t2 = t1 + b[i];
        a[i] = t2 * t1;
    }
}

// 4. Ветвления — непредсказуемые условия
int unpredictable_branch(const int* x, unsigned long n)
{
    int s = 0;
    for (unsigned long i = 0; i < n; ++i)
    {
        if (x[i] & 1)
            s += x[i];
        else
            s -= x[i];
    }
    return s;
}

// 5. Предсказуемое ветвление
int predictable_branch(unsigned long n)
{
    int s = 0;
    for (unsigned long i = 0; i < n; ++i)
    {
        if (i < n / 2)
            s += i;
        else
            s -= i;
    }
    return s;
}

// 6. Ручной разворот цикла — проверка unroll и упаковки
void unrolled_example(double* x, double* y, unsigned long n)
{
    for (unsigned long i = 0; i < n; i += 4)
    {
        x[i] = y[i] * 1.1;
        x[i + 1] = y[i + 1] * 1.2;
        x[i + 2] = y[i + 2] * 1.3;
        x[i + 3] = y[i + 3] * 1.4;
    }
}

// 7. Inline vs обычный вызов (стоит отключить автоматические вставки)
inline double inline_calc(double x)
{
    return x * 1.00001 + x * x;
}

double call_calc(double x)
{
    return x * 1.00001 + x * x;
}

void inline_vs_call(double* in, double* out1, double* out2, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i)
        out1[i] = inline_calc(in[i]);

    for (unsigned long i = 0; i < n; ++i)
        out2[i] = call_calc(in[i]);
}

// 8. Доступ к структурам против массивов (AoS vs SoA)
struct Vec3 { double x, y, z; };

void struct_access(Vec3* v, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i)
        v[i].x = v[i].y * v[i].z + 1.0;
}

void array_access(double* x, double* y, double* z, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i)
        x[i] = y[i] * z[i] + 1.0;
}

// 9. Указательная итерация — постинкремент адресов
void pointer_iteration(double* x, double* y, double* z, unsigned long n)
{
    double* px = x;
    double* py = y;
    double* pz = z;
    for (unsigned long i = 0; i < n; ++i)
    {
        *px = (*py) * (*pz);
        ++px; ++py; ++pz;
    }
}




int main()
{
    return 0;
}