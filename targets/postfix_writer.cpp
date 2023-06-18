#include <string>
#include <cmath>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void mml::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void mml::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (in_function_body()) {
    std::string label = mklbl(++_lbl);
    _pf.RODATA();
    _pf.ALIGN();
    _pf.LABEL(label);
    _pf.SDOUBLE(node->value());
    _pf.TEXT(get_current_label());
    _pf.ADDR(label);
    _pf.LDDOUBLE();
  } else _pf.SDOUBLE(node->value());
}
void mml::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl + 2);
  _pf.INT(0);
  _pf.EQ();
}
void mml::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  std::string label = mklbl(++_lbl);

  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JZ(label);

  node->right()->accept(this, lvl + 2);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(label);
}
void mml::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  std::string label = mklbl(++_lbl);

  node->left()->accept(this, lvl + 2);
  _pf.DUP32();
  _pf.JNZ(label);

  node->right()->accept(this, lvl + 2);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(label);
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (in_function_body()) _pf.INT(node->value());
  else _pf.SINT(node->value());
}

void mml::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  _pf.RODATA(); 
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  _pf.SSTRING(node->value());

  _pf.ALIGN();

  if (in_function_body()) {
    _pf.TEXT(get_current_label());
    _pf.ADDR(mklbl(lbl1));
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl); 
  _pf.NEG();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  node->left()->accept(this, lvl);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(static_cast<int>(log2(static_cast<int>(cdk::reference_type::cast(node->type())->referenced()->size())))); // prepare shift by log2 type size
    _pf.SHTL(); // shift left
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(static_cast<int>(log2(static_cast<int>(cdk::reference_type::cast(node->type())->referenced()->size())))); // prepare shift by log2 type size
    _pf.SHTL(); // shift left
  }

  if (node->type()->name() == cdk::TYPE_DOUBLE) _pf.DADD();
  else _pf.ADD();
}

void mml::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(static_cast<int>(log2(static_cast<int>(cdk::reference_type::cast(node->type())->referenced()->size())))); // prepare shift by log2 type size
    _pf.SHTL(); // shift left
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.INT(static_cast<int>(log2(static_cast<int>(cdk::reference_type::cast(node->type())->referenced()->size())))); // prepare shift by log2 type size
    _pf.SHTL(); // shift left
  }

  // pointer subtraction yields distance between two addresses
  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    int lbl1 = ++_lbl;
    // subtract addresses
    _pf.SUB();
    // get pointer relative offset
    _pf.INT(static_cast<int>(cdk::reference_type::cast(node->left()->type())->referenced()->size()));
    _pf.DIV();
    // get module
    _pf.DUP32();
    _pf.INT(0);
    _pf.LT();
    _pf.JZ(mklbl(lbl1));
    _pf.NEG();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl1));
  } else {
    if (node->is_typed(cdk::TYPE_DOUBLE)) _pf.DSUB();
    else _pf.SUB();
  }
}

void mml::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->type()->name() == cdk::TYPE_INT) _pf.MUL();
  else _pf.DMUL();
}

void mml::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->type()->name() == cdk::TYPE_INT) _pf.DIV();
  else _pf.DDIV();
}

void mml::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void mml::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.LT();
}

void mml::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.LE();
}

void mml::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.GE();
}

void mml::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.GT();
}

void mml::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.NE();
}

void mml::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->left()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  node->right()->accept(this, lvl + 2);
  if (node->type()->name() == cdk::TYPE_DOUBLE && node->right()->type()->name() == cdk::TYPE_INT) 
    _pf.I2D();

  if (node->left()->type()->name() == cdk::TYPE_DOUBLE || node->right()->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.DCMP();
    _pf.INT(0);
  }

  _pf.EQ();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  auto symbol = _symtab.find(node->name());
  if (symbol->global()) _pf.ADDR(symbol->get_label());
  else _pf.LOCAL(static_cast<int>(symbol->offset()));
}

