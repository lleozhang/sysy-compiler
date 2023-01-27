%code requires {
  #include <memory>
  #include <string>
  #include <iostream>
  #include <cstring>
  #include "AST.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
#include <cstring>
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }
// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  char char_val;
  BaseAST *ast_val;
}
// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE VOID EQ NEQ GEQ LEQ LOR LAND
%token <str_val> IDENT 
%token <int_val> INT_CONST
%token <char_val> OP
// 非终结符的类型定义
%type <ast_val> Compunit FuncDef Block Stmt Exp UnaryExp PrimaryExp SinExp NumExp MulExp MultiExp AddExp MuladdExp
%type <ast_val> RelExp MulRelExp EqExp MulEqExp LandExp MulAndExp LorExp MulLorExp BlockItem Decl ConstDecl ArrayFuncPara ArrayParaSize
%type <ast_val> ConstInitVal ConstDef Lval ConstExp MulConstDecl MulConstDef MulBlockItem SinBlockItem ArrPara MulArrPara
%type <ast_val> VarDecl VarDef SinVarDef SinVarName MulVarName MulVarDef InitVal LeVal AllLval AllLeval ArrLval ArrLeval
%type <ast_val> IfStmt SinIfStmt MulIfStmt WhileStmt ConWhile SinCompUnit MulCompUnit GloDecl ConstArrVal VarArrVal
%type <ast_val> FuncParas SinFuncPara MulFuncPara FuncExp CallPara SinCallPara MulCallPara ArraySize MulArraySize
%type <ast_val> ConstArrayDef ConstArrayVal MulConArrVal VarArrayDef SinNameVarArrDef MulNameVarArrDef VarArrInitVal MulVarArrInitVal
%type <int_val> Number


%%
// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值


CompUnit
  : Compunit {
    auto compunit = make_unique<CompUnit>();
    compunit->comp_unit = unique_ptr<BaseAST>($1);
    ast = move(compunit);
  }
  ;

