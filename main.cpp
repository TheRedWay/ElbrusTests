#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include <string>
using namespace std;
using clk = chrono::high_resolution_clock;

/*
 * ===========================================================
 *     BENCHMARK: x86 vs Ёльбрус (VLIW)
 * ===========================================================
 *   ѕровер€ютс€ разные аспекты архитектуры:
 *   - арифметика и ILP
 *   - конвейеризаци€ и разворот циклов
 *   - ветвлени€ и предикаты
 *   - кеш и stride
 *   - предвыборка
 *   - матричное умножение
 *   - зависимости
 *   - доступ к структурам
 *   - независимые операции
 *
 *   ¬ыводит результаты в консоль и сохран€ет в results.csv:
 *       “ест;¬рем€(сек);ќперации;ћлн.оп/с
 * ===========================================================
 */

struct Result {
    string name;
    double seconds;
    double operations;
};

vector<Result> results;

//--------------------------------------------------------------
// ¬спомогательна€ функци€ записи результатов
void save_result(const string& name, double seconds, double operations) {
    results.push_back({ name, seconds, operations });
    double mops = (operations / seconds) / 1e6;
    cout.setf(ios::fixed); cout.precision(3);
    cout << name << ": " << seconds << " с, "
        << mops << " млн оп/с\n";
}

//--------------------------------------------------------------
// 1. ALU TEST Ч чиста€ арифметика
void test_arithmetic(size_t N) {
    double a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++) {
        a = a + b; b = b * c; c = c + d; d = d * e;
        e = e + f; f = f * g; g = g + h; h = h * a;
    }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[1] ALU арифметика", dt, N * 8);
}

//--------------------------------------------------------------
// 2. PIPELINE / UNROLL TEST Ч программна€ конвейеризаци€
void test_pipeline(size_t N) {
    vector<float> a(N, 1), b(N, 2), c(N);
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i += 4) {
        c[i] = a[i] * b[i];
        c[i + 1] = a[i + 1] * b[i + 1];
        c[i + 2] = a[i + 2] * b[i + 2];
        c[i + 3] = a[i + 3] * b[i + 3];
    }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[2] Pipeline unroll x4", dt, N);
}

//--------------------------------------------------------------
// 3. BRANCH TEST Ч ветвлени€ и предикатное исполнение
void test_branches(size_t N) {
    vector<int> data(N);
    for (size_t i = 0; i < N; i++) data[i] = i % 100;
    volatile int sum = 0;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++) {
        if (data[i] < 50) sum += data[i];
        else sum -= data[i];
    }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[3] Branch ветвлени€", dt, N);
}

//--------------------------------------------------------------
// 4. MEMORY STRIDE TEST Ч работа с пам€тью
void test_memory(size_t N, size_t stride) {
    vector<int> arr(N, 1);
    volatile int s = 0;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i += stride)
        s += arr[i];
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[4] Memory stride=" + to_string(stride), dt, N / stride);
}

//--------------------------------------------------------------
// 5. PREFETCH TEST Ч ручна€ предвыборка
void test_prefetch(size_t N) {
    vector<int> arr(N, 1);
    volatile int s = 0;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++) {
        __builtin_prefetch(&arr[i + 16], 0, 1);
        s += arr[i];
    }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[5] Prefetch ручна€", dt, N);
}

//--------------------------------------------------------------
// 6. MATRIX MULTIPLICATION Ч проверка кеша
void test_matmul(int N) {
    vector<vector<float>> A(N, vector<float>(N, 1)),
        B(N, vector<float>(N, 1)),
        C(N, vector<float>(N, 0));
    auto t0 = clk::now();
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            float s = 0;
            for (int k = 0; k < N; k++)
                s += A[i][k] * B[k][j];
            C[i][j] = s;
        }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[6] Matrix " + to_string(N) + "x" + to_string(N), dt, (double)N * N * N);
}

//--------------------------------------------------------------
// 7. DATA DEPENDENCY TEST Ч зависимости по данным
void test_dependency(size_t N) {
    volatile int x = 1;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++)
        x = (x * 3 + 7) % 1000003;
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[7] Dependency зависимости", dt, N);
}

//--------------------------------------------------------------
// 8. STRUCT vs ARRAY Ч эффективность доступа
struct P { int x, y, z; };
void test_structs(size_t N) {
    vector<P> v(N);
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++)
        v[i].x += v[i].y + v[i].z;
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[8] Struct структуры", dt, N);
}

//--------------------------------------------------------------
// 9. INDEPENDENT OPS TEST Ч независимые операции
void test_independent(size_t N) {
    double a1 = 1, a2 = 2, a3 = 3, a4 = 4;
    auto t0 = clk::now();
    for (size_t i = 0; i < N; i++) {
        a1 *= 1.000001; a2 *= 1.000002;
        a3 *= 1.000003; a4 *= 1.000004;
    }
    auto t1 = clk::now();
    double dt = chrono::duration<double>(t1 - t0).count();
    save_result("[9] Independent независимые", dt, N * 4);
}

//--------------------------------------------------------------
// —охранение всех результатов в CSV
void save_csv(const string& filename) {
    ofstream file(filename);
    file << "“ест;¬рем€(сек);ќперации;ћлн.оп/с\n";
    for (auto& r : results) {
        double mops = (r.operations / r.seconds) / 1e6;
        file << r.name << ";" << r.seconds << ";" << r.operations << ";" << mops << "\n";
    }
    cout << "\n–езультаты сохранены в " << filename << endl;
}

//--------------------------------------------------------------
// √лавна€ функци€
//--------------------------------------------------------------
int main() {
    const size_t N = 50'000'000;

    cout << "==============================\n";
    cout << "  Ѕ≈Ќ„ћј–  ЁЋ№Ѕ–”— vs x86\n";
    cout << "==============================\n\n";

    test_arithmetic(N);
    test_pipeline(N);
    test_branches(N);

    cout << "\n--- “есты пам€ти ---\n";
    for (size_t s : {1, 2, 4, 8, 16, 32, 64, 128, 256})
        test_memory(N, s);

    test_prefetch(N);
    test_matmul(256);
    test_dependency(N);
    test_structs(N);
    test_independent(N);

    save_csv("results.csv");

    cout << "\n¬се тесты завершены.\n";
    return 0;
}
