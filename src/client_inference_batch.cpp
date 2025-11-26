#include <iostream>
#include "FHEController.h"
#include <chrono>
#include <filesystem>

#define GREEN_TEXT "\033[1;32m"
#define RED_TEXT "\033[1;31m"
#define RESET_COLOR "\033[0m"

using namespace std::chrono;
namespace fs = std::filesystem;

FHEController controller;

void setup_environment(int argc, char *argv[]);

vector<Ctxt> encoder1(string input_folder);
Ctxt encoder2(vector<Ctxt> input);
Ctxt pooler(Ctxt input);
Ctxt classifier(Ctxt input);

bool verbose = false;
bool plain = false;
// bool demo = false;
string text;
string input_folder;
string output_folder;

int main(int argc, char *argv[]) {
    setup_environment(argc, argv);

    // Load context and keys
    cout << "\n[0/2] Loading context and encrypted weights..." << endl;
    controller.load_context(verbose);
    controller.load_bootstrapping_and_rotation_keys("rotation_keys.txt", 16384, verbose);

    if (verbose) cout << "The evaluation of the circuit started." << endl;

    // get folder size
    fs::path path(input_folder);
    fs::directory_iterator it(path), endit;
    int folder_size = 0;
    for (; it != endit; ++it) folder_size++;

    for (int i = 0; i < folder_size; i++) {
        string input_file = input_folder + "/" + input_folder + "_" + to_string(i) + ".txt";
        cout << "[" << i + 1 << "/" << folder_size << "] [0/2] Loading input from " << input_file << "..." << endl;

        Ptxt plain_input = controller.read_plain_repeated_input(input_file);
        // vector<double> vector_input = read_values_from_file(input_file);
        Ctxt encrypted_input = controller.encrypt_ptxt(plain_input);

        cout << "[" << i + 1 << "/" << folder_size << "] [1/2] Running Pooler..." << endl;
        Ctxt pooled = pooler(encrypted_input);

        cout << "[" << i + 1 << "/" << folder_size << "] [2/2] Running Classifier..." << endl;
        Ctxt classified = classifier(pooled);

        if (verbose) cout << "The circuit has been evaluated, the results are sent back to the client" << endl << endl;
        if (verbose) cout << "CLIENT-SIDE" << endl;

        if (verbose)
            controller.print(classified, 2, "Output logits");

        // dump clf-encrypted
        string output_file = output_folder + "/res_" + to_string(i) + ".txt.enc";
        controller.save(classified, output_file);
    }
    return 0;
}

Ctxt classifier(Ctxt input) {
    // Load encrypted classifier weights
    Ctxt weight = controller.load_ciphertext("encrypted_weights/classifier_weight.txt.enc");
    Ctxt bias = controller.load_ciphertext("encrypted_weights/classifier_bias.txt.enc");

    Ctxt output = controller.mult(input, weight);
    output = controller.rotsum(output, 128, 1);
    output = controller.add(output, bias);

    vector<double> mask;
    for (int i = 0; i < controller.num_slots; i++) {
        mask.push_back(0);
    }
    mask[0] = 1;
    mask[128] = 1;

    output = controller.mult(output, controller.encrypt(mask, output->GetLevel()));
    output = controller.add(output, controller.rotate(controller.rotate(output, -1), 128));

    return output;
}

Ctxt pooler(Ctxt input) {
    auto start = high_resolution_clock::now();
    double tanhScale = 1 / 30.0;


    Ctxt weight_enc = controller.load_ciphertext("encrypted_weights/pooler_dense_weight.txt.enc");
    Ctxt bias_enc = controller.load_ciphertext("encrypted_weights/pooler_dense_bias.txt.enc");

    Ctxt output = controller.mult(input, weight_enc);

    output = controller.rotsum(output, 128, 128);
    output = controller.add(output, bias_enc);
    output = controller.eval_tanh_function(output, -1, 1, tanhScale, 200); // 9 depth
    output = controller.bootstrap(output);

    if (verbose) cout << "The evaluation of Pooler took: " << (duration_cast<milliseconds>(high_resolution_clock::now() - start)).count() / 1000.0 << " seconds." << endl;
    if (verbose) controller.print(output, 128, "Pooler (Repeated)");

    return output;
}

void setup_environment(int argc, char *argv[]) {
    string command;

    // if (argv[1] == "--demo") {
    //     demo = true;
    //     return;
    // }
    if (argc < 3) {
        cout << "Usage: ./client_inference <input_folder> <result_folder> [OPTIONS]\n\n";
        cout << "Options:\n";
        cout << "  --verbose: Print detailed information, need private key\n";
        cout << "  --plain: Compare with plain circuit\n\n";
        cout << "  --demo: continue with inference 'It's a good film'\n\n";
        cout << "Example:\n";
        cout << "  ./client_inference \"I think this movie is great!\" --verbose\n";
        // TODO: upd example in usage cout
        // TODO: upd options based on usage
        exit(0);
    } else {
        input_folder = argv[1];
        output_folder = argv[2];

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