void mml::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) 
    _pf.LDDOUBLE(); // load 8 bytes
  else if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING) ||
          node->is_typed(cdk::TYPE_POINTER) || node->is_typed(cdk::TYPE_FUNCTIONAL))
    _pf.LDINT(); // load 4 bytes
}

void mml::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->rvalue()->accept(this, lvl); // determine the new value
  if (node->is_typed(cdk::TYPE_DOUBLE)) {

    if (node->rvalue()->is_typed(cdk::TYPE_INT)) _pf.I2D(); // convert integer to double
    _pf.DUP64(); // duplicate double value

  } else _pf.DUP32(); // duplicate value

  node->lvalue()->accept(this, lvl); // where to store the value

  if (node->is_typed(cdk::TYPE_DOUBLE)) _pf.STDOUBLE();
  else _pf.STINT();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_program_node(mml::program_node * const node, int lvl) {
  // Note that MML doesn't have functions. Thus, it doesn't need
  // a function node. However, it must start in the main function.
  // The ProgramNode (representing the whole program) doubles as a
  // main function node.

  frame_size_calculator main_calculator(_compiler, _symtab);
  node->accept(&main_calculator, lvl);
  std::string segment = _mainLabel;

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT(segment);
  push_label(segment);
  _pf.ALIGN();
  _pf.GLOBAL(segment, _pf.FUNC());
  _pf.LABEL(segment);
  _pf.ENTER(main_calculator.size());

  node->block()->accept(this, lvl);

  // end the main function
  if (!_mainReturn) {
    _pf.INT(0);
    _pf.STFVAL32();
    _pf.LEAVE();
    _pf.RET();
  }

  pop_label();

  // library function imports
  for (const std::string &s : get_library_imports()) _pf.EXTERN(s);
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_evaluation_node(mml::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  node->argument()->accept(this, lvl);

  if (node->argument()->is_typed(cdk::TYPE_INT) || node->argument()->is_typed(cdk::TYPE_STRING) ||
      node->argument()->is_typed(cdk::TYPE_POINTER) || node->argument()->is_typed(cdk::TYPE_FUNCTIONAL)) {
    _pf.TRASH(4);
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8);
  } else if (!node->argument()->is_typed(cdk::TYPE_VOID) && // Arguments already trashed in do_function_call
             !node->argument()->is_typed(cdk::TYPE_FUNCTIONAL)) {
    std::cerr << "invalid type in expression" << std::endl;
    exit(1);
  }
}

void mml::postfix_writer::do_print_node(mml::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  if (node->arguments()) for (size_t i = 0; i < node->arguments()->size(); i++) {
    node->argument(i)->accept(this, lvl + 2);
    std::shared_ptr<cdk::basic_type> arg_type = node->argument(i)->type();

    if (arg_type == nullptr) {
      std::cerr << "typeless expression unprintable" << std::endl;
      exit(1);
    } else if (arg_type->name() == cdk::TYPE_INT) {
      insert_library_import("printi");
      _pf.CALL("printi");
      _pf.TRASH(4);
    } else if (arg_type->name() == cdk::TYPE_DOUBLE) {
      insert_library_import("printd");
      _pf.CALL("printd");
      _pf.TRASH(8);
    } else if (arg_type->name() == cdk::TYPE_STRING) {
      insert_library_import("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    } else if (arg_type->name() == cdk::TYPE_VOID) {
      std::cerr << "void type expression unprintable" << std::endl; // FIXME: print as char?
      exit(1);
    } else if (arg_type->name() == cdk::TYPE_UNSPEC) {
      std::cerr << "unspecified type expression unprintable" << std::endl;
      exit(1);
    } else {
      std::cerr << "unknown type expression unprintable" << std::endl;
      exit(1);
    }
    }

  if (node->newline()) {
    insert_library_import("println");
    _pf.CALL("println");
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_read_node(mml::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  if (node->is_typed(cdk::TYPE_INT)) {
    insert_library_import("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    insert_library_import("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_while_node(mml::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  int loopBeginLbl = ++_lbl, loopEndLbl = ++_lbl;
  _loopBeginLbl.push_back(loopBeginLbl);
  _loopEndLbl.push_back(loopEndLbl);

  _pf.LABEL(mklbl(loopBeginLbl));

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(loopEndLbl));

  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(loopBeginLbl));
  _pf.LABEL(mklbl(loopEndLbl));

  _loopBeginLbl.erase(std::next(_loopBeginLbl.end(), -1));
  _loopEndLbl.erase(std::next(_loopEndLbl.end(), -1));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_node(mml::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl1;

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));

  node->block()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_else_node(mml::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  int lbl1, lbl2;

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));

  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));

  node->elseblock()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl2));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_sizeof_node(mml::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  _pf.INT(static_cast<int>(node->expression()->type()->size()));
}

