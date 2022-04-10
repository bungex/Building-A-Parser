/* Definitions and some functions implementations
 * parse.cpp to be completed
 * Programming Assignment 2
 * Spring 2022
*/

#include "parse.h"
#include "lex.cpp"



map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}


//Stmt is either a WriteLnStmt, ForepeatStmt, IfStmt, or AssigStmt
//Stmt = AssigStmt | IfStmt | WriteStmt | ForStmt 
bool Stmt(istream& in, int& line) {
	bool status;
	//cout << "in ContrlStmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case WRITELN:
		status = WriteLnStmt(in, line);
		//cout << "After WriteStmet status: " << (status? true:false) <<endl;
		break;

	case IF:
		status = IfStmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		
		break;
		
	case FOR:
		status = ForStmt(in, line);
		
		break;
		
		
	default:
		Parser::PushBackToken(t);
		return false;
	}

	return status;
}//End of Stmt


//WriteStmt:= wi, ExpreList 
bool WriteLnStmt(istream& in, int& line) {
	LexItem t;
	//cout << "in WriteStmt" << endl;
	
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression after WriteLn");
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	//Evaluate: print out the list of expressions values

	return ex;
}


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	//cout << "in ExprList and before calling Expr" << endl;
	status = Expr(in, line);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		//cout << "before calling ExprList" << endl;
		status = ExprList(in, line);
		//cout << "after calling ExprList" << endl;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

bool Prog(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	
	if(t == PROGRAM){
		t = Parser::GetNextToken(in,line);
		if(t == IDENT){
			t = Parser::GetNextToken(in,line);
			if(t != SEMICOL){
				ParseError(line, "Missing semicolon in Statement.");
				return false;
			}	
		}
		else{
			ParseError(line,"Missing Program Name.");
			return false;
		}			
	}
	else{
		ParseError(line,"Missing PROGRAM.");
		return false;
	}
		
	t = Parser::GetNextToken(in, line);
			
	if(t == VAR){
		status = DeclBlock(in,line);
		if(!status){
			return false;
		}
	}
	else{
		ParseError(line,"Non-recognizable Declaration Block.");
		ParseError(line,"Incorrect Declaration Section.");
		return false;
	}
	t = Parser::GetNextToken(in, line);
			
	if(t == BEGIN){
		status = ProgBody(in,line);
		if(!status){
			return false;
		}
	}
	
	return status;
	
}

