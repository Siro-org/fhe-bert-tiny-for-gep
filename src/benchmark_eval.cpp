#include <iostream>
#include "FHEController.h"
#include <filesystem>
#include "Utils.h"

namespace fs = std::filesystem;
using namespace utils;


FHEController controller;
void setup_environment(int argc, char *argv[]);

string input_path;
string result_name;
string labels_file;
bool verbose = false;
bool plain = false;

std::vector<std::string> getFilesInDirectory(const std::string& path) {
    std::vector<std::string> file_list;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                file_list.push_back(entry.path().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Ошибка файловой системы: " << e.what() << std::endl;
    }
    return file_list;
}

int main(int argc, char *argv[]) {
    setup_environment(argc, argv);

    cout << "\n[0/2] Loading context and encrypted weights..." << endl;
    controller.load_context(verbose);
    controller.load_bootstrapping_and_rotation_keys("rotation_keys.txt", 16384, verbose);

    vector<double> labels = read_values_from_file(labels_file);

    double min = -2.0;
    double max = 2.0;
    int degree = 2000;

    // curr. batch = 1
    // SoonTM batch config
    vector<Ctxt> batch_acc;

    cout << "\n[1/2] Load clfs logits and evaluate" << endl;
    std::vector<std::string> clf_encs_paths = getFilesInDirectory(input_path);
    int n = clf_encs_paths.size();

    vector<Ctxt> vec_c_neg;
    vector<Ctxt> vec_c_pos;

    for (int i = 0; i < n; i++) {
        if (verbose) cout << "Processing " << clf_encs_paths[i] << endl;
        Ctxt classified = controller.load_ciphertext(clf_encs_paths[i]);

        vector<Ctxt> logits = controller.split_2_slots(classified);

        vec_c_neg.push_back(logits[0]);
        vec_c_pos.push_back(logits[1]);
    }

    Ctxt c_neg = controller.unwrap_vector_ctxts(vec_c_neg, n);
    Ctxt c_pos = controller.unwrap_vector_ctxts(vec_c_pos, n);

    Ctxt acc_enc = controller.accuracy(c_neg, c_pos, labels, min, max, degree);

    double approx_acc = 0.0;
    if (verbose) {
        // TODO: чтобы заменить это безобразие на более красивое решение
        // нужно использовать rot_sum с операцией div на 0 слот
        vector<double> dec = controller.decrypt_tovector(acc_enc, n);
        for (size_t i = 0; i < n; i++) {
            approx_acc += dec[i];  // суммируем только первые n слотов
        }
        approx_acc /= n;
        cout << "Approximate accuracy: " << approx_acc << endl;
    }

    cout << "\n[2/2] Save" << endl;
    controller.save(acc_enc, result_name);

    return 0;
}

void setup_environment(int argc, char *argv[]) {
    string command;

    if (argc < 4) {
        cout << "Usage: ./benchmark_eval <path_dir> <result_name> <labels_file> [OPTIONS]\n\n";
        cout << "Options:\n";
        cout << "  --verbose: Print detailed information, need private key\n";
        cout << "  --plain: Compare with plain circuit\n\n";
        cout << "Example:\n";
        cout << "  ./client_inference \"I think this movie is great!\" --verbose\n";
        exit(0);
    } else {
        input_path = argv[1];
        result_name = argv[2];
        labels_file = argv[3];

        for (int i = 2; i < argc; i++) {
            if (string(argv[i]) == "--verbose") {
                verbose = true;
            }
            if (string(argv[i]) == "--plain") {
                plain = true;
            }
        }
    }
}
