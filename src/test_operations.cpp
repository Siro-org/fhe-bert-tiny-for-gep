#include "FHEController.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>

#define TEST_PRECISION 1e-3
#define EPSILON 1e-2
#define SGN_EPSILON 0.5

FHEController controller;

bool test_add_commutativity() {
    cout << "\n=== Test: Addition Commutativity ===" << endl;
    try {
        vector<double> data1 = {1.5, 2.3, 3.1, 4.2, 5.0};
        vector<double> data2 = {0.5, 1.2, 2.1, 1.8, 3.0};

        Ctxt c1 = controller.encrypt_weights(data1, 0, 0);
        Ctxt c2 = controller.encrypt_weights(data2, 0, 0);

        Ctxt result1 = controller.add(c1, c2);
        Ctxt result2 = controller.add(c2, c1);

        vector<double> dec1 = controller.decrypt_tovector(result1, 5);
        vector<double> dec2 = controller.decrypt_tovector(result2, 5);

        for (int i = 0; i < 5; i++) {
            if (abs(dec1[i] - dec2[i]) > EPSILON) {
                cout << "FAILED: Addition not commutative at index " << i << endl;
                cout << "  c1+c2=" << dec1[i] << ", c2+c1=" << dec2[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Addition is commutative" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_mult_plaintext_encrypted() {
    cout << "\n=== Test: Multiplication Plaintext * Ciphertext ===" << endl;
    try {
        vector<double> weights = {2.0, 3.0, 4.0, 5.0};
        vector<double> plain_input = {1.0, 2.0, 3.0, 4.0};

        Ctxt enc_weights = controller.encrypt_weights(weights, 0, 4);
        Ptxt plain = controller.encode(plain_input, 0, 4);

        Ctxt result = controller.mult(enc_weights, plain);
        vector<double> decrypted = controller.decrypt_tovector(result, 4);

        vector<double> expected = {2.0, 6.0, 12.0, 20.0};

        for (int i = 0; i < 4; i++) {
            double error = abs(decrypted[i] - expected[i]);
            if (error > TEST_PRECISION * max(abs(expected[i]), 1.0)) {
                cout << "FAILED: Multiplication error at index " << i << endl;
                cout << "  Expected: " << expected[i] << ", Got: " << decrypted[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Plaintext*Ciphertext multiplication correct" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_rotation_distance() {
    cout << "\n=== Test: Rotation Distance Property ===" << endl;
    try {
        vector<double> data(8);
        for (int i = 0; i < 8; i++) data[i] = i + 1;

        Ctxt encrypted = controller.encrypt_weights(data, 0, 8);

        Ctxt rot2 = controller.rotate(encrypted, 2);
        Ctxt rot1_twice = controller.rotate(controller.rotate(encrypted, 1), 1);

        vector<double> dec_rot2 = controller.decrypt_tovector(rot2, 8);
        vector<double> dec_rot1_twice = controller.decrypt_tovector(rot1_twice, 8);

        for (int i = 0; i < 8; i++) {
            double error = abs(dec_rot2[i] - dec_rot1_twice[i]);
            if (error > EPSILON) {
                cout << "FAILED: Rotation distance property violated at index " << i << endl;
                cout << "  rotate(2)=" << dec_rot2[i] << ", rotate(1,1)=" << dec_rot1_twice[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Rotation distance property holds" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_rotation_inverse() {
    cout << "\n=== Test: Rotation Inverse Property ===" << endl;
    try {
        vector<double> data(8);
        for (int i = 0; i < 8; i++) data[i] = i + 1;

        Ctxt encrypted = controller.encrypt_weights(data, 0, 8);

        Ctxt rot_forward = controller.rotate(encrypted, 3);
        Ctxt rot_back = controller.rotate(rot_forward, -3);

        vector<double> original = controller.decrypt_tovector(encrypted, 8);
        vector<double> recovered = controller.decrypt_tovector(rot_back, 8);

        for (int i = 0; i < 8; i++) {
            double error = abs(original[i] - recovered[i]);
            if (error > EPSILON) {
                cout << "FAILED: Rotation inverse property violated at index " << i << endl;
                cout << "  Original=" << original[i] << ", Recovered=" << recovered[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Rotation inverse property holds" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_add_distributivity() {
    cout << "\n=== Test: Addition Distributivity with Scaling ===" << endl;
    try {
        vector<double> a = {1.0, 2.0, 3.0, 4.0};
        vector<double> b = {0.5, 1.5, 2.5, 3.5};
        vector<double> c = {2.0, 1.0, 3.0, 2.5};

        Ctxt ca = controller.encrypt_weights(a, 0, 4);
        Ctxt cb = controller.encrypt_weights(b, 0, 4);
        Ctxt cc = controller.encrypt_weights(c, 0, 4);

        Ctxt sum_bc = controller.add(cb, cc);
        Ctxt result = controller.add(ca, sum_bc);

        vector<double> result_direct = controller.decrypt_tovector(result, 4);

        vector<double> expected(4);
        for (int i = 0; i < 4; i++) {
            expected[i] = a[i] + b[i] + c[i];
        }

        for (int i = 0; i < 4; i++) {
            double error = abs(result_direct[i] - expected[i]);
            if (error > TEST_PRECISION * max(abs(expected[i]), 1.0)) {
                cout << "FAILED: Distributivity error at index " << i << endl;
                cout << "  Expected: " << expected[i] << ", Got: " << result_direct[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Addition distributivity holds" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_mult_by_scalar() {
    cout << "\n=== Test: Scalar Multiplication ===" << endl;
    try {
        vector<double> weights = {1.0, 2.0, 3.0, 4.0};
        double scalar = 2.5;

        Ctxt enc_weights = controller.encrypt_weights(weights, 0, 4);
        Ctxt scaled = controller.mult(enc_weights, scalar);

        vector<double> result = controller.decrypt_tovector(scaled, 4);

        for (int i = 0; i < 4; i++) {
            double expected = weights[i] * scalar;
            double error = abs(result[i] - expected);
            if (error > TEST_PRECISION * max(abs(expected), 1.0)) {
                cout << "FAILED: Scalar multiplication error at index " << i << endl;
                cout << "  Expected: " << expected << ", Got: " << result[i] << endl;
                return false;
            }
        }
        cout << "PASSED: Scalar multiplication correct" << endl;
        return true;
    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_non_commutativity_note() {
    cout << "\n=== Note: Ciphertext Rotations ===" << endl;
    cout << "WARNING: CKKS rotations have NON-COMMUTATIVE behavior when combined with ";
    cout << "ciphertext operations." << endl;
    cout << "rotate(A, i) and rotate(B, j) don't commute with ciphertexts due to ";
    cout << "Galois automorphisms." << endl;
    cout << "This is EXPECTED and NOT an error." << endl;
    return true;
}

// bool test_sign_x_function() {
//     cout << "\n=== Test: sign_x Function Approximation ===" << endl;
//     try {
//         // Тестовые значения
//         vector<double> test_values = {-1.0, -0.5, 0.0, 0.5, 1.0};
//         int d = 2; // количество композиций f4

//         for (double val : test_values) {
//             // Шифруем значение
//             Ctxt ctxt = controller.encrypt_weights({val}, 0, 1);

//             // Вычисляем sign_x
//             Ctxt result = controller.sign_x(ctxt, d);

//             // Дешифруем
//             vector<double> dec = controller.decrypt_tovector(result, 1);
//             double approx = dec[0];

//             // Ожидаем sign
//             double expected = (val > 0) ? 1.0 : (val < 0 ? -1.0 : 0.0);

//             double error = abs(approx - expected);

//             cout << fixed << setprecision(4);
//             cout << "x = " << val
//                  << ", sign_x ≈ " << approx
//                  << ", expected = " << expected
//                  << ", pass = " << (error < EPSILON ? "YES" : "NO") << endl;

//             if (error >= EPSILON) {
//                 cout << "FAILED: Approximation error exceeds EPSILON" << endl;
//                 return false;
//             }
//         }

//         cout << "PASSED: sign_x approximation within EPSILON" << endl;
//         return true;

//     } catch (exception& e) {
//         cout << "EXCEPTION: " << e.what() << endl;
//         return false;
//     }
// }

// TODO: change to +sign()
bool test_eval_sign_function() {
    std::cout << "\n=== Test: Eval Sign Function (Chebyshev) on [-30, 30] ===" << std::endl;
    try {
        // Пример входных значений
        std::vector<double> input = {25.3, -15.3, 5.0, 2.3, -24.2};
        double min = -30.0;
        double max = 30.0;


        // Ожидаемые значения
        std::vector<double> expected;
        for (double v : input) {
            if (v > 0) expected.push_back(1.0);
            else if (v < 0) expected.push_back(-1.0);
            else expected.push_back(0.0);
        }

        // Сравнение
        bool passed = true;
        // Шифруем вход
        Ctxt encrypted = controller.encrypt_weights(input, 0, 0);
        // Аппроксимация sign через Chebyshev
        // controller.mult(encrypted, max_abs);
        Ctxt approx_sign = controller.eval_sign_function(encrypted, min, max, 25);

        // Дешифруем
        std::vector<double> result = controller.decrypt_tovector(approx_sign, 0);
        for (int i = 0; i < input.size(); i++) {
            double error = std::abs(result[i] - expected[i]);
            if (error > SGN_EPSILON) { // допустимая погрешность для Chebyshev-аппроксимации
                std::cout << "FAILED at index " << i << ": got " << result[i] << ", expected " << expected[i] << std::endl;
                passed = false;
            }
            else {
                std::cout  << "Approx = " << result[i] << ", expected = " << expected[i] << std::endl;
            }
        }
        if (passed) std::cout << "PASSED: Eval Sign Function correct" << std::endl;
        return passed;

    } catch (std::exception &e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        return false;
    }
}

// TODO: change to +sign()
bool test_sign_difference() {
    cout << "\n=== Test: sign_difference Function ===" << endl;
    try {
        // Тестовые пары значений: {x, y}
        vector<pair<double,double>> test_pairs = {
            {0.8, 0.3},    // x > y → 1
            {0.2, 0.5},    // x < y → -1
            {0.0, 0.0},    // x = y → 0
            {-0.7, -0.2},  // x < y → -1
            {0.5, 0.5}     // x = y → 0
        };

        for (auto &p : test_pairs) {
            double x_val = p.first;
            double y_val = p.second;

            // Шифруем
            Ctxt enc_x = controller.encrypt_weights({x_val}, 0, 0);
            Ctxt enc_y = controller.encrypt_weights({y_val}, 0, 0);

            // Вычисляем sign_difference
            Ctxt enc_result = controller.sign_difference(enc_x, enc_y);

            // Дешифруем
            vector<double> dec = controller.decrypt_tovector(enc_result, 0);
            double approx = dec[0];

            // Ожидаем sign(x - y)
            double expected = (x_val > y_val) ? 1.0 : (x_val < y_val ? -1.0 : 0.0);

            double error = abs(approx - expected);

            cout << fixed << setprecision(4);
            cout << "x = " << x_val << ", y = " << y_val
                 << ", sign_difference ≈ " << approx
                 << ", expected = " << expected
                 << ", pass = " << (error < SGN_EPSILON ? "YES" : "NO") << endl;

            if (error >= SGN_EPSILON) {
                cout << "FAILED: Approximation error exceeds EPSILON" << endl;
                return false;
            }
        }

        cout << "PASSED: sign_difference approximation within EPSILON" << endl;
        return true;

    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}

bool test_accuracy() {
    cout << "\n=== Test: accuracy Function ===" << endl;
    try {
        // 1. Тестовые данные (пары логитов: [neg, pos])
        vector<pair<double,double>> test_logits = {
            // === True Positives (TP) ===
            {-1.5, -0.3},
            {-2.0, -1.0},
            {-0.8, -0.2},
            {-1.2, -0.4},
            {-0.5,  0.2},
            {-1.0, -0.3},
            {-2.5, -1.2},
            {-0.7,  0.0},
            {-1.8, -0.9},
            {-2.2, -1.1},
            {-1.1, -0.6},
            {-0.6, -0.1},
            {-0.9, -0.4},

            // === True Negatives (TN) ===
            { 1.5,  0.4},
            { 2.0,  1.0},
            { 0.8,  0.2},
            { 1.2,  0.3},
            { 0.6,  0.1},
            { 1.0,  0.5},
            { 2.5,  1.3},
            { 0.7,  0.0},
            { 1.8,  0.9},
            { 2.2,  1.1},
            { 1.1,  0.4},
            { 0.5, -0.1},

            // === False Positives (FP) ===
            { 1.5,  2.5},
            { 0.3,  1.0},
            {-0.2,  1.0},
            { 0.0,  1.5},
            {-1.0,  1.2},
            {-2.0,  2.5},
            { 0.5,  2.0},
            {-0.3,  1.8},
            { 1.0,  2.5},
            {-1.5,  1.0},
            {-0.5,  0.8},
            { 0.2,  1.7},

            // === False Negatives (FN) ===
            {-0.2, -2.0},
            { 0.1, -1.5},
            { 0.8, -2.5},
            { 1.0, -3.0},
            { 0.5, -1.8},
            { 0.3, -1.5},
            { 0.2, -1.2},
            { 0.9, -2.0},
            { 0.6, -1.7},
            { 0.8, -2.2},
            { 0.4, -1.0},
            { 0.5, -1.3}
        };

        vector<double> labels = {
            // TP: 13 × 1
            1,1,1,1,1,1,1,1,1,1,1,1,1,
            // TN: 12 × -1
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            // FP: 12 × -1 (ошибка: пред = 1)
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            // FN: 12 × 1 (ошибка: пред = -1)
            1,1,1,1,1,1,1,1,1,1,1,1
        };
        size_t n = labels.size();  // количество примеров
        // Разделяем логиты на два вектора
        vector<double> neg_logits;
        vector<double> pos_logits;
        neg_logits.reserve(test_logits.size());
        pos_logits.reserve(test_logits.size());

        for (const auto &p : test_logits) {
            neg_logits.push_back(p.first);
            pos_logits.push_back(p.second);
        }

        // Создаём plaintext и ciphertext

        Ctxt c_neg = controller.encrypt(neg_logits, 0, n);
        Ctxt c_pos = controller.encrypt(pos_logits, 0, n);

        // Параметры
        double min = -6.0;
        double max = 6.0;
        int degree = 2000;

        // Вычисляем точность
        Ctxt acc_enc = controller.accuracy(c_neg, c_pos, labels, min, max, degree);

        // 6. Дешифруем результат
        vector<double> dec = controller.decrypt_tovector(acc_enc, n);
        // получаем вид
        // [ 1.2996 1.2749 1.2501 1.2254 1.1999 1.1745 1.1496 1.1248 1.1000 1.0753 1.0505 1.0253 1.0007 0.9755 0.9503 0.9251 0.9008 0.8756 0.8504 0.8243]
        double approx_acc = 0.0;
        for (size_t i = 0; i < n; i++) {
            approx_acc += dec[i];  // суммируем только первые n слотов
        }
        approx_acc /= n;

        // 7. Эталонная точность в открытом виде
        size_t correct = 0;
        for (size_t i = 0; i < test_logits.size(); i++) {
            double neg = test_logits[i].first;
            double pos = test_logits[i].second;
            double pred = (neg - pos > 0) ? -1.0 : ((neg - pos < 0) ? 1.0 : 0.0);
            if (fabs(pred - labels[i]) < 1e-9)
                correct++;
        }
        double expected_acc = static_cast<double>(correct) / labels.size();

        // 8. Проверяем погрешность
        double error = fabs(approx_acc - expected_acc);

        cout << fixed << setprecision(6);
        cout << "Decrypted accuracy ≈ " << approx_acc
             << ", expected = " << expected_acc
             << ", error = " << error
             << ", pass = " << (error < EPSILON ? "YES" : "NO") << endl;

        if (error >= EPSILON) {
            cout << "FAILED: Accuracy approximation exceeds EPSILON" << endl;
            return false;
        }

        cout << "PASSED: accuracy approximation within EPSILON" << endl;
        return true;

    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}


bool test_split_slots_by_rotation_and_sign() {
    cout << "\n=== Test: split_slots_by_rotation Function ===" << endl;
    try {
        // --- 1. Подготовка тестовых данных ---
        // vector<double> values = {1.1, 2.2, 3.3, 4.4}; // известный вектор
        vector<double> values = {1.1, 2.2}; // известный вектор
        double true_sign = 1.0;
        size_t slot_count = values.size();

        cout << "Input vector: ";
        for (auto v : values) cout << v << " ";
        cout << endl;

        // --- 2. Шифруем весь вектор ---
        Ctxt enc_input = controller.encrypt_weights(values, 0, values.size());

        // --- 3. Вызываем split_slots_by_rotation ---
        vector<Ctxt> splitted = controller.split_2_slots(enc_input);

        if (splitted.size() != slot_count) {
            cout << "FAILED: размер вывода != slot_count" << endl;
            return false;
        }

        Ctxt diff = controller.add(splitted[0], controller.mult(splitted[1], -1));
        Ctxt sign = controller.eval_sign_function(diff, -4, 4, 200);

        // --- 4. Проверяем каждый расшифрованный шифротекст ---
        double EPS = 1e-2;
        bool all_ok = true;

        for (size_t i = 0; i < slot_count; ++i) {
            vector<double> dec = controller.decrypt_tovector(splitted[i], values.size());

            cout << "Slot " << i << ": " << dec[0] << endl;

            double error = abs(dec[0] - values[i]);

            if (error > EPS) {
                cout << "FAILED: slot[" << i <<
                        "expected=" << values[i] << ", got=" << dec[0] << endl;
                all_ok = false;
            }
        }

        vector<double> dec_sign = controller.decrypt_tovector(sign, 128);

        cout << "sign_values" << dec_sign << endl;
        cout << dec_sign[0] << endl;

        double error = abs(dec_sign[0] - true_sign);
        if (error > EPS) {
            cout << "FAILED" << endl;
            all_ok = false;
        }

        if (all_ok)
            cout << "PASSED: split_slots_by_rotation outputs correct per-slot ciphertexts" << endl;
        else
            cout << "FAILED: mismatch detected in decrypted slots" << endl;

        return all_ok;

    } catch (exception& e) {
        cout << "EXCEPTION: " << e.what() << endl;
        return false;
    }
}


int main() {
    cout << "\n╔════════════════════════════════════════════════════════╗" << endl;
    cout << "║     FHE Operations Unit Tests - Encrypted Weights      ║" << endl;
    cout << "║     CKKS Scheme Commutativity Verification            ║" << endl;
    cout << "╚════════════════════════════════════════════════════════╝" << endl;

    try {
        controller.generate_context(false, false);
        // vector<int> rotations = {1, 2, 3, -1, -2, -3};
        vector<int> rotations = {
            1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
            -1, -2, -3,
        };
        controller.generate_bootstrapping_and_rotation_keys(rotations, 16384, false, "rotation_keys.txt");

        int passed = 0;
        int total = 10;

        if (test_split_slots_by_rotation_and_sign()) passed++;
        // if (test_accuracy()) passed++;
        // if (test_add_commutativity()) passed++;
        // if (test_mult_plaintext_encrypted()) passed++;
        // if (test_rotation_distance()) passed++;
        // if (test_rotation_inverse()) passed++;
        // if (test_add_distributivity()) passed++;
        // if (test_mult_by_scalar()) passed++;
        // if (test_sign_x_function()) passed++;
        // if (test_eval_sign_function()) passed++;
        // if (test_sign_difference()) passed++;
        // if (test_non_commutativity_note()) {}

        cout << "\n╔════════════════════════════════════════════════════════╗" << endl;
        cout << "║                   TEST SUMMARY                         ║" << endl;
        cout << "║ Passed: " << setw(2) << passed << "/" << setw(2) << total << "                                      ║" << endl;
        cout << "║ Status: " << (passed == total ? "✓ ALL TESTS PASSED" : "✗ SOME TESTS FAILED")
             << setw(21) << "║" << endl;
        cout << "╚════════════════════════════════════════════════════════╝" << endl;

        return (passed == total) ? 0 : 1;

    } catch (exception& e) {
        cout << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }
}

