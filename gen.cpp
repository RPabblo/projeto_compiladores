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




// A nossa "memória" do programa compilado
std::vector<InstrucaoTAC> programaTAC;

// Lê a nossa lista de memória e imprime na tela exatamente como os antigos "cout" faziam
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
            // Operações normais (ex: t1 = a + b)
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
        
        // Esta é a nossa "memória" da Propagação de Constantes
        std::map<std::string, std::string> tabelaConstantes; 
        
        for (auto& inst : programaTAC) {
            
            // ==========================================
            // 1. PROPAGAÇÃO (Substituir variáveis)
            // ==========================================
            // Se o arg1 for uma variável que conhecemos o valor, troca pelo número!
            if (tabelaConstantes.count(inst.arg1)) {
                inst.arg1 = tabelaConstantes[inst.arg1];
                modificou = true;
            }
            // Faz o mesmo para o arg2
            if (tabelaConstantes.count(inst.arg2)) {
                inst.arg2 = tabelaConstantes[inst.arg2];
                modificou = true;
            }

            // ==========================================
            // 2. DOBRAMENTO (O que já tínhamos)
            // ==========================================
            if (inst.op == "+" || inst.op == "-" || inst.op == "*" || inst.op == "/") {
                if (isNumber(inst.arg1) && isNumber(inst.arg2)) {
                    float val1 = std::stof(inst.arg1);
                    float val2 = std::stof(inst.arg2);
                    float resultado = 0;

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

            // ==========================================
            // 3. REGISTRO DE CONSTANTES NA MEMÓRIA
            // ==========================================
            // Se a instrução virou uma atribuição de número (Ex: t2 = 10)
            if (inst.op == "=" && isNumber(inst.arg1)) {
                
                // ATENÇÃO: Substitua "inst.res" pelo nome correto do campo 
                // da sua struct InstrucaoTAC que guarda o destino da variável!
                // Pode ser inst.resultado, inst.destino, inst.res, etc.
                tabelaConstantes[inst.dest] = inst.arg1; 
            }
        }
    }



    // ELIMINAÇÃO DE CÓDIGO MORTO (Dead Code Elimination - DCE)
   
    bool modificouDCE = true;
    
    // O while roda até o compilador não achar mais nenhum "lixo" para apagar
    while (modificouDCE) {
        modificouDCE = false;
        
        // Dicionário para anotar quantas vezes cada variável é usada
        std::map<std::string, int> contagemUso;
        
        // Passo 1: Varre o programa e conta quem é lido
        for (const auto& inst : programaTAC) {
            // "goto" puro e "label" não usam variáveis matemáticas
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

        // Passo 2: A Vassoura (Varre e apaga as variáveis não utilizadas)
        // Usamos um iterador (it) porque vamos deletar itens da lista dinamicamente
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

            // 3. Se achou um laço válido, vamos duplicar o miolo!
            if (end_loop > start_loop) {
                std::vector<InstrucaoTAC> miolo;
                
                // Extrai as instruções de dentro do laço (pula a condição ifFalse e o goto)
                // Ajuste os índices '+2' se o seu ifFalse não estiver imediatamente após o label
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