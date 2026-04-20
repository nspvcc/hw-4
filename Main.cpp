#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std;


// Функція для обчислення кількості кроків за гіпотезою Колатца
uint64_t calculate_collatz_steps(uint64_t n) {
    uint64_t steps = 0;

    // Еволюція числа, поки воно не виродиться в 1
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;       // Якщо парне
        }
        else {
            n = 3 * n + 1;   // Якщо непарне
        }
        steps++;
    }
    return steps;
}


// Робоча функція для кожного потоку
void worker_thread(atomic<uint64_t>& current_number,
    uint64_t max_number,
    atomic<uint64_t>& total_steps) {

    uint64_t local_total_steps = 0; // Локальний лічильник кроків для цього потоку

    while (true) {
        // Беремо наступне число і збільшуємо спільний лічильник на 1
        // fetch_add - це атомарна (безпечна для потоків) операція
        uint64_t n = current_number.fetch_add(1);

        // Якщо ми вийшли за межі 10 000 000, потік завершує роботу
        if (n > max_number) {
            break;
        }

        // Обчислюємо кроки і додаємо до локальної суми
        local_total_steps += calculate_collatz_steps(n);
    }

    // Після завершення всіх обчислень у цьому потоці, 
    // додаємо локальну суму до глобальної суми кроків
    total_steps.fetch_add(local_total_steps);
}

// Головна функція
int main() {
    // 1. Налаштування параметрів
    const uint64_t MAX_NUMBER = 10000000; // Кількість чисел (10 млн)

    // Можна задати кількість потоків вручну, наприклад: unsigned int num_threads = 4;
    // Але ми за замовчуванням беремо максимальну кількість апаратних потоків твого процесора:
    unsigned int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 2; // Резервне значення, якщо систему не розпізнано

    cout << "We start the calculations for " << MAX_NUMBER << " numbers." << endl;
    cout << "Number of threads: " << num_threads << endl;

    // 2. Ініціалізація атомарних змінних
    // current_number починається з 1, це наша "роздача задач"
    atomic<uint64_t> current_number(1);
    atomic<uint64_t> total_steps(0);

    // 3. Фіксація часу початку обчислень
    auto start_time = chrono::high_resolution_clock::now();

    // 4. Створення та запуск потоків
    vector<thread> threads;
    for (unsigned int i = 0; i < num_threads; ++i) {
        // Кожен потік запускає функцію worker_thread з переданими параметрами
        threads.emplace_back(worker_thread, ref(current_number), MAX_NUMBER, ref(total_steps));
    }

    // 5. Очікування завершення роботи всіх потоків
    for (auto& t : threads) {
        t.join(); // Головний потік чекає, поки кожен робочий потік не закінчить роботу
    }

    // 6. Фіксація часу завершення та обчислення результатів
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time; // Різниця в секундах

    // Обчислюємо середню кількість кроків
    double average_steps = static_cast<double>(total_steps.load()) / MAX_NUMBER;

    // 7. Виведення результатів
    cout << "========================================" << endl;
    cout << "Calculation completed successfully!" << endl;
    cout << "Total execution time: " << elapsed.count() << " seconds." << endl;
    cout << "Total number of steps: " << total_steps.load() << endl;
    cout << "Average number of steps per number: " << average_steps << endl;

    return 0;
}