#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <deque>
#include <vector>
#include <cmath>
#include <windows.h>
#include <chrono>

using namespace std;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int nthPrime(int n) {
    int count = 0;
    int num = 1;
    while (count < n) {
        num++;
        if (isPrime(num)) count++;
    }
    return num;
}

// 1.2.1
mutex m1_2_1;
queue<int> q1_2_1;
bool flag1_2_1 = false;

void DataPreparation_1() {
    cout << "[1.2.1] Вводьте цілі числа. Для завершення введіть 0: \n";
    int val;
    while (cin >> val && val != 0) {
        unique_lock<mutex> lk(m1_2_1);
        q1_2_1.push(val);
        lk.unlock();
    }
    flag1_2_1 = true;
}

void DataProcessing_1() {
    unique_lock<mutex> lk(m1_2_1, defer_lock);
    while (!flag1_2_1) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    lk.lock();
    cout << "[1.2.1] Прості числа з уведених: ";
    while (!q1_2_1.empty()) {
        int num = q1_2_1.front();
        q1_2_1.pop();
        if (isPrime(num)) cout << num << " ";
    }
    cout << endl;
}

void task1() {
    thread t1(DataPreparation_1);
    t1.detach();
    DataProcessing_1();
}

// 1.2.2
mutex m1_2_2;
condition_variable cv1_2_2;
int global_i_1_2_2 = 0;

void Waits_2(int id) {
    cout << "[1.2.2] Потік " << id << " входить у стан очікування.\n";
    unique_lock<mutex> lk(m1_2_2);
    cv1_2_2.wait(lk, [] { return global_i_1_2_2 == 1; });
    cout << "[1.2.2] Потік " << id << " завершив очікування.\n";
}

void Awake_2() {
    this_thread::sleep_for(chrono::milliseconds(100));
    cout << "[1.2.2] Awake починає повідомляти решту (notify_all).\n";
    cv1_2_2.notify_all();

    this_thread::sleep_for(chrono::milliseconds(100));
    {
        lock_guard<mutex> lk(m1_2_2);
        global_i_1_2_2 = 1;
    }
    cout << "[1.2.2] Awake встановив i=1 і повторно повідомляє (notify_all).\n";
    cv1_2_2.notify_all();
}

void task2() {
    global_i_1_2_2 = 0;
    thread t1(Waits_2, 1);
    thread t2(Waits_2, 2);
    thread t3(Waits_2, 3);
    thread aw(Awake_2);
    t1.join(); t2.join(); t3.join(); aw.join();
}

// 1.2.3
mutex m1_2_3;
condition_variable cv1_2_3;
int global_i_1_2_3 = 0;

void Waits_3(int id) {
    unique_lock<mutex> lk(m1_2_3);
    cv1_2_3.wait(lk, [] { return global_i_1_2_3 == 1; });
    cout << "[1.2.3] Повідомлення з потоку " << id << endl;
}

void Notify_3() {
    this_thread::sleep_for(chrono::milliseconds(100));
    {
        lock_guard<mutex> lk(m1_2_3);
        global_i_1_2_3 = 1;
    }
    cout << "[1.2.3] Notify пробуджує ОДИН потік (notify_one).\n";
    cv1_2_3.notify_one();
}

void task3() {
    global_i_1_2_3 = 0;
    thread t1(Waits_3, 1);
    thread t2(Waits_3, 2);
    thread t3(Waits_3, 3);
    thread nt(Notify_3);

    nt.join();
    {
        lock_guard<mutex> lk(m1_2_3);
        global_i_1_2_3 = 1;
    }
    cv1_2_3.notify_all();
    t1.join(); t2.join(); t3.join();
}

//1.2.4
mutex m1_2_4;
condition_variable cv1_2_4;
queue<int> q1_2_4;
bool finished_1_2_4 = false;

void DataPreparation_4() {
    cout << "[1.2.4] Вводьте числа (0 для стоп): \n";
    int val;
    while (cin >> val && val != 0) {
        lock_guard<mutex> lk(m1_2_4);
        q1_2_4.push(val);
    }
    {
        lock_guard<mutex> lk(m1_2_4);
        finished_1_2_4 = true;
    }
    cv1_2_4.notify_one();
}

void DataProcessing_4() {
    unique_lock<mutex> lk(m1_2_4);
    cv1_2_4.wait(lk, [] { return !q1_2_4.empty() || finished_1_2_4; });

    cout << "[1.2.4] Прості числа: ";
    while (!q1_2_4.empty()) {
        int num = q1_2_4.front();
        q1_2_4.pop();
        if (isPrime(num)) cout << num << " ";
    }
    cout << endl;
}

