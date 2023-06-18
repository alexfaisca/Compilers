#ifndef __MML_AST_FUNCTION_CALL_NODE_H__
#define __MML_AST_FUNCTION_CALL_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>

namespace mml {

  class function_call_node: public cdk::expression_node {
    cdk::expression_node *_expression;
    cdk::sequence_node *_arguments;

  public:
    function_call_node(int lineno, cdk::sequence_node *arguments) :
        cdk::expression_node(lineno), _expression(nullptr), _arguments(arguments) {
    }

    function_call_node(int lineno, cdk::expression_node *expression, cdk::sequence_node *arguments) :
        cdk::expression_node(lineno), _expression(expression), _arguments(arguments) {
    }

  public:
    cdk::expression_node* expression() {
      return _expression;
    }
    void initialize_call(cdk::expression_node* function) {
        _expression = function;
    }
    cdk::sequence_node* arguments() {
      return _arguments;
    }
    cdk::typed_node *argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_call_node(this, level);
    }

  };

} // mml

#endif
