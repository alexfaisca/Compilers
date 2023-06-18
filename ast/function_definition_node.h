#ifndef __MML_AST_FUNCTION_DEFINITION_NODE_H__
#define __MML_AST_FUNCTION_DEFINITION_NODE_H__

#include <string>
#include <utility>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <vector>
#include <numeric>
#include <cdk/types/basic_type.h>
#include <cdk/types/structured_type.h>

#include "ast/block_node.h"

namespace mml {

  /**
   * Class for describing function_definition nodes.
   */
  class function_definition_node: public cdk::expression_node {
    cdk::sequence_node *_arguments;
    std::shared_ptr<cdk::basic_type> _output;
    block_node *_block;

  public:
    function_definition_node(int lineno, cdk::sequence_node *arguments, std::shared_ptr<cdk::basic_type> output, block_node *block)
      :cdk::expression_node(lineno), _arguments(arguments), _output(output), _block(block) {
      type(output); // Used in frame size calculation
    }

  public:
    inline cdk::sequence_node *arguments() {
      return _arguments;
    }
    cdk::typed_node *argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
    std::shared_ptr<cdk::basic_type> output() {
      return _output;
    }
    inline block_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // mml

#endif