bool ProgBody(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in,line);
	bool status = false;
	
	if(t == IDENT ){
		Parser::PushBackToken(t);
		status = Stmt(in, line);
		if(!status){
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}	
	}
	else if(t == IF){
		Parser::PushBackToken(t);
		status = Stmt(in, line);
		if(!status){
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}
	else if(t == FOR){
		Parser::PushBackToken(t);
		status = Stmt(in, line);
		if(!status){
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}
	t = Parser::GetNextToken(in,line);
	if(t == FOR){
		Parser::PushBackToken(t);
		status = Stmt(in, line);
		if(!status){
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}
	if(t != SEMICOL){
		if(t == ELSE){
			t = Parser::GetNextToken(in,line);
			if(t == IDENT || t == IF){
				Parser::PushBackToken(t);
				status = Stmt(in, line);
				if(!status){
					return false;
				}
			}
		}
		else{
			ParseError(line-1, "Missing semicolon in Statement.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}	
	}
	
	t = Parser::GetNextToken(in,line);
	if(t == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}
	if(t == WRITELN){
		Parser::PushBackToken(t);
		status = Stmt(in, line);
		if(!status){
			ParseError(line, "Syntactic error in Program Body.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}
	if(t == END){
		return true;
	}
	else{
		Parser::PushBackToken(t);
		status = ProgBody(in, line);
		if(!status){
			return false;
		}
	}
	
	
	
	return true;
	
}


bool DeclBlock(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in,line);
	bool status = false;
	
	if(t == IDENT){
		Parser::PushBackToken(t);
		status = DeclStmt(in,line);
	}
	
	if(!status){
		ParseError(line, "Incorrect Declaration Section.");
		return false;
	}
	return true;
	
}

bool DeclStmt(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	bool status = true;
	string id;
	
	if(t == IDENT){
		id = t.GetLexeme();
		if(!(defVar.find(id)->second)){
			defVar[id] = true;
		}
		else{
			ParseError(line, "Variable Redefination");
			ParseError(line, "Incorrect variable in Declaration Statement.");
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
	}
	
	t = Parser::GetNextToken(in, line);
	
	if(t == COMMA){
		status = DeclStmt(in, line);
		if(!status){
			return false;
		}
	}
	else if(t == COLON){
		t = Parser::GetNextToken(in, line);
		if(t == REAL || t == INTEGER || t == STRING){
			for(auto itr = defVar.begin(); itr != defVar.end(); ++itr){
				auto i = SymTable.find(itr->first);
				if(i == SymTable.end()){
					SymTable[itr->first] = t.GetToken();
				}
			}	
		}
		else{
			ParseError(line, "Incorrect Declaration Type.");
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
		
		t = Parser :: GetNextToken(in, line);
	
		if(t != SEMICOL){
			ParseError(line, "Missing semicolon");
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}
		
	}
	else {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}
	
	
	t = Parser :: GetNextToken(in, line);

	if (t == IDENT){
		Parser::PushBackToken(t);
		status = DeclStmt(in, line);
		if(!status){
			return false;
		}
		return true;
	}	
	Parser::PushBackToken(t);
	return true;
		
}
bool IfStmt(istream& in, int& line){
	LexItem t;	
	
	bool status = LogicExpr(in, line);
	if(!status){
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if(t == THEN){
		status = Stmt(in, line);
		if(!status){
			ParseError(line,"Missing Statement for If-Stmt Then-Part");
			return false;
		}
	}
	t = Parser::GetNextToken(in, line);
	if(t == ELSE){
		status = Stmt(in, line);
		if(!status){
			ParseError(line,"Missing Statement for If-Stmt Then-Part");
			return false;
		}
	}
	else{
		Parser::PushBackToken(t);
		return status;
	}
	
	
	return status;
}

bool LogicExpr(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	
	if( t != LPAREN ) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	bool status = Expr(in, line);
	
	if(!status){
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if(t != EQUAL &&  t != GTHAN && t != LTHAN ){
		ParseError(line, "Invalid statement");
		return false;
	}
	status = Expr(in, line);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	t = Parser::GetNextToken(in, line);
	if( t != RPAREN ) {
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}
	
	return true;
}


bool AssignStmt(istream& in, int& line){
	
	LexItem t;
	bool status;
	
	status = Var(in, line);
	
	if(status){
		t = Parser::GetNextToken(in, line);
		
		if(t == ASSOP){
			status = Expr(in, line);
			if(!status){
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}
		}
		
		else if(t == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		
	}
	else{
		ParseError(line,"Missing Assignment Operator :=");
		return false;
	}
	
	
	
	return status;
}

bool Var(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	string id;
	if(t == IDENT){
		id = t.GetLexeme();
		if(!(defVar.find(id)->second)){
			ParseError(line, "Indeclared Variable");
			return false;
		}
		return true;
	}
	else if(t == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
	
}

bool Expr(istream& in, int& line){

	LexItem t;
	bool status = Term(in, line);
	if(!status){
		return false;
	}
	
	t = Parser::GetNextToken(in, line);
	if(t == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    while(t == PLUS || t == MINUS){
    	status = Term(in, line);
    	if(!status){
    		ParseError(line, "Missing operand after operator");
    		return false;
		}
		t = Parser::GetNextToken(in, line);
		if(t == ERR){
			ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
		}
	}
	Parser :: PushBackToken(t);
	return true;
	
	
}

bool Term(istream& in, int& line){
	
	bool status = SFactor(in, line);
	
	if(!status){
		return false;
	}
	
	LexItem t = Parser::GetNextToken(in, line);
	if(t == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    
    while(t == MULT || t == DIV){
    	status = SFactor(in, line);
    	if(!status){
    		ParseError(line, "Missing operand after operator");
    		return false;
		}
		t = Parser::GetNextToken(in, line);
		if(t == ERR){
			ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << t.GetLexeme() << ")" << endl;
            return false;
		}
	}
	Parser :: PushBackToken(t);
	return true;
    
		
}

bool SFactor(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	
	int sign = 0;
	
	if(t == MINUS){
		sign = -1;
	}
	else if(t == PLUS){
		sign = 1;
	}
	else{
		Parser::PushBackToken(t);
	}
	status = Factor(in, line, sign);	
	return status;
}

bool Factor(istream& in, int& line, int sign){
	LexItem t = Parser::GetNextToken(in, line);
	
	if(t == IDENT){
		string id = t.GetLexeme();
		
		if(!(defVar.find(id)->second)){
			ParseError(line, "Undefind Variable");
			return false;
		}
		return true;		
	}
	else if(t == ICONST){
		return true;
	}
	else if(t == SCONST){
		return true;
	}
	else if(t == RCONST){
		return true;
	}
	else if(t == LPAREN){
		bool status = Expr(in, line);
		if(!status){
			ParseError(line, "Missing expression after (");
			return false;
		}
		if(Parser::GetNextToken(in, line) == RPAREN){
			return status;
		}
		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(t == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		ParseError(line, "Missing expression after relational operator");
		return false;
	}
	return true;
	
}
bool ForStmt(istream& in, int& line){
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	
	if(t == IDENT){
		Parser :: PushBackToken(t);
		status = Var(in, line);
		if(!status){
			cout << "here" << endl;
			ParseError(line, "For Statement Syntax Error");
			return false;
		}
	}

	t = Parser::GetNextToken(in, line);
	if(t == ASSOP){
		t = Parser::GetNextToken(in, line);
		if(t == ICONST){
			t = Parser::GetNextToken(in, line);
			if(t == DOWNTO || t == TO){
				t = Parser::GetNextToken(in, line);
				if(t == ICONST){
					t = Parser::GetNextToken(in, line);
					if(t == DO){
						t = Parser::GetNextToken(in, line);
						if(t == IDENT){
							Parser :: PushBackToken(t);
							status = Stmt(in, line);
							if(!status){
								ParseError(line, "For Statement Syntax Error");
								return false;
							}
						}
					}
					else{
						ParseError(line, "For Statement Syntax Error");
						return false;
					}
				}
				else{
					ParseError(line, "Missing Termination Value in For Statement");
					return false;
				}
			}
			else{
				ParseError(line, "For Statement Syntax Error");
				return false;
			}
		}
		else{
			ParseError(line, "For Statement Syntax Error");
			return false;
		}
		
		
	}
	else{
		ParseError(line, "For Statement Syntax Error");
		return false;
	}

	
	return true;
	
}