void mml::postfix_writer::do_return_node(mml::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  std::shared_ptr<cdk::basic_type> return_type;
  
  if (in_function_body()) {

    if (get_current_label() == _mainLabel) {
      return_type = cdk::primitive_type::create(4, cdk::TYPE_INT);
      _mainReturn = true;
    } else return_type = cdk::functional_type::cast(_functionType.top())->output(0);

  } else {
    std::cerr << "illegal return: nothing to return from" << std::endl;
    exit(1);
  }

  if (return_type->name() != cdk::TYPE_VOID) {
    node->value()->accept(this, lvl + 2);
    if (return_type->name() == cdk::TYPE_INT || return_type->name() == cdk::TYPE_STRING ||
        return_type->name() == cdk::TYPE_POINTER || return_type->name() == cdk::TYPE_FUNCTIONAL) {
      _pf.STFVAL32();
    } else if (return_type->name() == cdk::TYPE_DOUBLE) {
      if (node->value()->type()->name() == cdk::TYPE_INT) _pf.I2D();
      _pf.STFVAL64();
    } else {
      std::cerr << "illegal return: unknown return type" << std::endl;
      exit(1);
    }
  }
  _pf.LEAVE();
  _pf.RET();
}

void mml::postfix_writer::do_stop_node(mml::stop_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  _pf.JMP(mklbl(*std::next(_loopEndLbl.end(), -node->level())));
}

void mml::postfix_writer::do_next_node(mml::next_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  _pf.JMP(mklbl(*std::next(_loopBeginLbl.end(), -node->level())));
}

