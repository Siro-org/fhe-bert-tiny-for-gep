# FHE BERT Tiny for open-source benchmark validation

Fork of source code [FHE TinyBERT by lorenzorovida](https://github.com/lorenzorovida/FHE-BERT-Tiny) (also check their research paper after that)

## What is it

Reposiroty as Benchmark Validation Proof-Of-Concept with full privacy-preserving.
More details in [website](https://web3.siroproject.tech/).

### TL;DR

- We tested our GEP approach on TinyBERT with only encrypted Pooler and Classifier layers
  - (without encryption of the first two layers due to faster inference)
- GEP (Get-Eval-Publish) approach offers full privacy-preserving with advanced intercommunication between the Benchmark Owner and the Model Owner
- It make available open-source contribution Privacy Model for Skill Transfer Validation (which earlier sounded like an oxymoron)

## 0. Prerequisites

Currently Linux only, not tested in Docker yet

In order to run the program, you need to install:

- `cmake`
- `g++` or `clang`
- `OpenFHE` ([how to install OpenFHE](https://openfhe-development.readthedocs.io/en/latest/sphinx_rsts/intro/installation/installation.html))
The algorithm has been tested in OpenFHE v1.4.2
- `python` tested in 3.12
- Package manager, `uv` recommended

## 0.1 Make build

In workdir:

```bash
mkdir build
make build
```

## 1. Python libs

After intalling all the required prerequisites, install the required Python libraries using uv (or other pm):

```bash
uv sync
```

## How to

<u>Warning:</u> all steps are raw and unfinished, without end-to-end experience. Read the code first.

### 2.1 Publich to main Siro MRP (As bencmark owner validator)

If you want to publish model validation results to siroproject.tech for checking our POC

#### 2.1.1 Load model Encrypted data

Load model's encrypted data and keys from our Hugging Face repository:
[xLagerFeuer/fhe-bert-tiny-sst2](https://huggingface.co/xLagerFeuer/fhe-bert-tiny-sst2)

```bash
hf download xLagerFeuer/fhe-bert-tiny-sst2 --local-dir .
```

#### 2.1.2 Run inference_batch.py

Use inference_batch.py with default validation dataset (GLUE, SST2), batch size and verbose mode (ON by default)
Or with your custom settings

```bash
uv run inference_batch.py
```

#### 2.1.3 Run send_batch.py

```bash
uv run send_batch.py
```

##### before run details
Before running the script: set `BATCH_SIZE`, because final `benchmark_result.enc` as a ciphertext of Accuracy has the format:

\[match_0_sample, match_1_sample, ..., match_i_sample, ..., match_n_sample, 0.5, 0,5, ..., 0.5\]

Where N — Actual dataset batch size

As a consequence, Server-side accepts first J samples result, where J = BATCH_SIZE in data POST request

It means that if J ≤ i < N, then Server accepts only first J samples but not last N

And if J > N, then the server includes 0.5 in the Accuracy metric.

0.5 can be mistakely interpreted as a fallback allignment result, but in current CKKS circuit, it's an output from formula:

```
match = (pred_label * true_label + 1) * 1/2
```

Considering the fact that in every ciphertext the default initial value of every slot equals 0, we can accept that in unsued slots match result will always be:

```
(0*0+1)/2 = 0.5
```

UPD: due to implementation of round_01() function in server and client side, 1 instead 0.5

## TBD:

[ ] BYOM - Create your own Encrypted Model as Model Owner
[ ] Server repository open-source
[ ] Future Plans

<!--
### 2.2 Create your own Encrypted Model (As Model owner)

#### 2.2.1 Generate keys & Encrypt (or use default)

in encrypt_weights.cpp

keys/ folder

```bash

```

#### 2.2.2 -->

