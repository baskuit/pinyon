
#include <random>

class prng
{
    std::mt19937::result_type seed;
    std::mt19937 engine;
    std::uniform_real_distribution<double> uniform_;
    std::uniform_int_distribution<uint64_t> uniform_64_;

public:
    prng() : seed(std::random_device{}()), engine(std::mt19937{seed}) {}
    prng(std::mt19937::result_type seed) : seed(seed), engine(std::mt19937{seed}) {}

    std::mt19937::result_type get_seed() const
    {
        return seed;
    }

    // Uniform random in (0, 1)
    double uniform()
    {
        return uniform_(engine);
    }

    // Random integer in [0, n)
    int random_int(int n)
    {
        return uniform_64_(engine) % n;
    }

    uint64_t uniform_64()
    {
        return uniform_64_(engine);
    }

    template <template <typename...> typename Vector, template <typename> typename Wrapper, typename T>
    int sample_pdf(const Vector<Wrapper<T>> &input, int k)
    {
        double p = uniform();
        for (int i = 0; i < k; ++i)
        {
            p -= static_cast<double>(input[i]);
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    template <typename Vector>
    int sample_pdf(const Vector &input)
    {
        double p = uniform();
        for (int i = 0; i < input.size(); ++i)
        {
            p -= static_cast<double>(input[i]);
            if (p <= 0)
            {
                return i;
            }
        }
        return 0;
    }

    void discard(size_t n)
    {
        engine.discard(n);
    }
};

#include <thread>
#include <atomic>
#include <vector>
#include <iostream>


template <typename T>
struct A : std::atomic<T>
{
    A() : std::atomic<T>{} {}
    A(T t) : std::atomic<T>{t} {}
    A(const A &other) : std::atomic<T>{other.load()} {}
    A &operator=(const A &other)
    {
        *static_cast<std::atomic<T> *>(this) = other.load();
        return *this;
    }
};

template <typename T>
using V = std::vector<A<T>>;

const int num_doubles = 32;

struct S
{
    std::vector<double> data;
    S()
    {
        data.resize(num_doubles);
    }

    void update(prng &device)
    {
        int i = device.random_int(num_doubles);
        data[i]++;
    }
};

struct Stats
{
    V<double> data;
    Stats()
    {
        data.resize(num_doubles);
    }

    void update(prng &device)
    {
        int i = device.random_int(num_doubles);
        data[i].fetch_add(1);
    }
};

const int NUM_THREADS = 8;      // Number of threads
const int NUM_ITERATIONS = 1000000; // Number of times to increment the integer

int vanillaCounter = 0;
std::mutex mutexCounter;

void f(S *stats)
{
    prng device{};
    S other;
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        mutexCounter.lock();
        other = *stats;
        mutexCounter.unlock();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        mutexCounter.lock();
        stats->update(device);
        mutexCounter.unlock();
    }
}

void g(Stats *stats)
{
    prng device{};
    Stats other;
    for (int i = 0; i < NUM_ITERATIONS; ++i)
    {
        // other = *stats;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        other.update(device);
        *stats = other;
    }
}

int main()
{

    S s;
    Stats stats;

    auto start = std::chrono::high_resolution_clock::now();
    std::thread threadsMutex[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadsMutex[i] = std::thread(f, &s);
    }
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadsMutex[i].join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "mutex " << duration << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::thread threadsAtomic[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadsAtomic[i] = std::thread(g, &stats);
    }
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadsAtomic[i].join();
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "atomic " << duration << std::endl;

    return 0;
}