Compunit
  : SinCompUnit {
    auto ast = new Compunit();
    ast->compunit = unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulCompUnit {
    auto ast = new Compunit();
    ast->compunit = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinCompUnit
  : GloDecl {
    auto ast=new SinCompUnit();
    ast->func_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }|FuncDef {
    auto ast=new SinCompUnit();
    ast->func_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

GloDecl
  : Decl {
    auto ast=new GloDecl();
    ast->glo_decl=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulCompUnit
  : SinCompUnit Compunit {
    auto ast=new MulCompUnit();
    ast->sin_comp_unit=unique_ptr<BaseAST>($1);
    ast->mul_comp_unit=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

FuncDef
  : INT IDENT '('  ')' Block {
    auto ast = new FuncDef();
    ast->func_type = "int";
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }|INT IDENT '(' FuncParas ')' Block {
    auto ast = new FuncDef();
    ast->func_type = "int";
    ast->ident = *unique_ptr<string>($2);
    ast->func_para=unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }|VOID IDENT '('  ')' Block {
    auto ast = new FuncDef();
    ast->func_type = "void";
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }|VOID IDENT '(' FuncParas ')' Block {
    auto ast = new FuncDef();
    ast->func_type = "void";
    ast->ident = *unique_ptr<string>($2);
    ast->func_para=unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

FuncParas
  : SinFuncPara {
    auto ast=new FuncParas();
    ast->func_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulFuncPara {
    auto ast=new FuncParas();
    ast->func_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }|ArrayFuncPara {
    auto ast=new FuncParas();
    ast->func_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinFuncPara 
  : INT IDENT {
    auto ast=new SinFuncPara();
    ast->IDENT=*unique_ptr<string>($2);
    $$=ast;
  }
  ;

ArrayFuncPara
  : INT IDENT ArrayParaSize {
    auto ast=new ArrayFuncPara();
    ast->IDENT=*unique_ptr<string>($2);
    ast->siz=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

MulFuncPara
  : SinFuncPara ',' FuncParas {
    auto ast=new MulFuncPara();
    ast->sin_func_para=unique_ptr<BaseAST>($1);
    ast->mul_func_para=unique_ptr<BaseAST>($3);
    $$=ast;
  }|ArrayFuncPara ',' FuncParas {
    auto ast=new MulFuncPara();
    ast->sin_func_para=unique_ptr<BaseAST>($1);
    ast->mul_func_para=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

ArrayParaSize
  : '[' ']' {
    auto ast=new ArrayParaSize();
    $$=ast;
  }|'[' ']' ArraySize{
    auto ast=new ArrayParaSize();
    ast->array_para_size=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

Block
  : '{' BlockItem '}' {
    auto ast=new Block();
    ast->block = unique_ptr<BaseAST>($2);
    ast->typ=0;
    $$=ast;
  }|'{' '}'
  {
    auto ast=new Block();
    ast->typ=1;
    $$=ast;
  }
  ;

BlockItem
  : MulBlockItem{
    auto ast=new BlockItem();
    ast->block_item=unique_ptr<BaseAST>($1);
    $$=ast;
  }|SinBlockItem{
    auto ast=new BlockItem();
    ast->block_item=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;


MulBlockItem
  : SinBlockItem BlockItem{
    auto ast=new MulBlockItem();
    ast->sin_item=unique_ptr<BaseAST>($1);
    ast->mul_item=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

SinBlockItem
  : Stmt {
    auto ast=new SinBlockItem();
    ast->sin_block_item=unique_ptr<BaseAST>($1);
    $$=ast;
  }|Decl {
    auto ast=new SinBlockItem();
    ast->sin_block_item=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

Decl
  : ConstDecl {
    auto ast=new Decl();
    ast->decl=unique_ptr<BaseAST>($1);
    $$=ast;
  }|VarDecl {
    auto ast=new Decl();
    ast->decl=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

ConstDecl
  : CONST INT MulConstDef ';'{
    auto ast=new ConstDecl();
    ast->const_decl=unique_ptr<BaseAST>($3);
    $$=ast;
  }|CONST INT ConstDef ';'
  {
    auto ast=new ConstDecl();
    ast->const_decl=unique_ptr<BaseAST>($3);
    $$=ast;
  }|CONST INT ConstArrayDef ';'{
    auto ast=new ConstDecl();
    ast->const_decl=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

MulConstDef
  : ConstDef MulConstDecl{
    auto ast=new MulConstDef();
    ast->const_def=unique_ptr<BaseAST>($1);
    ast->mul_const_dcl=unique_ptr<BaseAST>($2);
    $$=ast;
  }|ConstArrayDef MulConstDecl{
    auto ast=new MulConstDef();
    ast->const_def=unique_ptr<BaseAST>($1);
    ast->mul_const_dcl=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

MulConstDecl
  : ',' MulConstDef{
    auto ast=new MulConstDecl();
    ast->mul_const_def=unique_ptr<BaseAST>($2);
    $$=ast;
  }|',' ConstDef {
    auto ast=new MulConstDecl();
    ast->mul_const_def=unique_ptr<BaseAST>($2);
    $$=ast;
  }|',' ConstArrayDef {
    auto ast=new MulConstDecl();
    ast->mul_const_def=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

ConstArrayDef 
  : IDENT ArraySize '=' ConstArrayVal {
    auto ast=new ConstArrayDef();
    ast->IDENT=*unique_ptr<string>($1);
    ast->siz=unique_ptr<BaseAST>($2);
    ast->const_array_val=unique_ptr<BaseAST>($4);
    $$=ast;
  }
  ;

ArraySize
  : '[' ConstExp ']'{
    auto ast=new ArraySize();
    ast->array_size=unique_ptr<BaseAST>($2);
    $$=ast;
  }|MulArraySize {
    auto ast=new ArraySize();
    ast->array_size=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulArraySize
  : '[' ConstExp ']' ArraySize{
    auto ast=new MulArraySize();
    ast->sin_array_size=unique_ptr<BaseAST>($2);
    ast->mul_array_size=unique_ptr<BaseAST>($4);
    $$=ast;
  }
  ;

ConstArrayVal
  : '{' ConstArrVal '}'{
    auto ast=new ConstArrayVal();
    ast->const_array_val=unique_ptr<BaseAST>($2);
    $$=ast;
  }|'{' '}' {
    auto ast=new ConstArrayVal();
    $$=ast;
  }
  ;

ConstArrVal
  : ConstInitVal{
    auto ast=new ConstArrVal();
    ast->const_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulConArrVal {
    auto ast=new ConstArrVal();
    ast->const_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }|ConstArrayVal {
    auto ast=new ConstArrVal();
    ast->const_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulConArrVal
  : ConstInitVal ',' ConstArrVal {
    auto ast=new MulConArrVal();
    ast->sin_con_arr_val=unique_ptr<BaseAST>($1);
    ast->mul_con_arr_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }|ConstArrayVal ',' ConstArrVal {
    auto ast=new MulConArrVal();
    ast->sin_con_arr_val=unique_ptr<BaseAST>($1);
    ast->mul_con_arr_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal{
    auto ast=new ConstDef();
    ast->IDENT=*unique_ptr<string>($1);
    ast->const_init_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

ConstInitVal 
  : ConstExp{
    auto ast=new ConstInitVal();
    ast->const_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

AllLval
  : Lval {
    auto ast=new AllLval();
    ast->all_lval=unique_ptr<BaseAST>($1);
    $$=ast;
  }|ArrLval {
    auto ast=new AllLval();
    ast->all_lval=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

ArrLval
  : IDENT ArrPara {
    auto ast=new ArrLval();
    ast->IDENT=*unique_ptr<string>($1);
    ast->pos_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

AllLeval 
  : LeVal {
    auto ast=new AllLeval();
    ast->all_leval=unique_ptr<BaseAST>($1);
    $$=ast;
  }|ArrLeval {
    auto ast=new AllLeval();
    ast->all_leval=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

ArrLeval
  : IDENT ArrPara {
    auto ast=new ArrLeval();
    ast->IDENT=*unique_ptr<string>($1);
    ast->pos_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

ArrPara
  : '[' Exp ']' {
    auto ast=new ArrPara();
    ast->arr_para=unique_ptr<BaseAST>($2);
    $$=ast;
  }|MulArrPara {
    auto ast=new ArrPara();
    ast->arr_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulArrPara
  : '[' Exp ']' ArrPara {
    auto ast=new MulArrPara();
    ast->sin_arr_para=unique_ptr<BaseAST>($2);
    ast->mul_arr_para=unique_ptr<BaseAST>($4);
    $$=ast;
  }
  ;

Lval
  : IDENT{
    auto ast=new Lval();
    ast->IDENT=*unique_ptr<string>($1);
    $$=ast;
  }
  ;

LeVal
  : IDENT{
    auto ast=new LeVal();
    ast->IDENT=*unique_ptr<string>($1);
    $$=ast;
  }
  ;

ConstExp
  : Exp {
    auto ast=new ConstExp();
    ast->const_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

VarDecl
  : INT VarDef ';' {
    auto ast=new VarDecl();
    ast->var_decl=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

VarDef
  : SinVarDef {
    auto ast=new VarDef();
    ast->var_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulVarDef {
    auto ast=new VarDef();
    ast->var_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinVarDef 
  : SinVarName {
    auto ast=new SinVarDef();
    ast->sin_var_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulVarName {
    auto ast=new SinVarDef();
    ast->sin_var_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }|VarArrayDef {
    auto ast=new SinVarDef();
    ast->sin_var_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

VarArrayDef
  : SinNameVarArrDef{
    auto ast=new VarArrayDef();
    ast->var_array_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulNameVarArrDef{
    auto ast=new VarArrayDef();
    ast->var_array_def=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinNameVarArrDef
  : IDENT ArraySize {
    auto ast=new SinNameVarArrDef();
    ast->IDENT=*unique_ptr<string>($1);
    ast->siz=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

MulNameVarArrDef 
  : IDENT ArraySize '='  VarArrInitVal  {
    auto ast=new MulNameVarArrDef();
    ast->IDENT=*unique_ptr<string>($1);
    ast->siz=unique_ptr<BaseAST>($2);
    ast->init_val=unique_ptr<BaseAST>($4);
    $$=ast;
  }
  ;

VarArrInitVal
  : '{' VarArrVal '}' {
    auto ast=new VarArrInitVal();
    ast->var_arr_init_val=unique_ptr<BaseAST>($2);
    $$=ast;
  }|'{' '}' {
    auto ast=new VarArrInitVal();
    $$=ast;
  }
  ;

VarArrVal
  : InitVal {
    auto ast=new VarArrVal();
    ast->var_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulVarArrInitVal {
    auto ast=new VarArrVal();
    ast->var_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }|VarArrInitVal {
    auto ast=new VarArrVal();
    ast->var_arr_val=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulVarArrInitVal 
  : InitVal ',' VarArrVal {
    auto ast=new MulVarArrInitVal();
    ast->sin_var_arr_init_val=unique_ptr<BaseAST>($1);
    ast->mul_var_arr_init_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }|VarArrInitVal ',' VarArrVal{
    auto ast=new MulVarArrInitVal();
    ast->sin_var_arr_init_val=unique_ptr<BaseAST>($1);
    ast->mul_var_arr_init_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

SinVarName
  : IDENT {
    auto ast=new SinVarName();
    ast->IDENT=*unique_ptr<string>($1);
    $$=ast;
  }
  ;

MulVarName
  : IDENT '=' InitVal {
    auto ast=new MulVarName();
    ast->IDENT=*unique_ptr<string>($1);
    ast->init_val=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

InitVal
  : Exp {
    auto ast=new InitVal();
    ast->init_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulVarDef 
  : SinVarDef ',' VarDef {
    auto ast=new MulVarDef();
    ast->sin_var=unique_ptr<BaseAST>($1);
    ast->mul_var=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

Stmt
  : RETURN Exp ';'{
    auto ast=new Stmt();
    ast->exp= unique_ptr<BaseAST>($2);
    ast->typ=0;
    $$=ast;
  }|AllLeval '=' Exp ';'{
    auto ast=new Stmt();
    ast->lval=unique_ptr<BaseAST>($1);
    ast->exp=unique_ptr<BaseAST>($3);
    ast->typ=1;
    $$=ast;
  }|Block {
    auto ast=new Stmt();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->typ=3;
    $$=ast;
  }|Exp ';'{
    auto ast=new Stmt();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->typ=2;
    $$=ast;
  }|';'
  {
    auto ast=new Stmt();
    ast->typ=4;
    $$=ast;
  }|IfStmt
  {
    auto ast=new Stmt();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->typ=5;
    $$=ast;
  }|WhileStmt
  {
    auto ast=new Stmt();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->typ=6;
    $$=ast;
  }|ConWhile
  {
    auto ast=new Stmt();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->typ=7;
    $$=ast;
  }|RETURN ';'
  {
    auto ast=new Stmt();
    ast->typ=8;
    $$=ast;
  }
  ;

IfStmt
  : SinIfStmt {
    auto ast=new IfStmt();
    ast->if_stm=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulIfStmt {
    auto ast=new IfStmt();
    ast->if_stm=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinIfStmt
  : IF '(' Exp ')' Stmt {
    auto ast=new SinIfStmt();
    ast->if_exp=unique_ptr<BaseAST>($3);
    ast->if_stmt=unique_ptr<BaseAST>($5);
    $$=ast;
  }
  ;

MulIfStmt
  : IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast=new MulIfStmt();
    ast->if_exp=unique_ptr<BaseAST>($3);
    ast->if_stmt=unique_ptr<BaseAST>($5);
    ast->el_stmt=unique_ptr<BaseAST>($7);
    $$=ast;
  }
  ;

WhileStmt
  : WHILE '(' Exp ')' Stmt {
    auto ast=new WhileStmt();
    ast->while_exp=unique_ptr<BaseAST>($3);
    ast->while_stmt=unique_ptr<BaseAST>($5);
    $$=ast;
  }
  ;

ConWhile 
  : CONTINUE ';' {
    auto ast=new ConWhile();
    ast->str="continue";
    $$=ast;
  }|BREAK ';' {
    auto ast=new ConWhile();
    ast->str="break";
    $$=ast;
  }
  ;



Exp
  : LorExp{
    auto ast=new Exp();
    ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

UnaryExp
: PrimaryExp{
    auto ast=new UnaryExp();
    ast->un_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|SinExp
  {
    auto ast=new UnaryExp();
    ast->un_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|FuncExp
  {
    auto ast=new UnaryExp();
    ast->un_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

FuncExp
  : IDENT '(' ')'{
    auto ast=new FuncExp();
    ast->IDENT=*unique_ptr<string>($1);
    ast->typ=0;
    $$=ast;
  }|IDENT '(' CallPara ')'{
    auto ast=new FuncExp();
    ast->IDENT=*unique_ptr<string>($1);
    ast->call_para=unique_ptr<BaseAST>($3);
    ast->typ=1;
    $$=ast;
  }
  ;

CallPara 
  : SinCallPara {
    auto ast=new CallPara();
    ast->call_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }| MulCallPara {
    auto ast=new CallPara();
    ast->call_para=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

SinCallPara 
  : Exp {
    auto ast=new SinCallPara();
    ast->para_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulCallPara 
  : SinCallPara ',' CallPara {
    auto ast=new MulCallPara();
    ast->sin_call_para=unique_ptr<BaseAST>($1);
    ast->mul_call_para=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

PrimaryExp
:'(' Exp ')'{
    auto ast=new PrimaryExp();
    ast->p_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }|NumExp {
      auto ast=new PrimaryExp();
      ast->p_exp=unique_ptr<BaseAST>($1);
      $$=ast;
  }|AllLval {
    auto ast=new PrimaryExp();
    ast->p_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

NumExp
: Number {
    auto ast=new NumExp();
    ast->num=$1;
    $$=ast;
  }
  ;

Number
: INT_CONST {
    $$=$1;
  }
  ;

SinExp
: '+' UnaryExp {
    auto ast=new SinExp();
    ast->una_op='+';
    ast->una_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }|'-' UnaryExp {
    auto ast=new SinExp();
    ast->una_op='-';
    ast->una_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }|'!' UnaryExp {
    auto ast=new SinExp();
    ast->una_op='!';
    ast->una_exp=unique_ptr<BaseAST>($2);
    $$=ast;
  }
  ;

MulExp
: UnaryExp {
    auto ast=new MulExp();
    ast->mul_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MultiExp{
    auto ast=new MulExp();
    ast->mul_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  } 
  ;

MultiExp
: MulExp '*' UnaryExp{
    auto ast=new MultiExp();
    ast->mu_exp=unique_ptr<BaseAST>($1);
    ast->op='*';
    ast->un_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|MulExp '/' UnaryExp{
    auto ast=new MultiExp();
    ast->mu_exp=unique_ptr<BaseAST>($1);
    ast->op='/';
    ast->un_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|MulExp '%' UnaryExp{
    auto ast=new MultiExp();
    ast->mu_exp=unique_ptr<BaseAST>($1);
    ast->op='%';
    ast->un_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;


AddExp
: MulExp{
    auto ast=new AddExp();
    ast->add_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MuladdExp{
    auto ast=new AddExp();
    ast->add_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MuladdExp
: AddExp '+' MulExp{
    auto ast=new MuladdExp();
    ast->ad_exp=unique_ptr<BaseAST>($1);
    ast->op='+';
    ast->mult_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|AddExp '-' MulExp{
    auto ast=new MuladdExp();
    ast->ad_exp=unique_ptr<BaseAST>($1);
    ast->op='-';
    ast->mult_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;


RelExp
: AddExp {
    auto ast=new RelExp();
    ast->rel_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulRelExp {
    auto ast=new RelExp();
    ast->rel_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulRelExp
: RelExp '<' AddExp {
    auto ast=new MulRelExp();
    ast->re_exp=unique_ptr<BaseAST>($1);
    ast->op="<";
    ast->ad_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|RelExp '>' AddExp {
    auto ast=new MulRelExp();
    ast->re_exp=unique_ptr<BaseAST>($1);
    ast->op=">";
    ast->ad_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|RelExp LEQ AddExp {
    auto ast=new MulRelExp();
    ast->re_exp=unique_ptr<BaseAST>($1);
    ast->op="<=";
    ast->ad_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|RelExp GEQ AddExp {
    auto ast=new MulRelExp();
    ast->re_exp=unique_ptr<BaseAST>($1);
    ast->op=">=";
    ast->ad_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

EqExp
: RelExp {
    auto ast=new EqExp();
    ast->eq_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulEqExp {
    auto ast=new EqExp();
    ast->eq_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulEqExp
: EqExp EQ RelExp {
    auto ast=new MulEqExp();
    ast->e_exp=unique_ptr<BaseAST>($1);
    ast->op="==";
    ast->re_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }|EqExp NEQ RelExp {
    auto ast=new MulEqExp();
    ast->e_exp=unique_ptr<BaseAST>($1);
    ast->op="!=";
    ast->re_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

LandExp
: EqExp {
    auto ast=new LandExp();
    ast->land_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulAndExp {
    auto ast=new LandExp();
    ast->land_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulAndExp
: LandExp LAND EqExp {
    auto ast=new MulAndExp();
    ast->and_exp=unique_ptr<BaseAST>($1);
    ast->op="&&";
    ast->e_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;

LorExp
: LandExp {
    auto ast=new LorExp();
    ast->lor_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }|MulLorExp {
    auto ast=new LorExp();
    ast->lor_exp=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

MulLorExp
: LorExp LOR LandExp {
    auto ast=new MulLorExp();
    ast->lo_exp=unique_ptr<BaseAST>($1);
    ast->op="||";
    ast->lan_exp=unique_ptr<BaseAST>($3);
    $$=ast;
  }
  ;
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  
	extern int yylineno;	// defined and maintained in lex
	extern char *yytext;	// defined and maintained in lex
	int len=strlen(yytext);
	int i;
	char buf[512]={0};
	for (i=0;i<len;++i)
	{
		sprintf(buf,"%s%d ",buf,yytext[i]);
	}
	fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);

}