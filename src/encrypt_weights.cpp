#include "FHEController.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

FHEController controller;

struct WeightSpec {
    string path;
    string func;
    vector<string> args; // аргументы кроме path
};


int verbose = 1;

double parse_arg(const string& s) {
    if (s.find('/') != string::npos) {
        // поддержка вида "1/13.5"
        auto pos = s.find('/');
        double num = stod(s.substr(0, pos));
        double denom = stod(s.substr(pos + 1));
        return num / denom;
    }
    return stod(s);
}

string join_args(const vector<string>& args) {
    string out;
    for (size_t i = 0; i < args.size(); ++i) {
        out += args[i];
        if (i + 1 < args.size()) out += ", ";
    }
    return out.empty() ? "—" : out;
}

Ptxt call_read_func(const WeightSpec& spec) {
    if (verbose) {
        cout << "   [ℹ] Function: " << spec.func
         << " | Args: [" << join_args(spec.args) << "]" << endl;

    }

    if (spec.func == "read_plain_input") {
        if (spec.args.size() == 0) return controller.read_plain_input(spec.path);
        if (spec.args.size() == 1) return controller.read_plain_input(spec.path, parse_arg(spec.args[0]));
        if (spec.args.size() == 2) return controller.read_plain_input(spec.path, parse_arg(spec.args[0]), parse_arg(spec.args[1]));
    }
    else if (spec.func == "read_plain_repeated_input") {
        if (spec.args.size() == 0) return controller.read_plain_repeated_input(spec.path);
        if (spec.args.size() == 1) return controller.read_plain_repeated_input(spec.path, parse_arg(spec.args[0]));
        if (spec.args.size() == 2) return controller.read_plain_repeated_input(spec.path, parse_arg(spec.args[0]), parse_arg(spec.args[1]));
    }
    else if (spec.func == "read_plain_expanded_input") {
        if (spec.args.size() == 0) return controller.read_plain_expanded_input(spec.path);
        if (spec.args.size() == 1) return controller.read_plain_expanded_input(spec.path, parse_arg(spec.args[0]));
        if (spec.args.size() == 2) return controller.read_plain_expanded_input(spec.path, parse_arg(spec.args[0]), parse_arg(spec.args[1]));
        if (spec.args.size() == 3) return controller.read_plain_expanded_input(spec.path, parse_arg(spec.args[0]), parse_arg(spec.args[1]), parse_arg(spec.args[2]));
    }

    throw runtime_error("Unknown function or invalid args: " + spec.func);
}