void mml::postfix_writer::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  
  size_t arguments_size = 0;
  int offset = 8;
  bool recursive = node->expression() == nullptr;
  std::shared_ptr<cdk::functional_type> function_type, expression_type;
  std::shared_ptr<cdk::basic_type> expected_return_type, return_type;

  if (recursive) {
    function_type = cdk::functional_type::cast(_functionType.top());
    expression_type = function_type;
  } else { // Test for covariant types
    function_type = cdk::functional_type::cast(node->expression()->type());
    auto rvalue = dynamic_cast<cdk::rvalue_node*>(node->expression());

    if (rvalue) {
      auto variable = dynamic_cast<cdk::variable_node *>(rvalue->lvalue());

      if (variable) {
        auto symbol = _symtab.find(variable->name());
        expression_type = cdk::functional_type::cast(symbol->get_expression_type());
      } else expression_type = function_type;

    } else expression_type = function_type;

  }

  return_type = expression_type->output(0);
  expected_return_type = function_type->output(0);

  // Save function context
  if (recursive) {
    for (int i = -4; i >= -static_cast<int>(_currentFrameSize) + static_cast<int>(return_type->size()); i -= 4) _pf.LOCV(i);
    if (node->arguments()->size() > 0) for (size_t i = 0, size = 0; i < node->arguments()->size(); i++, offset += static_cast<int>(size)) {

        _pf.LOCAL(offset);
        if ((size = function_type->input(i)->size()) == 4) _pf.LDINT();
        else _pf.LDDOUBLE();

      }
  }

  if (node->arguments()->size() > 0) {

    auto checker = new mml::type_checker(_compiler, _mainLabel, _functionLabels.empty() ? nullptr : &_functionLabels.top(), get_current_loop_depth(), _functionType, _symtab, this);
    enter_function_args();

    for (size_t i = node->arguments()->size(); i-- != 0;) {
      auto *argument = dynamic_cast<cdk::expression_node *>(node->argument(i));
      auto *arg = new variable_declaration_node(node->lineno(), tPRIVATE, function_type->input(i), mklbl(++_lbl), argument);

      try {
        arg->accept(checker, 4);
      } catch (const std::string &problem) {
        std::cerr << (node)->lineno() << ": " << problem << std::endl;
        return;
      }

      _expectedType = argument->type();
      arg->accept(this, lvl + 2);
      arguments_size += expression_type->input(i)->size();
    }

    leave_function_args();
  }

  if (node->expression()) node->expression()->accept(this, lvl + 2);
  else _pf.ADDR(get_current_label());
  _pf.BRANCH();

  if (arguments_size > 0) _pf.TRASH(static_cast<int>(arguments_size));

  // Load function context
  if (recursive) { // TODO: Ideally allocate space somewhere to store function call returns and get rid of bool recursive checking
    if (return_type->name() == cdk::TYPE_INT || return_type->name() == cdk::TYPE_STRING ||
        return_type->name() == cdk::TYPE_FUNCTIONAL || return_type->name() == cdk::TYPE_POINTER ||
        return_type->name() == cdk::TYPE_DOUBLE) {

      if (return_type->size() == 8) {

        _pf.LDFVAL64();
        _pf.LOCAL(-static_cast<int>(_currentFrameSize));
        _pf.STDOUBLE();

      } else {

        _pf.LDFVAL32();
        _pf.LOCA(-static_cast<int>(_currentFrameSize));

      }
    }
    if (arguments_size > 0) for (size_t i = node->arguments()->size(), size = 0; i-- != 0; ) {

        offset -= static_cast<int>(size = dynamic_cast<cdk::expression_node *>(node->argument(i))->type()->size());
        _pf.LOCAL(offset);
        if (size == 4) _pf.STINT();
        else _pf.STDOUBLE();

    }
    for (int i = -static_cast<int>(_currentFrameSize) + static_cast<int>(return_type->size()); i < 0; i += 4) _pf.LOCA(i);
    // Load previously stored return value
    if (return_type->name() == cdk::TYPE_INT || return_type->name() == cdk::TYPE_STRING ||
        return_type->name() == cdk::TYPE_FUNCTIONAL || return_type->name() == cdk::TYPE_POINTER || return_type->name() == cdk::TYPE_DOUBLE) {

      if (return_type->size() == 8) {

        _pf.LOCAL(-static_cast<int>(_currentFrameSize));
        _pf.LDDOUBLE();

      } else _pf.LOCV(-static_cast<int>(_currentFrameSize));

    }
  } else {
    if (return_type->name() == cdk::TYPE_INT || return_type->name() == cdk::TYPE_STRING ||
        return_type->name() == cdk::TYPE_FUNCTIONAL || return_type->name() == cdk::TYPE_POINTER) {

      _pf.LDFVAL32();
      if (return_type->name() == cdk::TYPE_INT && expected_return_type->name() == cdk::TYPE_DOUBLE) _pf.I2D();

    } else if (return_type->name() == cdk::TYPE_DOUBLE) {

      _pf.LDFVAL64();
      if (expected_return_type->name() == cdk::TYPE_INT) _pf.D2I();

    } else if (return_type->name() != cdk::TYPE_VOID) {
      std::cerr << "unknown return type cannot call function" << std::endl;
      exit(1);
    }
  }
}

