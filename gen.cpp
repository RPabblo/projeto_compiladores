#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "error.h"
#include "gen.h"
using std::cout;
using std::endl;

extern Lexer * scanner;

Expression *Lvalue(Expression *n)
{
    if (n->node_type == NodeType::IDENTIFIER)
    {
        return n;
    }
    else if (n->node_type == NodeType::ACCESS)
    {
        Access * a = (Access*) n;
        return new Access(a->type, a->token, a->id, Rvalue(a->expr));
    }
    else
    {
        stringstream ss;
        ss << "Expressão \'" << n->ToString() << "\' não possui valor-l";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}

Expression *Rvalue(Expression *n)
{
    if (n->node_type == NodeType::IDENTIFIER || n->node_type == NodeType::CONSTANT)
    {
        return n;
    }
    else if (n->node_type == NodeType::ARI)
    {   
        Arithmetic * ari = (Arithmetic*) n;
        Temp * t = new Temp(ari->type);
        Expression * e1 = Rvalue(ari->expr1);
        Expression * e2 = Rvalue(ari->expr2);

/*         cout << '\t' << t->ToString() << " = " 
             << e1->ToString() << " " 
             << ari->ToString() << " " 
             << e2->ToString() << endl; */
        programaTAC.push_back(InstrucaoTAC(
            ari->ToString(),   // op (ex: "+")
            t->ToString(),     // dest (ex: "t1")
            e1->ToString(),    // arg1 (ex: "a")
            e2->ToString()     // arg2 (ex: "b")
        ));

        return t;
    }
    else if (n->node_type == NodeType::REL)
    {
        Relational * rel = (Relational*) n;
        Temp * t = new Temp(rel->type);
        Expression * e1 = Rvalue(rel->expr1);
        Expression * e2 = Rvalue(rel->expr2);
/*         cout << '\t' << t->ToString() << " = " 
             << e1->ToString() << " " 
             << rel->ToString() << " " 
             << e2->ToString() << endl; */
        programaTAC.push_back(InstrucaoTAC(
            rel->ToString(),   // op (ex: "<")
            t->ToString(),     // dest (ex: "t1")
            e1->ToString(),    // arg1 (ex: "a")
            e2->ToString()     // arg2 (ex: "b")
        ));

        return t;
    }
    else if (n->node_type == NodeType::LOG)
    {
        Logical * log = (Logical*) n;
        Temp * t = new Temp(log->type);
        Expression * e1 = Rvalue(log->expr1);
        Expression * e2 = Rvalue(log->expr2);
        /*         cout << '\t' << t->ToString() << " = " 
             << e1->ToString() << " " 
             <<  log->ToString() << " " 
             << e2->ToString() << endl;*/
        programaTAC.push_back(InstrucaoTAC(
            log->ToString(),   // op (ex: "&&")
            t->ToString(),     // dest (ex: "t1")
            e1->ToString(),    // arg1 (ex: "a")
            e2->ToString()     // arg2 (ex: "b")
        ));
        return t;
    }
    else if (n->node_type == NodeType::UNARY)
    {
        UnaryExpr * una = (UnaryExpr*) n;
        Temp * t = new Temp(una->type);
        Expression * e = Rvalue(una->expr);
        programaTAC.push_back(InstrucaoTAC(
            una->ToString(),   // op (ex: "!")
            t->ToString(),     // dest (ex: "t1")
            e->ToString(),     // arg1 (ex: "a")
            ""                // arg2 (ex: "")
        )); 
        return t;
    }
    else if (n->node_type == NodeType::ACCESS)
    {
        Access * access = (Access*) n;
        Temp * temp = new Temp(access->type);
        Expression * right = Lvalue(n);
/*                 cout << '\t' << temp->ToString() << " = " 
                    << right->ToString() 
                    << endl; */
        programaTAC.push_back(InstrucaoTAC(
            "=",                // op (ex: "=")
            temp->ToString(),   // dest (ex: "t1")
            right->ToString(),  // arg1 (ex: "a")
            ""                  // arg2 (ex: "")
        ));
        return temp;
    }
    else if (n->node_type == NodeType::ASSIGN)
    {
        Access * acc = (Access*) Lvalue(n);
        Expression * left = Lvalue(acc->id);
        Expression * right = Rvalue(acc->expr);
/*         cout << '\t' 
             << left->ToString()  
             << " = " 
             << right->ToString() 
             << endl; */
        programaTAC.push_back(InstrucaoTAC("=", left->ToString(), right->ToString()));
        
        return right;
    }
    else
    {
        stringstream ss;
        ss << "Expressão \'" << n->ToString() << "\' não possui valor-r";
        throw SyntaxError{scanner->Lineno(), ss.str()};
    }
}



// novos




// A memória do programa compilado
std::vector<InstrucaoTAC> programaTAC;

// Lê a lista de memória e imprime na tela exatamente como os antigos COUT faziam
void ImprimirTAC() {
    for (const auto& inst : programaTAC) {
        if (inst.op == "label") {
            std::cout << inst.arg1 << ":" << std::endl;
        } else if (inst.op == "goto") {
            std::cout << "\tgoto " << inst.arg1 << std::endl;
        } else if (inst.op == "ifFalse" || inst.op == "ifTrue") {
            std::cout << "\t" << inst.op << " " << inst.arg1 << " goto " << inst.arg2 << std::endl;
        } else if (inst.op == "=") {
            std::cout << "\t" << inst.dest << " = " << inst.arg1 << std::endl;
        } else {
            std::cout << "\t" << inst.dest << " = " << inst.arg1 << " " << inst.op << " " << inst.arg2 << std::endl;
        }
    }
}


// Função auxiliar para verificar se a string é um número (inteiro ou float)
bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c) && c != '.') return false;
    }
    return true;
}

