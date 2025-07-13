#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <coroutine>
#include <cstdlib>
#include <cstdio>
#include <climits>

using namespace std;
using namespace std::chrono;

struct CoroutineData {
    string filename;
    vector<int> data;
    steady_clock::time_point start_time;
    double duration = 0;
    int yield_count = 0;
    bool done = false;
};

struct coroutine {
    struct promise_type {
        coroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

struct SortGenerator {
    struct promise_type;
    using handle_type = coroutine_handle<promise_type>;
    
    struct promise_type {
        CoroutineData* data;
        SortGenerator get_return_object() {
            return SortGenerator{handle_type::from_promise(*this)};
        }
        suspend_always initial_suspend() { return {}; }
        suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    
    handle_type h_;
    SortGenerator(handle_type h) : h_(h) {}
    
    bool next() {
        if (!h_.done()) {
            h_.resume();
            return !h_.done();
        }
        return false;
    }
};

vector<int> read_numbers(const string& filename) {
    vector<int> numbers;
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        cerr << "Cannot open file: " << filename << endl;
        return numbers;
    }
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        numbers.push_back(num);
    }
    fclose(file);
    return numbers;
}

void quick_sort(vector<int>& arr) {
    if (arr.empty()) return;
    struct Range {
        int start, end;
    };
    vector<Range> stack;
    stack.push_back({0, static_cast<int>(arr.size()) - 1});
    while (!stack.empty()) {
        Range range = stack.back();
        stack.pop_back();
        
        if (range.start >= range.end) continue;
        
        int pivot = arr[range.end];
        int i = range.start - 1;
        for (int j = range.start; j < range.end; ++j) {
            if (arr[j] <= pivot) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[range.end]);
        int p = i + 1;
        stack.push_back({range.start, p - 1});
        stack.push_back({p + 1, range.end});
    }
}

SortGenerator sort_file_coroutine(CoroutineData* data) {
    data->start_time = steady_clock::now();
    data->data = read_numbers(data->filename);
    co_await suspend_always{};
    data->yield_count++;
    quick_sort(data->data);
    co_await suspend_always{};
    data->yield_count++;
    data->duration = duration_cast<duration<double>>(steady_clock::now() - data->start_time).count();
    data->done = true;
    co_return;
}
vector<int> merge_sorted_arrays(const vector<vector<int>>& arrays) {
    vector<int> result;
    if (arrays.empty()) return result;
    struct HeapItem {
        int value;
        size_t array_idx;
        size_t element_idx;
        bool operator>(const HeapItem& other) const {
            return value > other.value;
        }
    };
    vector<HeapItem> heap;
    vector<size_t> indices(arrays.size(), 0);
    for (size_t i = 0; i < arrays.size(); ++i) {
        if (!arrays[i].empty()) {
            heap.push_back({arrays[i][0], i, 0});
        }
    }
    
    make_heap(heap.begin(), heap.end(), greater<>{});
    while (!heap.empty()) {
        pop_heap(heap.begin(), heap.end(), greater<>{});
        HeapItem smallest = heap.back();
        heap.pop_back();
        result.push_back(smallest.value);
        size_t next_idx = smallest.element_idx + 1;
        if (next_idx < arrays[smallest.array_idx].size()) {
            heap.push_back({arrays[smallest.array_idx][next_idx], smallest.array_idx, next_idx});
            push_heap(heap.begin(), heap.end(), greater<>{});
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " file1 [file2 ...]" << endl;
        return 1;
    }
    auto program_start = steady_clock::now();
    vector<CoroutineData> coroutines_data(argc - 1);
    vector<SortGenerator> coroutines;
    for (int i = 1; i < argc; ++i) {
        coroutines_data[i-1].filename = argv[i];
        coroutines.push_back(sort_file_coroutine(&coroutines_data[i-1]));
    }
    
    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (size_t i = 0; i < coroutines.size(); ++i) {
            if (!coroutines_data[i].done) {
                coroutines[i].next();
                all_done = false;
            }
        }
    }
    
    vector<vector<int>> sorted_arrays;
    for (auto& data : coroutines_data) {
        sorted_arrays.push_back(move(data.data));
    }
    
    // Слияние отсортированных массивов
    auto merge_start = steady_clock::now();
    vector<int> merged = merge_sorted_arrays(sorted_arrays);
    double merge_time = duration_cast<duration<double>>(steady_clock::now() - merge_start).count();
    
    // Вывод результатов
    double total_time = duration_cast<duration<double>>(steady_clock::now() - program_start).count();
    
    cout << "Total program time: " << total_time << " seconds" << endl;
    cout << "Merge time: " << merge_time << " seconds" << endl << endl;
    
    cout << "Coroutine statistics:" << endl;
    for (size_t i = 0; i < coroutines_data.size(); ++i) {
        cout << "File: " << coroutines_data[i].filename << endl;
        cout << "  Execution time: " << coroutines_data[i].duration << " seconds" << endl;
        cout << "  Yield count: " << coroutines_data[i].yield_count << endl;
        cout << "  Numbers sorted: " << coroutines_data[i].data.size() << endl << endl;
    }
    
    // Вывод первых 20 чисел для проверки (можно закомментировать для больших файлов)
    cout << "First 20 numbers of merged result:" << endl;
    for (int i = 0; i < 20 && i < merged.size(); ++i) {
        cout << merged[i] << " ";
    }
    cout << endl;
    
    return 0;
}