void mml::postfix_writer::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS

  std::string function = _forward ? *lbl : mklbl(++_lbl);
  size_t offset = _offset, frame_size = _currentFrameSize;

  _functionType.push(node->type());

  _offset = 8;
  _symtab.push();

  if (node->arguments()) {
    auto checker = new mml::type_checker(_compiler, _mainLabel, _functionLabels.empty() ? nullptr : &_functionLabels.top(), get_current_loop_depth(), _functionType, _symtab, this);

    for (size_t i = 0; i < node->arguments()->size(); i++) {

      try {
        node->argument(i)->accept(checker, 0);
      } catch (const std::string &problem) {
        std::cerr << (node)->lineno() << ": " << problem << std::endl;
        return;
      }

      std::shared_ptr<symbol> symbol = _symtab.find(dynamic_cast<variable_declaration_node *>(node->argument(i))->identifier());
      symbol->set_offset(_offset);
      _offset += node->type()->size();
    }
  }

  _pf.TEXT(function);
  _pf.ALIGN();
  push_label(function);
  if (!_forward) _pf.LABEL(function);

  frame_size_calculator function_frame_size_calculator(_compiler, _symtab);
  node->accept(&function_frame_size_calculator, lvl);
  _pf.ENTER((_currentFrameSize = function_frame_size_calculator.size()));
  _offset = 0; // variable declaration inside function

  node->block()->accept(this, lvl + 4);

  _offset = offset;
  _currentFrameSize = frame_size;

  if (node->output() && node->output()->name() == cdk::TYPE_VOID) {
    _pf.LEAVE();
    _pf.RET();
  } 

  pop_label();

  if (in_function_body()) {
    _pf.TEXT(get_current_label());
    _pf.ADDR(function);
  } else if (!_forward) {
    _pf.DATA();
    _pf.SADDR(function);
  }

  _symtab.pop(); 
  _functionType.pop();
}

void mml::postfix_writer::do_block_node(mml::block_node * const node, int lvl) {
  if (node->declarations()) node->declarations()->accept(this,lvl);
  if (node->instructions()) node->instructions()->accept(this,lvl);
}

void mml::postfix_writer::do_addressof_node(mml::addressof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->left_value()->accept(this, lvl + 2);
}

void mml::postfix_writer::do_index_pointer_node(mml::index_pointer_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->base()->accept(this, lvl + 2); // get pointer base address
  node->index()->accept(this, lvl + 2); // get pointer element displacement
  _pf.INT(static_cast<int>(node->type()->size()));
  _pf.MUL();
  _pf.ADD(); // get element memory address
}

void mml::postfix_writer::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  if (in_function_body()) _pf.INT(0);
  else _pf.SINT(0);
}

