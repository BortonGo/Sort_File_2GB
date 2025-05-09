#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <climits>
#include <string>

using namespace std;

// ��������� ��� ���������
const size_t CHUNK_SIZE = 100 * 1024 * 1024; // 100 �� (����� ������������ � ����������� �� ��������� RAM)
const string INPUT_FILE = "large_input.dat";
const string OUTPUT_FILE = "sorted_output.dat";
const string TEMP_PREFIX = "temp_";

// ������� ��� ��������� �������� ����� � ���������� �������
void generate_large_file() {
    ofstream out(INPUT_FILE, ios::binary);
    if (!out) {
        cerr << "Cannot create input file" << endl;
        return;
    }

    const long long NUM_NUMBERS = static_cast<long long>(2LL * 1024 * 1024 * 1024) / sizeof(int);
    srand(time(nullptr));

    for (long long i = 0; i < NUM_NUMBERS; ++i) {
        int num = rand() % INT_MAX;
        out.write(reinterpret_cast<const char*>(&num), sizeof(num));
    }

    cout << "Generated " << NUM_NUMBERS << " random numbers ("
        << NUM_NUMBERS * sizeof(int) / (1024 * 1024) << " MB)" << endl;
}

// ������� ��� ������ � ���������� ������
void sort_chunks() {
    ifstream in(INPUT_FILE, ios::binary);
    if (!in) {
        cerr << "Cannot open input file" << endl;
        return;
    }

    vector<int> buffer;
    buffer.reserve(CHUNK_SIZE / sizeof(int));
    int chunk_num = 0;

    while (true) {
        int num;
        in.read(reinterpret_cast<char*>(&num), sizeof(num));

        if (in.eof()) break;

        buffer.push_back(num);

        // ���� ����� ��������, ��������� � ���������� �� ��������� ����
        if (buffer.size() * sizeof(int) >= CHUNK_SIZE) {
            sort(buffer.begin(), buffer.end());

            string temp_filename = TEMP_PREFIX + to_string(chunk_num++) + ".dat";
            ofstream temp_out(temp_filename, ios::binary);

            for (int n : buffer) {
                temp_out.write(reinterpret_cast<const char*>(&n), sizeof(n));
            }

            buffer.clear();
        }
    }

    // ��������� ���������� ����� (���� �������� ������)
    if (!buffer.empty()) {
        sort(buffer.begin(), buffer.end());

        string temp_filename = TEMP_PREFIX + to_string(chunk_num++) + ".dat";
        ofstream temp_out(temp_filename, ios::binary);

        for (int n : buffer) {
            temp_out.write(reinterpret_cast<const char*>(&n), sizeof(n));
        }
    }

    cout << "Created " << chunk_num << " sorted chunks" << endl;
}

// ��������� ��� ������� � ������������ ��������
struct HeapItem {
    int value;
    ifstream* stream;

    bool operator>(const HeapItem& other) const {
        return value > other.value;
    }
};

// ������� ��� ������� ��������������� ������
void merge_chunks() {
    vector<string> temp_files;
    priority_queue<HeapItem, vector<HeapItem>, greater<HeapItem>> min_heap;

    // ������� ��� ��������� �����
    for (int i = 0; ; ++i) {
        string filename = TEMP_PREFIX + to_string(i) + ".dat";
        ifstream test(filename, ios::binary);
        if (!test) break;
        temp_files.push_back(filename);
        test.close();
    }

    // ��������� ��� ��������� ����� � �������������� ����
    vector<ifstream> streams(temp_files.size());
    for (size_t i = 0; i < temp_files.size(); ++i) {
        streams[i].open(temp_files[i], ios::binary);
        int num;
        if (streams[i].read(reinterpret_cast<char*>(&num), sizeof(num))) {
            min_heap.push({ num, &streams[i] });
        }
    }

    ofstream out(OUTPUT_FILE, ios::binary);
    if (!out) {
        cerr << "Cannot create output file" << endl;
        return;
    }

    // �������� ���� �������
    while (!min_heap.empty()) {
        HeapItem smallest = min_heap.top();
        min_heap.pop();

        out.write(reinterpret_cast<const char*>(&smallest.value), sizeof(smallest.value));

        int next_num;
        if (smallest.stream->read(reinterpret_cast<char*>(&next_num), sizeof(next_num))) {
            min_heap.push({ next_num, smallest.stream });
        }
    }

    // ��������� ��� ������
    for (auto& stream : streams) {
        stream.close();
    }

    // ������� ��������� �����
    for (const auto& filename : temp_files) {
        remove(filename.c_str());
    }

    cout << "Merged all chunks into sorted file" << endl;
}

int main() {
    // ��� 1: ��������� �������� ����� (����� ����������������, ���� ���� ��� ����)
    generate_large_file();

    // ��� 2: ��������� �� ��������������� �����
    sort_chunks();

    // ��� 3: ������� ������ � ��������������� ����
    merge_chunks();

    cout << "Sorting completed successfully!" << endl;
    return 0;
}