#ifndef __MML_AST_ADRESSOF_NODE_H
#define __MML_AST_ADRESSOF_NODE_H

#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/expression_node.h>

namespace mml {

  /**
   * Class for describing addressof nodes.
   */
  class addressof_node : public cdk::expression_node {
    cdk::lvalue_node *_left_value;

  public:
    inline addressof_node(int lineno, cdk::lvalue_node *left_value) :
    expression_node(lineno), _left_value(left_value) {
    }

  public:
    inline cdk::lvalue_node *left_value() {
      return _left_value;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_addressof_node(this, level);
    }

  };

} // mml

#endif
