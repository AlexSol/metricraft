Круто. По скрінах бачу, що ти хочеш легкий, безпечний агент збору метрик з плагінами (Wasm/exec/Lua), єдиним JSON-контрактом і API типу /plugins/<name>/run, /plugins/reload, плюс експортер /metrics (Prometheus або textfile). Нижче — стислий, але робочий план + мінімальний скелет.


Архітектура (мінімум компонентів)

1. Scheduler (C++20)
  Розуміє: interval | cron (пізніше) | jitter | timeout | max_concurrency | retry/backoff.

2. Plugin Manager
  - Типи: wasm, exec (bash/python/native), lua (опційно).
  - Сканує plugins/, парсить маніфести, валідить, робить hot-reload.

3. Runtimes (sandbox)
  - wasm: Wasmtime (WASI) через C API/wasmtime-cpp: preopen FS, fuel для CPU, memory cap.
  - exec: окремий процес із setrlimit (CPU/AS/NOFILE), umask, ізоляція користувачем nobody, опційно cgroups/seccomp.
  - lua: окремий lua_State на плагін, доступ лише до whitelisted функцій.

4. Єдиний контракт плагіна
  - Вхід (stdin або функція): ctx_json
  - Вихід (stdout або функція): metrics_json
   // ctx
   {"store_id":"kyiv-001","host":"pos-12","env":{"POS_LOG":"/var/log/pos/*.log"}}

   // output
   {"metrics":[
     {"name":"pos_transactions_total","type":"counter","value":1234,
      "labels":{"store_id":"kyiv-001","pos":"12"}}
   ]}
   Типи: counter|gauge|histogram. Імена/лейбли валідимо (regex + allowlist).

5. API
   - POST /plugins/<name>/run — відразу викликає плагін, повертає JSON.
   - POST /plugins/reload — перескан каталог.
   - GET /metrics — Prometheus формат (або textfile-дроп).
6. Маніфест плагіна (єдиний формат, YAML)
```yml
name: pos_tx
runtime: wasm        # wasm | exec | lua  | python (bind)
entry: plugins/pos_tx/pos_tx.wasm   # або binary/script
schedule:
  interval: 30s
  jitter: 2s
timeout: 5s
resources:
  memory_mb: 32
  cpu_ms: 50
capabilities:
  fs_read:
    - /proc/uptime
    - /var/log/pos/*.log
  network: false
env:
  STORE_ID: kyiv-001

secrets:
  DB_USER: { from: env, name: "METRICRAFT_DB_USER" }
  DB_PASS: { from: env, name: "METRICRAFT_DB_PASS" }

```


## Структура репозиторію

```sh
    metrics-agent/
    ├─ src/
    │  ├─ main.cpp                # точка входу
    │  ├─ scheduler.hpp           # планувальник (interval/jitter/timeout)
    │  ├─ manifest.hpp            # YAML-маніфест плагіна
    │  ├─ plugin.hpp              # інтерфейс рантаймів
    │  ├─ exec_runner.hpp         # запуск зовнішніх плагінів
    │  ├─ py_runner.hpp           # embedded Python (опційно)
    │  ├─ lua_runner.hpp          # embedded Lua (опційно)
    │  ├─ mapper.hpp              # мапінг у Prometheus
    │  ├─ state_store.hpp         # простий JSON-стейт
    │  ├─ textfile_writer.hpp     # атомарний запис *.prom
    │  └─ util.hpp
    ├─ plugins/
    │  ├─ cpu/                    # приклад exec (Python) плагіна
    │  ├─ cpu_lua/                # приклад Lua плагіна
    │  └─ wasm_template_rust/     # шаблон Rust/WASI плагіна
    ├─ tests/
    │  └─ wasm_integration.sh     # інтеграційний тест для WASI-плагіна
    ├─ hardening/
    │  ├─ seccomp/                # exec_strict.json, exec_relaxed.json
    │  └─ cgroupv2/               # create_and_attach.sh, examples.md
    ├─ CMakeLists.txt
    └─ README.md
```

## Маніфест плагіна (єдиний формат, YAML)

```yaml
name: my_plugin
version: 1.0.0
description: A sample plugin
author: Your Name
runtime: wasm        # wasm | exec | lua
entry: plugins/pos_tx/pos_tx.wasm   # або binary/script
schedule:
  interval: 30s
  jitter: 2s
timeout: 5s
resources:
  memory_mb: 32
  cpu_ms: 50
capabilities:
  fs_read:
    - /proc/uptime
    - /var/log/pos/*.log
  network: false
env:
  STORE_ID: kyiv-001

---
# --
# Збірка
# mkdir build && cd build
# cmake -DAGENT_ENABLE_PYTHON=ON -DAGENT_ENABLE_LUA=ON ..
# cmake --build . -j

# # Запуск (за замовчуванням)
# ./agent   # PLUGINS_DIR=./plugins, TEXTFILE_DIR=./out, STATE_DIR=./state

# # Перевірка
# ls ../out/*.prom