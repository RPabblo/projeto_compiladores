#include <iostream>
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