vector<WeightSpec> get_all_ptxt_specs() {
    return {
        // // ───── Layer 0 (encoder1, level = 8) ─────
        // {"../weights-sst2/layer0_attself_query_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_attself_query_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer0_attself_key_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_attself_key_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer0_attself_value_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_attself_value_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer0_selfoutput_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_selfoutput_bias.txt", "read_plain_expanded_input", {"8"}},
        // {"../weights-sst2/layer0_selfoutput_mean.txt", "read_plain_repeated_input", {"8", "-1"}},
        // {"../weights-sst2/layer0_selfoutput_vy.txt", "read_plain_input", {"8", "1"}},
        // {"../weights-sst2/layer0_selfoutput_normbias.txt", "read_plain_expanded_input", {"8", "1"}},
        // {"../weights-sst2/layer0_intermediate_weight1.txt", "read_plain_input", {"8", "1/13.5"}},
        // {"../weights-sst2/layer0_intermediate_weight2.txt", "read_plain_input", {"8", "1/13.5"}},
        // {"../weights-sst2/layer0_intermediate_weight3.txt", "read_plain_input", {"8", "1/13.5"}},
        // {"../weights-sst2/layer0_intermediate_weight4.txt", "read_plain_input", {"8", "1/13.5"}},
        // {"../weights-sst2/layer0_intermediate_bias.txt", "read_plain_input", {"8", "1/13.5"}},
        // {"../weights-sst2/layer0_output_weight1.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_output_weight2.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_output_weight3.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_output_weight4.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer0_output_bias.txt", "read_plain_expanded_input", {"8"}},
        // {"../weights-sst2/layer0_output_mean.txt", "read_plain_repeated_input", {"8", "-1"}},
        // {"../weights-sst2/layer0_output_vy.txt", "read_plain_input", {"8", "1"}},
        // {"../weights-sst2/layer0_output_normbias.txt", "read_plain_expanded_input", {"8", "1"}},

        // // ───── Layer 1 (encoder2, level = 8) ─────
        // {"../weights-sst2/layer1_attself_query_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_attself_query_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer1_attself_key_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_attself_key_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer1_attself_value_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_attself_value_bias.txt", "read_plain_repeated_input", {"8"}},
        // {"../weights-sst2/layer1_selfoutput_weight.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_selfoutput_bias.txt", "read_plain_expanded_input", {"8"}},
        // {"../weights-sst2/layer1_selfoutput_mean.txt", "read_plain_repeated_input", {"8", "-1"}},
        // {"../weights-sst2/layer1_selfoutput_vy.txt", "read_plain_input", {"8", "1"}},
        // {"../weights-sst2/layer1_selfoutput_normbias.txt", "read_plain_expanded_input", {"8", "1"}},
        // {"../weights-sst2/layer1_intermediate_weight1.txt", "read_plain_input", {"8", "1/17.0"}},
        // {"../weights-sst2/layer1_intermediate_weight2.txt", "read_plain_input", {"8", "1/17.0"}},
        // {"../weights-sst2/layer1_intermediate_weight3.txt", "read_plain_input", {"8", "1/17.0"}},
        // {"../weights-sst2/layer1_intermediate_weight4.txt", "read_plain_input", {"8", "1/17.0"}},
        // {"../weights-sst2/layer1_intermediate_bias.txt", "read_plain_input", {"8", "1/17.0"}},
        // {"../weights-sst2/layer1_output_weight1.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_output_weight2.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_output_weight3.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_output_weight4.txt", "read_plain_input", {"8"}},
        // {"../weights-sst2/layer1_output_bias.txt", "read_plain_expanded_input", {"8"}},
        // {"../weights-sst2/layer1_output_mean.txt", "read_plain_repeated_input", {"8", "-1"}},
        // {"../weights-sst2/layer1_output_vy.txt", "read_plain_input", {"8", "1"}},
        // {"../weights-sst2/layer1_output_normbias.txt", "read_plain_expanded_input", {"8", "1"}},

        // ───── Pooler ─────
        {"weights-sst2/pooler_dense_weight.txt", "read_plain_input", {"13", "1/30.0"}},
        {"weights-sst2/pooler_dense_bias.txt", "read_plain_repeated_input", {"12", "1/30.0"}},
        // {"weights-sst2/pooler_dense_weight.txt", "read_plain_input", {"12"}},
        // {"weights-sst2/pooler_dense_bias.txt", "read_plain_repeated_input", {"12"}},

        // ───── Classifier ─────
        {"weights-sst2/classifier_weight.txt", "read_plain_input", {"13"}},
        {"weights-sst2/classifier_bias.txt", "read_plain_expanded_input", {"12"}}
    };
}

int main() {
    cout << "\n[🔐] Encrypting all Ptxt weights from weights-sst2/ → encrypted_weights/\n";

    // Step 1: Generate context
    cout << "\n[1/4] Generating FHE context..." << endl;
    // controller.parameters_folder("../keys")
    controller.generate_context(true, false);

    // Step 2: Generate rotation keys
    cout << "[2/4] Generating rotation keys..." << endl;
    vector<int> rotations = {
        1, 2, 3, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
        -1, -2, -3, -4, -8, -16, -32, -64, -256, -512
    };
    controller.generate_bootstrapping_and_rotation_keys(rotations, 16384, true, "rotation_keys.txt");

    // Step 3: Encrypt weights
    cout << "[3/4] Encrypting model weights from weights-sst2/..." << endl;
    system("mkdir -p encrypted_weights");
    for (auto& spec : get_all_ptxt_specs()) {
        cout << "→ Encrypting " << fs::path(spec.path).filename() << " ..." << endl;

        Ptxt p = call_read_func(spec);
        Ctxt c = controller.encrypt_ptxt(p);

        string out = "encrypted_weights/" + fs::path(spec.path).filename().string() + ".enc";
        controller.save(c, out);
    }

    // Step 4: Summary
    cout << "[4/4] Encryption complete" << endl;

    cout << "\n╔══════════════════════════════════════════════════════════════════╗" << endl;
    cout << "║                              SETUP COMPLETE                        ║" << endl;
    cout << "║                                                                    ║" << endl;
    cout << "║  Encrypted " << get_all_ptxt_specs().size() << " weight files      ║" << endl;
    cout << "║  Keys saved to: ../keys/                                           ║" << endl;
    cout << "║  Weights saved to: ../encrypted_weights/                           ║" << endl;
    cout << "╚════════════════════════════════════════════════════════════════════╝" << endl;

    cout << "\nGenerated files:" << endl;
    cout << "  ✓ ../keys/crypto-context.txt" << endl;
    cout << "  ✓ ../keys/public-key.txt" << endl;
    cout << "  ✓ ../keys/secret-key.txt (KEEP SECURE)" << endl;
    cout << "  ✓ ../keys/mult-keys.txt" << endl;
    cout << "  ✓ ../keys/rot_rotation_keys.txt" << endl;
    cout << "  ✓ ../encrypted_weights/*.enc (" << get_all_ptxt_specs().size() << " files)" << endl;

    return 0;
}
