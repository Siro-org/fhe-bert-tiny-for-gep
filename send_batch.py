# --- Сетевой Конфиг ---
import requests


# GEP_DOMAIN_NAME = "0.0.0.0"
# GEP_DOMAIN_PORT = "8000"
# GEP_API = f"http://{GEP_DOMAIN_NAME}:{GEP_DOMAIN_PORT}/api/publish"
GEP_API = f"http://siroproject.tech:8000/api/publish"

BATCH_SIZE = 128
BENCH_RESULT_NAME = f"to_test/benchmark_result_{BATCH_SIZE}_batch.enc"

def send_result():
    print(f"[INFO] Sending Anon result to {GEP_API}")

    data = {
        "model_hf_id": "bert-base-uncased",
        "benchmark_name": "sst-2",
        "metric_name": "accuracy",
        "batch_size": BATCH_SIZE,

    }
    with open(BENCH_RESULT_NAME, "rb") as f:
        files = {
            "ciphertext": (BENCH_RESULT_NAME, f, "application/octet-stream")
        }
        result = requests.post(GEP_API, data=data, files=files)
        print(result.json())

send_result()
