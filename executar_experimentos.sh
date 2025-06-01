#!/bin/bash

# Compilar
gcc -o sequencial src/sequencial.c
gcc -fopenmp -o paralelo src/paralelo.c

# Tamanhos de matriz e threads
sizes=(256 512 768)
threads=(2 4 8 16)

# Limpa logs antigos
rm -f log/*.log

echo "Iniciando experimentos..."

# Função para calcular média
media() {
    awk '{s+=$1} END {print s/NR}'
}

# Teste SEQUENCIAL
for size in "${sizes[@]}"; do
    echo "Testando versão sequencial com matriz ${size}x${size}"
    logfile="log/seq_${size}.log"
    for i in {1..10}; do
        ./sequencial entradas/entrada_${size}.txt saidas/saida_seq_${size}.txt | grep "Tempo" | awk '{print $3}' >> "$logfile"
    done
    echo -n "Média sequencial (${size}x${size}): "
    media < "$logfile"
done

# Teste PARALELO
for size in "${sizes[@]}"; do
    for t in "${threads[@]}"; do
        echo "Testando versão paralela com matriz ${size}x${size} e ${t} threads"
        logfile="log/par_${size}_${t}.log"
        export OMP_NUM_THREADS=$t
        for i in {1..10}; do
            ./paralelo entradas/entrada_${size}.txt saidas/saida_par_${size}_${t}.txt ${t} | grep "Tempo" | awk '{print $2}' >> "$logfile"
            #./paralelo entradas/entrada_${size}.txt saidas/saida_par_${size}_${t}.txt ${t}
        done
        echo -n "Média paralela (${size}x${size}, ${t} threads): "
        media < "$logfile"
    done
done
echo "Experimentos concluídos."
