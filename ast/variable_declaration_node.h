#ifndef __MML_AST_VARIABLE_DECLARATION_NODE_H__
#define __MML_AST_VARIABLE_DECLARATION_NODE_H__

#include <cdk/ast/typed_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/types/basic_type.h>
#include "mml_parser.tab.h"

namespace mml {

  class variable_declaration_node: public cdk::typed_node {
    int _qualifier;
    std::string _identifier;
    cdk::expression_node *_initializer;

  public:
    variable_declaration_node(int lineno, int qualifier, std::shared_ptr<cdk::basic_type> varType, const std::string &identifier,
                              cdk::expression_node *initializer) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _initializer(initializer) {

        type(varType);
    }

    variable_declaration_node(int lineno, int qualifier, const std::string &identifier, cdk::expression_node *initializer) :
        cdk::typed_node(lineno), _qualifier(qualifier), _identifier(identifier), _initializer(initializer) {
        
      type(cdk::primitive_type::create(4, cdk::TYPE_UNSPEC));
    }

  public:
    int qualifier() {
      return _qualifier;
    }
    const std::string& identifier() const {
      return _identifier;
    }
    cdk::expression_node* initializer() {
      return _initializer;
    }
    inline bool is_public(){
      return _qualifier == tPUBLIC;
    }
    inline bool is_forward(){
      return _qualifier == tFORWARD;
    }
    inline bool is_foreign(){
      return _qualifier == tFOREIGN;
    }
    

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_variable_declaration_node(this, level);
    }

  };

} // mml

#endif