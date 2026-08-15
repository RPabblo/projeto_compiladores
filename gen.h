#include <iostream>
#include <sstream>
#include <vector>

#ifndef COMPILER_GENERATOR
#define COMPILER_GENERATOR

#include "ast.h"

Expression * Lvalue(Expression * n);
Expression * Rvalue(Expression * n);

// Estrutura que guarda uma instrução matemática, desvio ou rótulo
struct InstrucaoTAC {
    std::string op;      // Ex: "+", "-", "=", "goto", "ifFalse", "label"
    std::string dest;    // Ex: "t1", "soma"
    std::string arg1;    // Ex: "a", "10", "L1"
    std::string arg2;    // Ex: "b", "L2"

    // Construtor para facilitar a criação da instrução
    InstrucaoTAC(std::string op, std::string dest, std::string arg1, std::string arg2 = "") 
        : op(op), dest(dest), arg1(arg1), arg2(arg2) {}
};

// Avisa aos outros arquivos que essa lista existe
extern std::vector<InstrucaoTAC> programaTAC;

// Função que imprimirá o código no terminal (após otimizarmos)
void ImprimirTAC();

#endif
