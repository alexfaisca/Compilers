#ifndef __MML_TARGETS_TYPE_CHECKER_H__
#define __MML_TARGETS_TYPE_CHECKER_H__

#include "targets/basic_ast_visitor.h"
#include <stack>
#include <cdk/types/reference_type.h>
#include <cdk/types/functional_type.h>

namespace mml {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class type_checker: public basic_ast_visitor {
    const std::string _mainLabel;
    std::string *_currentFunctionLabel;
    int _currentLoopDepth;
    cdk::symbol_table<mml::symbol> &_symtab;
    std::stack<function_definition_node*> _functionStack;
    std::stack<std::shared_ptr<cdk::basic_type>> _functionType;
    basic_ast_visitor *_parent;

  public:
    type_checker(std::shared_ptr<cdk::compiler> compiler, const std::string &mainLabel, std::string *currentFunctionLabel, int currentDepth, std::stack<std::shared_ptr<cdk::basic_type>> &functionType, cdk::symbol_table<mml::symbol> &symtab, basic_ast_visitor *parent) :
            basic_ast_visitor(compiler), _mainLabel(mainLabel) , _currentFunctionLabel(currentFunctionLabel), _currentLoopDepth(currentDepth), _symtab(symtab), _functionType(functionType), _parent(parent) {
    }

  public:
    ~type_checker() {
      os().flush();
    }

  protected:
    void processUnaryExpression(cdk::unary_operation_node *const node, int lvl);
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
    }
    void processBinaryExpression(cdk::binary_operation_node *const node, int lvl);
    void processIntegerOnlyExpression(cdk::binary_operation_node *const node, int lvl);
    void processCompareExpression(cdk::binary_operation_node *const node, int lvl);
    void processEqualityExpression(cdk::binary_operation_node *const node, int lvl);
    static void processPointerType(const std::shared_ptr<cdk::reference_type>& left, const std::shared_ptr<cdk::reference_type>& right);
    static void processFunctionalType(const std::shared_ptr<cdk::functional_type>& left, const std::shared_ptr<cdk::functional_type>& right);

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

} // mml

//---------------------------------------------------------------------------
//     HELPER MACRO FOR TYPE CHECKING
//---------------------------------------------------------------------------

#define CHECK_TYPES(compiler, mainLabel, currentFunctionLabel, currentDepth, functionType, symtab, node) { \
  try { \
    mml::type_checker checker(compiler, mainLabel, currentFunctionLabel, currentDepth, functionType, symtab, this); \
    (node)->accept(&checker, 0); \
  } \
  catch (const std::string &problem) { \
    std::cerr << (node)->lineno() << ": " << problem << std::endl; \
    return; \
  } \
}

#define ASSERT_SAFE_EXPRESSIONS CHECK_TYPES(_compiler, _mainLabel, _functionLabels.empty() ? nullptr : &_functionLabels.top(), get_current_loop_depth(), _functionType, _symtab, node)

#endif
