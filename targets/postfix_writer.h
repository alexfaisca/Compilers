#ifndef __MML_TARGETS_POSTFIX_WRITER_H__
#define __MML_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>
#include <set>
#include <list>
#include <stack>

namespace mml {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    std::stack<std::shared_ptr<cdk::basic_type>> _functionType;
    std::stack<std::string> _functionLabels;
    std::shared_ptr<cdk::basic_type> _expectedType;
    std::list<int> _loopBeginLbl, _loopEndLbl;
    std::set<std::string> _libraryImportFunctions; // set holding external functions' names necessary for program operation
    const std::string _mainLabel = "_main";
    std::string *lbl = nullptr;
    int _lbl;
    size_t _currentFrameSize;
    size_t _offset;
    bool _mainReturn;
    bool _functionArgument;
    bool _forward;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<mml::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
      basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0), _currentFrameSize(0), _offset(0),
      _mainReturn(false), _functionArgument(false), _forward(false) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }
    void enter_function_args() {
      _functionArgument = true;
    }
    void leave_function_args() {
      _functionArgument = false;
    }
    bool in_function_args() const {
      return _functionArgument;
    }
    bool in_function_body() const {
      return !_functionLabels.empty();
    }
    void push_label(std::string &lbl) {
      _functionLabels.push(lbl);
    }
    void pop_label() {
      _functionLabels.pop();
    }
    std::string &get_current_label() {
      return _functionLabels.top();
    }
    void insert_library_import(std::string s) {
      _libraryImportFunctions.insert(s);
    }
    std::set<std::string> get_library_imports() {
      return _libraryImportFunctions;
    }
    int get_current_loop_depth() {
      return (int)_loopBeginLbl.size();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // mml

#endif