// O motor de otimização
void OtimizarTAC() {
    bool modificou = true;
    
    while (modificou) {
        modificou = false;
        
        // Esta é a "memória" da Propagação de Constantes
        std::map<std::string, std::string> tabelaConstantes; 
        
        for (auto& inst : programaTAC) {
            
           // Limpa o mapeamento se houver desvio de fluxo
            if (inst.op == "label" || inst.op == "goto" || inst.op == "ifFalse" || inst.op == "ifTrue") {
                tabelaConstantes.clear();
                continue;
            }
            // 1. PROPAGAÇÃO (Substituir variáveis)
          
            // Se o arg1 for uma variável que o valor é conhecido em tempo de compilação, substituímos pelo valor
            if (tabelaConstantes.count(inst.arg1)) {
                inst.arg1 = tabelaConstantes[inst.arg1];
                modificou = true;
            }
            // Se o arg2 for uma variável que o valor é conhecido em tempo de compilação, substituímos pelo valor
            if (tabelaConstantes.count(inst.arg2)) {
                inst.arg2 = tabelaConstantes[inst.arg2];
                modificou = true;
            }

      
            // 2. DOBRAMENTO DE CONSTANTES (Constant Folding)

            // Se a operação é aritmética e ambos os argumentos são números, podemos calcular o resultado em tempo de compilação
            if (inst.op == "+" || inst.op == "-" || inst.op == "*" || inst.op == "/") {
                if (isNumber(inst.arg1) && isNumber(inst.arg2)) {
                    float val1 = std::stof(inst.arg1);
                    float val2 = std::stof(inst.arg2);
                    float resultado = 0;

                    // Calcula o resultado da operação
                    if (inst.op == "+") resultado = val1 + val2;
                    else if (inst.op == "-") resultado = val1 - val2;
                    else if (inst.op == "*") resultado = val1 * val2;
                    else if (inst.op == "/") {
                        if (val2 != 0) resultado = val1 / val2;
                        else continue; 
                    }

                    std::string str_resultado = (resultado == (int)resultado) 
                                                ? std::to_string((int)resultado) 
                                                : std::to_string(resultado);

                    inst.op = "=";
                    inst.arg1 = str_resultado;
                    inst.arg2 = "";
                    modificou = true;
                }
            }

         
            // 3. REGISTRO DE CONSTANTES NA MEMÓRIA
            
            // Se a instrução é uma atribuição de constante (ex: t1 = 10), registramos que t1 agora tem o valor 10
            if (inst.op == "=" && isNumber(inst.arg1)) {
                
  
                tabelaConstantes[inst.dest] = inst.arg1; 
            }
        }
    }



    // ELIMINAÇÃO DE CÓDIGO MORTO (Dead Code Elimination - DCE)
   
    // A ideia é varrer o programa várias vezes, apagando instruções que escrevem em variáveis temporárias que nunca são lidas
    bool modificouDCE = true;
    
    // O while roda até o compilador não achar mais nenhum "lixo" para apagar
    while (modificouDCE) {
        modificouDCE = false;
        
        // Dicionário para anotar quantas vezes cada variável é usada
        std::map<std::string, int> contagemUso;
        
        // Passo 1: Varre o programa e conta quem é lido
        for (const auto& inst : programaTAC) {
            // Ignora instruções que não leem variáveis (como label e goto) 
            // (Mas ifFalse usa, então ele entra na contagem!)
            if (inst.op != "label" && inst.op != "goto") {
                if (!inst.arg1.empty() && !isNumber(inst.arg1)) {
                    contagemUso[inst.arg1]++;
                }
                if (!inst.arg2.empty() && !isNumber(inst.arg2)) {
                    contagemUso[inst.arg2]++;
                }
            }
        }

        // Passo 2: Varre o programa e apaga instruções que escrevem em variáveis temporárias que nunca são lidas
        // Atenção: usamos um iterador para poder apagar elementos do vetor sem quebrar o loop
        for (auto it = programaTAC.begin(); it != programaTAC.end(); ) {
            
            // Verifica se a instrução grava algum valor em uma variável
            if (it->op == "=" || it->op == "+" || it->op == "-" || it->op == "*" || it->op == "/") {
                

                std::string destino = it->dest; 
                
                // Verifica se a variável de destino é um Temporário (começa com 't' e um número)
                bool isTemporario = (destino.length() >= 2 && destino[0] == 't' && isdigit(destino[1]));
                
                // Se for um temporário E ninguém estiver usando ele (contagem zero)
                if (isTemporario && contagemUso[destino] == 0) {
                    
                    // Apaga a instrução inteira do TAC e ajusta a lista
                    it = programaTAC.erase(it); 
                    modificouDCE = true; // Avisa que apagamos algo, vamos varrer de novo!
                    
                    continue; // Pula para a próxima repetição para não quebrar o iterador
                }
            }
            ++it; // Vai para a próxima instrução
        }
    }
    // FIM DA ELIMINAÇÃO DE CÓDIGO MORTO


    // FASE: LOOP UNROLLING (Fator 2)
    // A ideia é identificar laços simples e duplicar o corpo do laço para reduzir overhead de controle de loop
    for (size_t i = 0; i < programaTAC.size(); i++) {
       
        // 1. Identifica o início do laço (label ou rótulo)
        if (programaTAC[i].op == "label" || programaTAC[i].op.find("L") == 0) {
            size_t start_loop = i;
            size_t end_loop = 0;
            std::string label_name = programaTAC[i].arg1; 
            if (label_name.empty()) label_name = programaTAC[i].op; // Depende da sua struct

            // 2. Procura onde o laço termina (o goto que volta pro início)
            for (size_t j = i + 1; j < programaTAC.size(); j++) {
                if (programaTAC[j].op == "goto" && programaTAC[j].arg1 == label_name) {
                    end_loop = j;
                    break;
                }
            }

            // 3. Se achou um laço válido, vamos duplicar o core do loop
            if (end_loop > start_loop) {
                std::vector<InstrucaoTAC> miolo;
                
                // Extrai as instruções de dentro do laço (pula a condição ifFalse e o goto)
                // O miolo do loop começa depois do label e termina antes do goto
                for (size_t k = start_loop + 2; k < end_loop; k++) {
                    miolo.push_back(programaTAC[k]);
                }

                // Insere o miolo extra antes do salto final
                programaTAC.insert(programaTAC.begin() + end_loop, miolo.begin(), miolo.end());
                
                // Quebra para evitar loop infinito na manipulação do vetor
                break; 
            }
        }
    }
}