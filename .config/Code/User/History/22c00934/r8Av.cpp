#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <random>

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10000);

    std::ofstream outfile("nums2.txt");
    if (!outfile) {
        std::cerr << "Не удалось открыть файл для записи!" << std::endl;
        return 1;
    }
    for (int i = 0; i < 40000; ++i) {
        outfile << dist(gen) << " ";
        if ((i + 1) % 20 == 0) {
            outfile << "\n";
        }
    }
    outfile.close();
    std::cout << "Файл nums1.txt успешно создан с 40000 случайными числами." << std::endl;
    return 0;
}