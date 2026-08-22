#include <iostream>
#include <chrono>

using namespace std;

int main() {
    // 1. Variáveis necessárias para rodar o TAC
    long long i = 0;
    long long soma = 0;
    long long t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
    long long a = 0;
    
    cout << "Iniciando a execucao..." << endl;
    

    // 1. O TAC é um código intermediário gerado pelo compilador.
    // 2. O TAC não é código executável, mas sim uma representação abstrata da lógica do programa.
    // 3. O TAC é usado para otimização e geração de código final.
    // 4. O TAC pode ser convertido para código assembly ou código de máquina.
    // 5. O TAC é usado para análise semântica e estática do programa.
    // 6. O TAC é usado para geração de código intermediário.
    // 7. O TAC é usado para geração de código otimizado.
    
    // --- INICIA O CRONÔMETRO
    auto start = chrono::high_resolution_clock::now();
    
    long long MAX = 4294967296;
    // Não esqueça de usar virgula no final de cada linha, exceto para os rótulos (L1, L2, etc.) e if's (ifFalse, ifTrue)
    // COLE O TAC ABAIXO (Abaixo está um exemplo de como o TAC fica em formato C++)
        
        i = 0;
        soma = 0;
L1:
        t1 = i < MAX;
        if(!t1)  goto L2;
        t2 = soma + i;
        soma = t2;
        t3 = i + 1;
        i = t3;
        if(!t1) goto L2;
        t2 = soma + i;
        soma = t2;
        t3 = i + 1;
        i = t3;
        goto L1;
L2:

    // FIM DO TAC
    // ========================================================

    // --- PARA O CRONÔMETRO ---
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;

    cout << "Soma final: " << soma << endl;
    cout << "Valor de a: " << a << endl;
    cout << "Tempo de execucao: " << duration.count() << " milissegundos" << endl;

    return 0;
}