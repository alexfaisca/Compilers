%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
#include <vector>
#include <cdk/types/functional_type.h>
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /*
  ion type */
  //-- don't change *any* of these --- END!

  int                   i;  /* integer value */
  double                d;  /* double value */
  std::string          *s;  /* symbol name or string literal */
  cdk::basic_node      *node;      /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;

  mml::block_node      *block;

  std::vector<std::shared_ptr<cdk::basic_type>> *ts;
};

%token <i> tINTEGER
%token <d> tREAL
%token <s> tIDENTIFIER tSTRING
%token <expression> tNULLPTR
%token tWHILE tIF tBEGIN tEND tWRITE tWRITELN tREAD tARROW
%token tTYPE_STRING tTYPE_INT tTYPE_REAL tTYPE_VOID tTYPE_AUTO
%token tPUBLIC tFORWARD tFOREIGN tPRIVATE
%token tSTOP tNEXT tRETURN tRECURSION
%token tSIZEOF tADDRESSOF

%type <node> instruction vardec argdec stop next return program file iffalse inst
%type <sequence> instructions args argdecs vardecs
%type <expression> expr opt_initializer integer real fdef fcall
%type <lvalue> lval
%type <type> data_type func_type
%type <block> block
%type <s> string
%type <ts> type_seq opt_type_seq

%nonassoc tIFX
%nonassoc tIF
%nonassoc tELIF
%nonassoc tELSE
%nonassoc tWHILE
%nonassoc tUNARY

%right '='
%left tOR
%left tAND
%left '<' '>' tGE tLE tEQ tNE
%left '+' '-'
%left '*' '/' '%'

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file:                                                       { compiler->ast(new cdk::sequence_node(LINE)); }
    | vardecs                                               { compiler->ast(new cdk::sequence_node(LINE, $1)); }
    | program                                               { compiler->ast(new cdk::sequence_node(LINE, $1)); }
    | vardecs program                                       { compiler->ast(new cdk::sequence_node(LINE, $2, $1)); }

program   : tBEGIN tEND                                     { $$ = new mml::program_node(LINE, new mml::block_node(LINE, nullptr, nullptr)); }
          | tBEGIN vardecs tEND                             { $$ = new mml::program_node(LINE, new mml::block_node(LINE, $2, nullptr)); }
          | tBEGIN instructions tEND                        { $$ = new mml::program_node(LINE, new mml::block_node(LINE, nullptr, $2)); }
          | tBEGIN vardecs instructions tEND                { $$ = new mml::program_node(LINE, new mml::block_node(LINE, $2, $3)); }
          ;

vardecs : vardec ';'                                        { $$ = new cdk::sequence_node(LINE, $1); }
        | vardecs vardec ';'                                { $$ = new cdk::sequence_node(LINE, $2, $1); }
        ;

instructions : instruction                                  { $$ = new cdk::sequence_node(LINE, $1); }
             | instructions instruction                     { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;         

instruction : expr ';'                                      { $$ = new mml::evaluation_node(LINE, $1); }
            | args tWRITE                                   { $$ = new mml::print_node(LINE, $1, false); }
            | args tWRITELN                                 { $$ = new mml::print_node(LINE, $1, true); }
            | tWHILE '('expr')' inst                        { $$ = new mml::while_node(LINE, $3, $5); }
            | tIF '('expr')' inst %prec tIFX                { $$ = new mml::if_node(LINE, $3, $5); }
            | tIF '('expr')' inst iffalse                   { $$ = new mml::if_else_node(LINE, $3, $5, $6); }
            | block                                         { $$ = $1; }
            | stop ';'                                      { $$ = $1; }
            | next ';'                                      { $$ = $1; }
            | return ';'                                    { $$ = $1; }
            ;

iffalse   : tELIF '('expr')' inst %prec tIF                 { $$ = new mml::if_node(LINE, $3, $5); }
          | tELIF '('expr')' inst iffalse                   { $$ = new mml::if_else_node(LINE, $3, $5, $6); }
          | tELSE instruction                               { $$ = $2; }
          ;

inst : expr ';'                                             { $$ = new mml::evaluation_node(LINE, $1); }
     | args tWRITE                                          { $$ = new mml::print_node(LINE, $1, false); }
     | args tWRITELN                                        { $$ = new mml::print_node(LINE, $1, true); }
     | tWHILE '('expr')' inst                               { $$ = new mml::while_node(LINE, $3, $5); }
     | tIF '('expr')' inst                                  { $$ = new mml::if_node(LINE, $3, $5); }
     | block                                                { $$ = $1; }
     | stop ';'                                             { $$ = $1; }
     | next ';'                                             { $$ = $1; }
     | return ';'                                           { $$ = $1; }
     ;
              
vardec : tPUBLIC data_type tIDENTIFIER opt_initializer      { $$ = new mml::variable_declaration_node(LINE, tPUBLIC,  $2, *$3, $4); delete $3; }
       | tFOREIGN func_type tIDENTIFIER                     { $$ = new mml::variable_declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); delete $3; }
       | tFORWARD data_type tIDENTIFIER                     { $$ = new mml::variable_declaration_node(LINE, tFORWARD, $2, *$3, nullptr); delete $3; }
       | data_type tIDENTIFIER opt_initializer              { $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, $3); delete $2; }
       | tPUBLIC tIDENTIFIER '=' expr                       { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, *$2, $4); delete $2; }
       | tPUBLIC tTYPE_AUTO tIDENTIFIER '=' expr            { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, *$3, $5); delete $3; }
       | tTYPE_AUTO tIDENTIFIER '=' expr                    { $$ = new mml::variable_declaration_node(LINE, tPRIVATE, *$2, $4); delete $2; }
       ;