void task4() {
    finished_1_2_4 = false;
    while (!q1_2_4.empty()) q1_2_4.pop();

    thread t1(DataPreparation_4);
    t1.detach();
    DataProcessing_4();
}

// 1.2.5
void task5() {
    int n;
    cout << "[1.2.5] Введіть номер n для пошуку простого числа: ";
    cin >> n;

    auto start_def = chrono::high_resolution_clock::now();
    future<int> f_deferred = async(launch::deferred, nthPrime, n);
    cout << "[Deferred] Функція (sqrt) від n: " << sqrt(n) << endl;
    cout << "[Deferred] n-те просте число: " << f_deferred.get() << endl;
    auto end_def = chrono::high_resolution_clock::now();

    auto start_async = chrono::high_resolution_clock::now();
    future<int> f_async = async(launch::async, nthPrime, n);
    cout << "[Async] Функція (sin) від n: " << sin(n) << endl;
    cout << "[Async] n-те просте число: " << f_async.get() << endl;
    auto end_async = chrono::high_resolution_clock::now();

    cout << "Час Deferred: " << chrono::duration_cast<chrono::milliseconds>(end_def - start_def).count() << " ms\n";
    cout << "Час Async: " << chrono::duration_cast<chrono::milliseconds>(end_async - start_async).count() << " ms\n";
}

//1.2.6
deque<packaged_task<int()>> tasks_1_2_6;
deque<int> ns_1_2_6;
mutex m1_2_6;
condition_variable cv1_2_6;
bool done_1_2_6 = false;

void Worker_6() {
    while (true) {
        packaged_task<int()> task;
        int n_val = 0;
        {
            unique_lock<mutex> lk(m1_2_6);
            cv1_2_6.wait(lk, [] { return !tasks_1_2_6.empty() || done_1_2_6; });
            if (done_1_2_6 && tasks_1_2_6.empty()) return;

            task = move(tasks_1_2_6.front());
            tasks_1_2_6.pop_front();
            n_val = ns_1_2_6.front();
            ns_1_2_6.pop_front();
        }
        task();
    }
}

void task6() {
    done_1_2_6 = false;
    thread worker(Worker_6);
    vector<future<int>> futures;
    vector<int> req_ns;

    cout << "[1.2.6] Вводьте номери n-тих простих чисел (-1 для стоп): \n";
    int n;
    while (cin >> n && n != -1) {
        packaged_task<int()> task(bind(nthPrime, n));
        futures.push_back(task.get_future());
        req_ns.push_back(n);
        {
            lock_guard<mutex> lk(m1_2_6);
            tasks_1_2_6.push_back(move(task));
            ns_1_2_6.push_back(n);
        }
        cv1_2_6.notify_one();
    }

    {
        lock_guard<mutex> lk(m1_2_6);
        done_1_2_6 = true;
    }
    cv1_2_6.notify_one();

    for (size_t i = 0; i < futures.size(); i++) {
        cout << req_ns[i] << "-те просте число: " << futures[i].get() << endl;
    }
    worker.join();
}

// 1.2.7
void Thread1_7(int n, promise<int> p_n, promise<bool> p_bool, promise<int> p_n10) {
    p_n.set_value(nthPrime(n));
    p_bool.set_value(true);
    p_n10.set_value(nthPrime(n * 10));
}

void Thread2_7(int n, future<bool> f_bool) {
    if (f_bool.get()) {
        this_thread::sleep_for(chrono::seconds(2));
        cout << "[Потік 2] Квадратний корінь з " << n << " = " << sqrt(n) << endl;
    }
}

void task7() {
    int n;
    cout << "[1.2.7] Введіть номер n: ";
    cin >> n;

    promise<int> p_n, p_n10;
    promise<bool> p_bool;

    future<int> f_n = p_n.get_future();
    future<int> f_n10 = p_n10.get_future();
    future<bool> f_bool = p_bool.get_future();

    thread t1(Thread1_7, n, move(p_n), move(p_bool), move(p_n10));
    thread t2(Thread2_7, n, move(f_bool));

    cout << "[Головний потік] " << n << "-те просте число: " << f_n.get() << endl;
    cout << "[Головний потік] " << n * 10 << "-те просте число: " << f_n10.get() << endl;

    t1.join();
    t2.join();
}

int main() {

    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);


    int choice;
    do {
        cout << "\nЛабораторна робота №2\n";
        cout << "Оберіть завдання (1-7) або 0 для виходу: ";
        cin >> choice;
        switch (choice) {
        case 1: task1(); break;
        case 2: task2(); break;
        case 3: task3(); break;
        case 4: task4(); break;
        case 5: task5(); break;
        case 6: task6(); break;
        case 7: task7(); break;
        }
    } while (choice != 0);
    return 0;
}
