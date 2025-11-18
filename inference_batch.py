from fileinput import filename
import requests
import torch
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from datasets import load_dataset
import subprocess
import os
import numpy as np

# --- Конфиг ---
MODEL_NAME = "philschmid/tiny-bert-sst2-distilled"
HS_FOLDER = "hs"
HS_FILE = f"{HS_FOLDER}/hs"
BINARY_DIR = "./build"  # путь к бинарю
OUTPUT_DIR = "results"
LABELS_FILE = "labels.txt"
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)
BENCH_RESULT_NAME = f"benchmark_result.enc"

NUM_THREADS = 4  # для параллельной записи
DATASET_SIZE = 100 # TODO:
# TOTAL_SIZE TODO: refactoring
# BATCH_SIZE = 32
# BIAS_INDEX = 0
BATCH_SIZE = 96
BIAS_INDEX = 32

# --- Сетевой Конфиг ---
GEP_DOMAIN_NAME = "siroproject.tech"
GEP_DOMAIN_PORT = 8080
GEP_API = f"http://{GEP_DOMAIN_NAME}:{GEP_DOMAIN_PORT}/api"

# --- Dev Config ---
VERBOSE = ""
SET_VERBOSE = True
if SET_VERBOSE:
    VERBOSE = "--verbose"

# --- Выбираем устройство ---
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
# device = torch.device("cpu")
print(f"[INFO] Using device: {device}")

# --- Загружаем SST-2 тест ---
dataset = load_dataset("glue", "sst2", split="validation")
texts = dataset["sentence"]
labels = dataset["label"]
texts = np.array(texts).astype(str).tolist()
labels = np.array(labels).tolist()

# dev
texts = texts[BIAS_INDEX:BATCH_SIZE]
labels = labels[BIAS_INDEX:BATCH_SIZE]
labels = [x if x != 0 else -1 for x in labels] # [1, 0, 0, 1] to [1, -1, -1, 1]

np.savetxt(LABELS_FILE, np.array(labels).reshape(1, -1), delimiter=",", fmt="%d")

# --- Загружаем токенизатор и модель ---
tokenizer = AutoTokenizer.from_pretrained(MODEL_NAME)
model = AutoModelForSequenceClassification.from_pretrained(MODEL_NAME)

# --- Берем только первые два слоя TinyBERT encoder ---
# TinyBERT
class TinyBertPartial(torch.nn.Module):
    def __init__(self, full_model):
        super().__init__()
        self.embeddings = full_model.bert.embeddings
        self.encoder_layers = full_model.bert.encoder.layer[:2]

    def forward(self, tokens_tensor, len_tokenized_text, attention_mask=None, **kwargs):
        x = self.embeddings(tokens_tensor, len_tokenized_text)
        for layer in self.encoder_layers:
            x = layer(x)[0]
        return x

partial_model = TinyBertPartial(model)
partial_model = partial_model.to(device)
partial_model.eval()

test_count = 0
len_test = len(texts)


def wrap_sentence(x: np.ndarray) -> np.ndarray:
    return np.char.add("[CLS] ", np.char.add(x, " [SEP]"))

# --- Функция для инференса батчей ---
def inference_batch(batch_texts):
    global test_count

    # batch_texts = map(wrap_sentence, batch_texts)
    batch_texts = wrap_sentence(batch_texts)
    inputs = tokenizer(list(batch_texts), return_tensors="pt", padding=True, truncation=True)

    tokens_tensor = inputs["input_ids"]
    len_tokenized_text = tokens_tensor.shape[1]
    len_text_tensor = torch.tensor([[1] * len_tokenized_text])
    with torch.no_grad():
        hidden_states = partial_model(tokens_tensor.to(device), len_text_tensor.to(device))

        if isinstance(hidden_states, tuple):
            hidden_states = tuple(hs.cpu() for hs in hidden_states)
        else:
            hidden_states = hidden_states.cpu()

        for i, hs in enumerate(hidden_states):
            print(f"Iter: {i} | Input Text: {texts[i]}")
            file_name = f"{HS_FILE}_{i}.txt"
            np.savetxt(file_name, hs.detach().cpu().numpy()[0, :], delimiter=",")

            print(f"[INFO] Output saved to {file_name}")

            # OUTPUT_FILE = f"{OUTPUT_DIR}/res_{test_count}.txt.enc"
        # --- Запускаем бинарь с аргументом ---
        if os.path.exists(BINARY_DIR):
            subprocess.run([BINARY_DIR + "/client_inference_batch",
                            HS_FOLDER,
                            OUTPUT_DIR,
                            VERBOSE
                            ]
            )
            test_count += 1
        else:
            print(f"[WARNING] Binary not found at {BINARY_DIR}")

def benchmark_batch():
    if os.path.exists(BINARY_DIR):
        subprocess.run([BINARY_DIR + "/benchmark_eval",
                        OUTPUT_DIR,
                        BENCH_RESULT_NAME,
                        LABELS_FILE,
                        VERBOSE
                        ]
        )
    else:
        print(f"[WARNING] Binary not found at {BINARY_DIR}")


def send_result():
    print(f"[INFO] Sending Anon result to {GEP_API}")

    payload = {
        "model_hf_id": "bert-base-uncased",
        "benchmark_name": "sst-2",
        "metric_name": "accuracy",
        "batch_size": BATCH_SIZE,

    }
    with open(BENCH_RESULT_NAME, "rb") as f:
        requests.post(GEP_API, data=payload, files={"ciphertext": f})


# for i in range(0, len(texts), BATCH_SIZE):
#     batch_texts = texts[i:i+BATCH_SIZE]
#     inference_batch(batch_texts)

benchmark_batch()

# send_result()