data_type : tTYPE_STRING                                    { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);  }
          | tTYPE_INT                                       { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);     }
          | tTYPE_REAL                                      { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);  }
          | tTYPE_VOID                                      { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
          | '[' data_type ']'                               { $$ = cdk::reference_type::create(4, $2); }
          | func_type                                       { $$ = $1; }
          ;

opt_type_seq :                                              { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>{};  }
             | type_seq                                     { $$ = $1; }

type_seq : data_type                                        { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>{}; $$->push_back($1); }
         | type_seq ',' data_type                           { $$ = $1; $$->push_back($3); }
         ;

func_type : data_type '<' opt_type_seq '>'                  { $$ = cdk::functional_type::create(*$3, $1); }
          ;

fdef: '('argdecs')' tARROW data_type block                  { $$ = new mml::function_definition_node(LINE, $2, $5, $6); };

fcall: lval '(' args ')'                                    { $$ = new mml::function_call_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     | '(' expr ')' '(' args ')'                            { $$ = new mml::function_call_node(LINE, $2, $5); }
     | tRECURSION '(' args ')'                              { $$ = new mml::function_call_node(LINE, $3); }
     ;

opt_initializer :                                           { $$ = nullptr; }
                | '=' expr                                  { $$ = $2; }
                ;

block : '{' '}'                                             { $$ = new mml::block_node(LINE, nullptr, nullptr); }
      | '{' vardecs '}'                                     { $$ = new mml::block_node(LINE, $2, nullptr); }
      | '{' instructions '}'                                { $$ = new mml::block_node(LINE, nullptr, $2); }
      | '{' vardecs instructions '}'                        { $$ = new mml::block_node(LINE, $2, $3); }
      ;

stop : tSTOP                                                { $$ = new mml::stop_node(LINE); }
     | tSTOP tINTEGER                                       { $$ = new mml::stop_node(LINE, $2); }

next : tNEXT                                                { $$ = new mml::next_node(LINE); }
     | tNEXT tINTEGER                                       { $$ = new mml::next_node(LINE, $2); }

return : tRETURN                                            { $$ = new mml::return_node(LINE, nullptr); }
       | tRETURN expr                                       { $$ = new mml::return_node(LINE, $2); }
       ;

argdecs :                                                   { $$ = nullptr;  }
        | argdec                                            { $$ = new cdk::sequence_node(LINE, $1);     }
        | argdecs ',' argdec                                { $$ = new cdk::sequence_node(LINE, $3, $1); }
        ;

argdec : data_type tIDENTIFIER                              { $$ = new mml::variable_declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); delete $2; }
       ;

args :                                                      { $$ = new cdk::sequence_node(LINE); }
      | expr                                                { $$ = new cdk::sequence_node(LINE, $1);     }
      | args ',' expr                                       { $$ = new cdk::sequence_node(LINE, $3, $1); }
      ;

lval : tIDENTIFIER                                          { $$ = new cdk::variable_node(LINE, $1); }
     | lval '[' expr ']'                                    { $$ = new mml::index_pointer_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     | '(' expr ')' '[' expr ']'                            { $$ = new mml::index_pointer_node(LINE, $2, $5); }
     ;


expr : integer                                              { $$ = $1; }
     | real                                                 { $$ = $1; }
     | string                                               { $$ = new cdk::string_node(LINE, $1); }
     | tNULLPTR                                             { $$ = new mml::null_node(LINE); }
     /* LEFT VALUES */
     | lval                                                 { $$ = new cdk::rvalue_node(LINE, $1); }
     /* ASSIGNMENTS */
     | lval '=' expr                                        { $$ = new cdk::assignment_node(LINE, $1, $3); }
      /* ARITHMETIC EXPRESSIONS */
     | expr '+' expr                                        { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr                                        { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr                                        { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr                                        { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr                                        { $$ = new cdk::mod_node(LINE, $1, $3); }
     /* LOGICAL EXPRESSIONS */
     | expr '<' expr                                        { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr                                        { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr                                        { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr                                        { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr                                        { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr                                        { $$ = new cdk::eq_node(LINE, $1, $3); }
     /* LOGICAL EXPRESSIONS */
     | expr tAND expr                                       { $$ = new cdk::and_node(LINE, $1, $3); }
     | expr tOR expr                                        { $$ = new cdk::or_node (LINE, $1, $3); }
     /* UNARY OPS */
     | '-' expr %prec tUNARY                                { $$ = new cdk::neg_node(LINE, $2); } 
     | '+' expr %prec tUNARY                                { $$ = $2; } 
     | lval tADDRESSOF %prec tUNARY                         { $$ = new mml::addressof_node(LINE, $1); }
     | '~' expr %prec tUNARY                                { $$ = new cdk::not_node(LINE, $2); }
     /* USER INPUT */
     | tREAD                                                { $$ = new mml::read_node(LINE); }
     /* FUNCTION DEFINITION */
     | fdef                                                 { $$ = $1; }
     /* FUNCTION CALLS */
     | fcall                                                { $$ = $1; }
     /* OTHER */
     | tSIZEOF '(' expr ')'                                 { $$ = new mml::sizeof_node(LINE, $3); }
     | '(' expr ')'                                         { $$ = $2; }
     | '[' expr ']'                                         { $$ = new mml::stack_alloc_node(LINE, $2); }
     ;   

integer    : tINTEGER                                       { $$ = new cdk::integer_node(LINE, $1); };
real       : tREAL                                          { $$ = new cdk::double_node(LINE, $1); };
string     : tSTRING                                        { $$ = $1; }
           | string tSTRING                                 { $$ = $1; $$->append(*$2); delete $2; }
           ;

%%