void mml::postfix_writer::do_variable_declaration_node(mml::variable_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  std::shared_ptr<symbol> symbol = _symtab.find(node->identifier());
  std::string label;
  bool forward = _forward;
  int offset = 8;
  _forward = false;

  if (node->is_foreign()) {

    label = mklbl(++_lbl);
    symbol->set_global();
    insert_library_import(node->identifier());
    _pf.DATA();
    _pf.ALIGN();
    _pf.LABEL(label);
    symbol->set_label(label);
    _pf.SADDR(node->identifier());

  } else if (node->is_forward()) {

    std::string holder = mklbl(++_lbl);
    label = mklbl(++_lbl);
    symbol->set_global();
    symbol->set_label(holder);
    symbol->set_segment(label);
    symbol->set_forward();
    _pf.TEXT(label);
    _pf.LABEL(label);
    _pf.DATA();
    _pf.ALIGN();
    _pf.LABEL(holder);
    _pf.SADDR(label);

  } else if (in_function_args()) {

    if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {  // Create a 0 size framed wrapper code section to cast args and return type

      auto type = cdk::functional_type::cast(node->type()), expected_type = cdk::functional_type::cast(_expectedType);
      auto return_type = type->output(0), expected_return = expected_type->output(0);

      _pf.TEXT(node->identifier());
      push_label(const_cast<std::string &>(node->identifier()));
      _pf.LABEL(node->identifier());
      _pf.ENTER(expected_return->size());

      // Arguments cast
      offset += static_cast<int>(expected_type->input()->size());

      for (size_t i = type->input_length(); i-- != 0;) {
        auto expected = expected_type->input(i), real = type->input(i);
        offset -= static_cast<int>(expected->size());
        _pf.LOCAL(offset);
        if (expected->size() == 4) _pf.LDINT();
        else _pf.LDDOUBLE();
        if (expected->name() == cdk::TYPE_INT && real->name() == cdk::TYPE_DOUBLE) _pf.D2I();
        else if (expected->name() == cdk::TYPE_DOUBLE && real->name() == cdk::TYPE_INT) _pf.I2D();
      }

      // Function call
      node->initializer()->accept(this, lvl + 2);
      _pf.BRANCH();

      // Return type cast
      if (expected_return->size() == 4 || expected_return->name() == cdk::TYPE_FUNCTIONAL) _pf.LDFVAL32();
      else _pf.LDFVAL64();

      if (return_type->name() == cdk::TYPE_INT && expected_return->name() == cdk::TYPE_DOUBLE) _pf.D2I();
      else if (return_type->name() == cdk::TYPE_DOUBLE && expected_return->name() == cdk::TYPE_INT) _pf.I2D();

      if (return_type->size() == 4 || return_type->name() == cdk::TYPE_FUNCTIONAL) _pf.STFVAL32();
      else _pf.STFVAL64();

      _pf.LEAVE();
      _pf.RET();
      pop_label();
      _pf.TEXT(get_current_label());
      _pf.ADDR(node->identifier());

    } else { // Cast args

      node->initializer()->accept(this, lvl + 2);
      
      if (node->initializer()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_DOUBLE)) _pf.I2D();
      else if (node->initializer()->is_typed(cdk::TYPE_DOUBLE) && node->is_typed(cdk::TYPE_INT)) _pf.D2I();
    }
  } else if (in_function_body()) {

    _offset -= node->type()->size();
    symbol->set_offset(_offset);

    if (node->initializer()) {
      node->initializer()->accept(this, lvl + 2);
      if (node->is_typed(cdk::TYPE_DOUBLE) && node->initializer()->is_typed(cdk::TYPE_INT)) _pf.I2D();

      _pf.LOCAL(static_cast<int>(_offset));
      if (node->is_typed(cdk::TYPE_DOUBLE)) _pf.STDOUBLE();
      else _pf.STINT();
    }

  } else {

    symbol->set_global();
    if (node->is_public()) _pf.GLOBAL(node->identifier(), _pf.OBJ());
    if (!node->initializer()) {

      label = mklbl(++_lbl);
      symbol->set_label(label);
      _pf.BSS();
      _pf.ALIGN();
      _pf.LABEL(label);
      _pf.SALLOC(static_cast<int>(node->type()->size()));

    } else {

      if ((_forward = symbol->is_forward())) lbl = &symbol->get_segment();
      else {
        _pf.DATA();
        _pf.ALIGN();
        label = mklbl(++_lbl); // Function address holder
        _pf.LABEL(label);
        symbol->set_label(label);
        label = mklbl(++_lbl); // Function address
        symbol->set_segment(label);
        lbl = &label;
      }

      if (node->is_typed(cdk::TYPE_DOUBLE) && node->initializer()->is_typed(cdk::TYPE_INT))
        _pf.SDOUBLE(static_cast<double>(dynamic_cast<cdk::integer_node *>(node->initializer())->value()));
      else node->initializer()->accept(this, lvl + 2);

    }
  }
  _forward = forward;
}

void mml::postfix_writer::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl);
  _pf.INT(static_cast<int>(log2(static_cast<int>(cdk::reference_type::cast(node->type())->referenced()->size()))));
  _pf.SHTL();
  _pf.ALLOC();
  _pf.SP();
}

void mml::postfix_writer::do_identity_node(mml::identity_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->argument()->accept(this, lvl);
}