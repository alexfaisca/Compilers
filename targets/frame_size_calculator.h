#ifndef __MML_TARGETS_FRAME_SIZE_CALCULATOR_H__
#define __MML_TARGETS_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"
#include <stack>
#include "targets/symbol.h"
#include <cdk/symbol_table.h>

namespace mml {

  class frame_size_calculator: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;
    size_t _size;

  public:
    frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<mml::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab), _size(0) {}

  public:
    size_t size() const {
      return _size;
    }
    ~frame_size_calculator();

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };


}

#endif
