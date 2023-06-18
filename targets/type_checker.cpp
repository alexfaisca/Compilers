#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void mml::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) node->node(i)->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
    // EMPTY
}

void mml::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
    // EMPTY
}

void mml::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC

  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))

    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {

    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

  } else throw std::string("Wrong type in unary expression.");
}

void mml::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

void mml::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void mml::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void mml::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

void mml::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

void mml::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

void mml::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}

void mml::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processIntegerOnlyExpression(node, lvl);
}

void mml::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processCompareExpression(node, lvl);
}

void mml::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processCompareExpression(node, lvl);
}

void mml::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processCompareExpression(node, lvl);
}

void mml::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processCompareExpression(node, lvl);
}

void mml::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processEqualityExpression(node, lvl);
}

void mml::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processEqualityExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC
  const std::string &id = node->name();
  std::shared_ptr<mml::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) node->type(symbol->type());
  else throw std::string("undeclared variable " + id);
}

void mml::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw std::string("undeclared variable " + id);
  }
}

void mml::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);

  if (node->lvalue()->is_typed(cdk::TYPE_UNSPEC)) throw std::string("assignment to unspecified typed lval");

  if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) { // check if read_node or stack_alloc node were received as rvalue
    if (dynamic_cast<mml::read_node *>(node->rvalue()) != nullptr) {

      if (node->lvalue()->is_typed(cdk::TYPE_INT) || node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
        node->rvalue()->type(node->lvalue()->type());
      else throw std::string("unexpected integer / double in assignment");

    } else if (dynamic_cast<mml::stack_alloc_node *>(node->rvalue()) != nullptr) {

      if (node->lvalue()->is_typed(cdk::TYPE_POINTER))
        node->rvalue()->type(node->lvalue()->type());
      else throw std::string("unexpected pointer in assignment");

    } else throw std::string("unspecified type in assignment");
  }

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {
  
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else throw std::string("Incompatible assignment to integer.");
  
  } else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {

    if (node->rvalue()->is_typed(cdk::TYPE_INT) || node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else throw std::string("Incompatible assignment to real");

  } else if (node->lvalue()->is_typed(cdk::TYPE_STRING)) {

    if (node->rvalue()->is_typed(cdk::TYPE_STRING)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    } else throw std::string("Incompatible assignment to string");

  } else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {

    if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      if (cdk::reference_type::cast(node->rvalue()->type())->referenced()->name() != cdk::TYPE_UNSPEC)
        processPointerType(cdk::reference_type::cast(node->lvalue()->type()),
                           cdk::reference_type::cast(node->rvalue()->type()));
      node->type(node->lvalue()->type());
    } else throw std::string("Incompatible assignment to pointer.");

  } else if (node->lvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {

      if (node->rvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {
        processFunctionalType(cdk::functional_type::cast(node->lvalue()->type()),
                              cdk::functional_type::cast(node->rvalue()->type()));
        node->type(node->lvalue()->type());
        // Stores casting relevant information in the symtab
        _symtab.find(dynamic_cast<cdk::variable_node *>(node->lvalue())->name())->set_expression_type(node->rvalue()->type());
      }

    } else throw std::string("Wrong types in assignment.");
}

//---------------------------------------------------------------------------

void mml::type_checker::do_program_node(mml::program_node *const node, int lvl) {
  _functionType.push(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)));
  node->block()->accept(this, lvl+2);
  _functionType.pop();
}

void mml::type_checker::do_evaluation_node(mml::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void mml::type_checker::do_print_node(mml::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_read_node(mml::read_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_while_node(mml::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  _currentLoopDepth++;
  node->block()->accept(this, lvl + 2);
  _currentLoopDepth--;
}

//---------------------------------------------------------------------------

void mml::type_checker::do_if_node(mml::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {

    if (dynamic_cast<read_node *>(node->condition()) != nullptr)
      node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else throw std::string("Unknown node with unspecified type.");

  } else if (!node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("Expected integer condition.");
}

void mml::type_checker::do_if_else_node(mml::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC)) {

    if (dynamic_cast<read_node *>(node->condition()) != nullptr)
      node->condition()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else throw std::string("Unknown node with unspecified type.");

  } else if (!node->condition()->is_typed(cdk::TYPE_INT))
    throw std::string("Expected integer condition.");
}

//---------------------------------------------------------------------------

void mml::type_checker::do_sizeof_node(mml::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->expression()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_return_node(mml::return_node *const node, int lvl) {
  // Assert there is a block to return from
  if (node->value()) {
    node->value()->accept(this, lvl + 2);
    // Check for return type incongruency
    if (!_functionType.empty()) {
      std::shared_ptr<cdk::basic_type> return_type = cdk::functional_type::cast(_functionType.top())->output(0);

      if (return_type->name() == cdk::TYPE_VOID) {

        if (node->value() && !node->value()->is_typed(cdk::TYPE_VOID))
          throw std::string("non void return type in void function");

      } else if (return_type->name() == cdk::TYPE_DOUBLE) {

        if (!node->value()->is_typed(cdk::TYPE_INT) && !node->value()->is_typed(cdk::TYPE_DOUBLE))
          throw std::string("non numeric return type in double type function");

      } else if (return_type->name() == cdk::TYPE_INT) {

        if (!node->value()->is_typed(cdk::TYPE_INT))
          throw std::string("non integer return type in integer type function");

      } else if (return_type->name() == cdk::TYPE_STRING) {

        if (!node->value()->is_typed(cdk::TYPE_STRING))
          throw std::string("non string return type in string type function");

      } else if (return_type->name() == cdk::TYPE_POINTER) {

        if (node->value()->type()->name() == cdk::TYPE_POINTER) {
          if (cdk::reference_type::cast(node->value()->type())->referenced()->name() != cdk::TYPE_UNSPEC)
            processPointerType(cdk::reference_type::cast(return_type),
                               cdk::reference_type::cast(node->value()->type()));
        } else throw std::string("non pointer return type in pointer type function");

      } else if (return_type->name() == cdk::TYPE_FUNCTIONAL) {

        if (node->value()->type()->name() == cdk::TYPE_FUNCTIONAL) {
          processFunctionalType(cdk::functional_type::cast(return_type),
                                cdk::functional_type::cast(node->value()->type()));
          auto rvalue = dynamic_cast<cdk::rvalue_node*>(node->value());
          if (rvalue) {
            auto variable = dynamic_cast<cdk::variable_node *>(rvalue->lvalue());
            if (variable) {
              auto symbol = _symtab.find(variable->name());
              symbol->set_expression_type(node->value()->type());
            }
          }
        } else throw std::string("non function return type in function type function");

      } else throw std::string("unknown function return type");
    }
  }
}

void mml::type_checker::do_stop_node(mml::stop_node * const node, int lvl) {
  if (node->level() < 1) throw std::string("illegal depth stop (< 1)");
  else if (node->level() > _currentLoopDepth) throw std::string("illegal depth stop (too shallow)");
}

void mml::type_checker::do_next_node(mml::next_node * const node, int lvl) {
  if (node->level() < 1) throw std::string("illegal next stop (< 1)");
  else if (node->level() > _currentLoopDepth) throw std::string("illegal next stop (too shallow)");
}

void mml::type_checker::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC
  std::shared_ptr<cdk::functional_type> function_type;
  if (node->expression()) {
    node->expression()->accept(this, lvl + 2);
    if (node->expression()->is_typed(cdk::TYPE_FUNCTIONAL)) function_type = cdk::functional_type::cast(node->expression()->type());
    else throw std::string("Not a function");
  } else if (!_functionType.empty() && _currentFunctionLabel && *_currentFunctionLabel != _mainLabel) {
    function_type = cdk::functional_type::cast(_functionType.top());
  } else throw std::string("No function to call.");

  if (node->expression()) processFunctionalType(function_type, cdk::functional_type::cast(node->expression()->type()));
  node->type(function_type->output(0));
}

void mml::type_checker::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  std::shared_ptr<cdk::basic_type> output_type = node->output();
  std::vector<std::shared_ptr<cdk::basic_type>> input_type;

  _symtab.push();
  if (node->arguments()) {
    for (size_t i = 0; i < node->arguments()->size(); i++) {
      auto *argument = dynamic_cast<mml::variable_declaration_node*>(node->arguments()->node(i));
      if (!argument || !argument->type() || argument->is_typed(cdk::TYPE_UNSPEC)) throw std::string("invalid argument");
      else input_type.push_back(argument->type());
      const std::string &id = argument->identifier();
      auto symbol = mml::make_symbol(argument->type(), id, argument->qualifier(), (bool)argument->initializer(), argument->is_typed(cdk::TYPE_FUNCTIONAL));
      _symtab.insert(id, symbol);
    }
  }

  if (!input_type.empty()) node->type(cdk::functional_type::create(input_type, output_type));
  else node->type(cdk::functional_type::create(output_type));

  _functionStack.push(node);
  _functionType.push(node->type());
  node->block()->accept(this, lvl + 2);
  _functionType.pop();
  _functionStack.pop();
  _symtab.pop();
}

void mml::type_checker::do_block_node(mml::block_node * const node, int lvl) {
  if (node->declarations()) node->declarations()->accept(this, lvl + 2);
  if (node->instructions()) node->instructions()->accept(this, lvl + 2);
}

void mml::type_checker::do_addressof_node(mml::addressof_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->left_value()->accept(this, lvl + 2);
  if (node->left_value()->is_typed(cdk::TYPE_UNSPEC) || node->left_value()->is_typed(cdk::TYPE_VOID))
    throw std::string("Wrong type in unary logical expression.");
  node->type(cdk::reference_type::create(4, node->left_value()->type()));
}

void mml::type_checker::do_index_pointer_node(mml::index_pointer_node * const node, int lvl) {
  ASSERT_UNSPEC

  if (node->base()) {

    node->base()->accept(this, lvl + 2);
    if (!node->base()->is_typed(cdk::TYPE_POINTER))
      throw std::string("pointer expresssion expected as base");

  } else throw std::string("pointer expresssion expected as base");

  node->index()->accept(this, lvl + 2);
  if (!node->index()->is_typed(cdk::TYPE_INT)) throw std::string("integer expresssion expected as index");
  node->type(cdk::reference_type::cast(node->base()->type())->referenced());
}

void mml::type_checker::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0,cdk::TYPE_UNSPEC)));
}

void mml::type_checker::do_variable_declaration_node(mml::variable_declaration_node * const node, int lvl) {
  if (node->initializer()) {
    node->initializer()->accept(this, lvl + 2);
    
    if (_symtab.find(node->identifier()) && node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)) // Forward calls
      node->type(_symtab.find(node->identifier())->type());

    // Type auto
    if (!node->type() || node->type()->name() == cdk::TYPE_UNSPEC) node->type(node->initializer()->type());
    else { // Check for type congruency between initializer and variable type
      if (node->is_typed(cdk::TYPE_INT)) {

        if (!node->initializer()->is_typed(cdk::TYPE_INT))
          throw std::string("Wrong initializer type: integer expected.");

      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {

        if (!node->initializer()->is_typed(cdk::TYPE_DOUBLE) &&!node->initializer()->is_typed(cdk::TYPE_INT))
          throw std::string("Wrong initializer type: integer or double expected.");

      } else if (node->is_typed(cdk::TYPE_STRING)) {

        if (!node->initializer()->is_typed(cdk::TYPE_STRING))
          throw std::string("Wrong initializer type: string expected.");

      } else if (node->is_typed(cdk::TYPE_VOID)) {

        if (!node->initializer()->is_typed(cdk::TYPE_VOID))
          throw std::string("Wrong initializer type: void expected.");

      } else if (node->is_typed(cdk::TYPE_POINTER)) {

        if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) {  // Stack Alloc

          if (dynamic_cast<mml::stack_alloc_node *>(node->initializer()) != nullptr)
            node->initializer()->type(node->type());
          else throw std::string("Wrong initializer type: pointer expected.");

        } else if (!node->initializer()->is_typed(cdk::TYPE_POINTER)) {

          throw std::string("Wrong initializer type: pointer expected.");

        } else if (cdk::reference_type::cast(node->initializer()->type())->referenced()->name() != cdk::TYPE_UNSPEC)
          processPointerType(cdk::reference_type::cast(node->type()),
                             cdk::reference_type::cast(node->initializer()->type()));

      } else if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {

        if (node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL))
          processFunctionalType(cdk::functional_type::cast(node->type()), cdk::functional_type::cast(node->initializer()->type()));
        else throw std::string("Wrong initializer type: functional type expected.");

      } else throw std::string("Unknown type for initializer.");
    }
  } else if (!node->type() || node->type()->name() == cdk::TYPE_UNSPEC)
    throw std::string("variable declaration requires a type");

  // Check if variable identifier is unique
  const std::string &id = node->identifier();
  auto symbol = mml::make_symbol(node->type(), id, node->qualifier(), (bool)node->initializer(), node->is_typed(cdk::TYPE_FUNCTIONAL));

  if (node->is_typed(cdk::TYPE_FUNCTIONAL) && node->initializer() && node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)) 
    symbol->set_expression_type(node->initializer()->type());

  if (!_symtab.insert(id, symbol)) {
    if ((_symtab.find(id)->type() != node->type() || _symtab.find(id)->qualifier() != node->qualifier()) &&
        !(node->initializer() && node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)))
      throw std::string("'" + id + "' has already been declared");
  }

  _parent->set_new_symbol(symbol);
}

void mml::type_checker::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) { // check if read_node received as argument

    if (dynamic_cast<mml::read_node *>(node->argument()) != nullptr)
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else throw std::string("argument with unspecified type where integer was expected");

  } else if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("integer expression expected in allocation expression");

  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

void mml::type_checker::do_identity_node(mml::identity_node * const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::processBinaryExpression(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT)) { // integer to operate

    if (node->right()->is_typed(cdk::TYPE_INT))
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (node->right()->is_typed(cdk::TYPE_DOUBLE))
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (node->right()->is_typed(cdk::TYPE_POINTER))
      node->type(node->right()->type());
    else throw std::string("Incompatible data types in binary expression.");

  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) { // double to operate

    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else throw std::string("Incompatible data types in binary expression.");

  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) { // pointer to operate

    if (node->right()->is_typed(cdk::TYPE_INT))
      node->type(node->left()->type());
    else if (node->right()->is_typed(cdk::TYPE_POINTER)) {

      if (typeid(node) == typeid(cdk::sub_node)) node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      else throw std::string("two pointers may not be operated upon except through subtraction");

    } else throw std::string("Incompatible data types in binary expression.");

  } else throw std::string("Wrong types in operation expression");
}

void mml::type_checker::processIntegerOnlyExpression(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in left binary operator.");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in left binary operator.");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::processCompareExpression(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Integer or double expression expected in left binary operator");

  node->right()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Integer or double expression expected in right binary operator");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::processEqualityExpression(cdk::binary_operation_node * const node, int lvl) {
  ASSERT_UNSPEC
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_UNSPEC)) { // check if read_node or input_node received as left arg

    if (dynamic_cast<mml::read_node *>(node->left()) != nullptr)
      node->left()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (dynamic_cast<mml::stack_alloc_node *>(node->left()) != nullptr)
      throw std::string("operation with unassigned stack allocation not allowed");
    else throw std::string("operation with unspecified typed argument");
      
  }

  if (node->right()->is_typed(cdk::TYPE_UNSPEC)) { // check if read_node or input_node received as right arg

    if (dynamic_cast<mml::read_node *>(node->left()) != nullptr)
      node->right()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    else if (dynamic_cast<mml::stack_alloc_node *>(node->left()) != nullptr)
      throw std::string("operation with unassigned stack allocation not allowed");
    else throw std::string("operation with unspecified typed argument");

  }

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    if ((cdk::reference_type::cast(node->left()->type())->referenced()->name() != cdk::TYPE_UNSPEC) &&
       (cdk::reference_type::cast(node->right()->type())->referenced()->name() != cdk::TYPE_UNSPEC))
      processPointerType(cdk::reference_type::cast(node->left()->type()),
                         cdk::reference_type::cast(node->right()->type()));
  } else if (node->left()->type()->name() != node->right()->type()->name() &&
            !(node->left()->is_typed(cdk::TYPE_INT)) && !(node->right()->is_typed(cdk::TYPE_DOUBLE)) &&
            !(node->left()->is_typed(cdk::TYPE_INT)) && !(node->right()->is_typed(cdk::TYPE_DOUBLE)))
    throw std::string("operation with incompatible types");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::processPointerType(const std::shared_ptr<cdk::reference_type>& left, const std::shared_ptr<cdk::reference_type>& right) {
  int left_depth = 0, right_depth = 0;

  std::shared_ptr<cdk::basic_type> left_type = left;
  std::shared_ptr<cdk::basic_type> right_type = right;

  while (left_type->name() == cdk::TYPE_POINTER) {
    left_depth++;
    left_type = cdk::reference_type::cast(left_type)->referenced();
  }

  while (right_type->name() == cdk::TYPE_POINTER) {
    right_depth++;
    right_type = cdk::reference_type::cast(right_type)->referenced();
  }

  if (left_depth != right_depth) throw std::string("pointer type mismatch (depth)");

  if (!((left_type->name() == cdk::TYPE_INT && right_type->name() == cdk::TYPE_INT) ||
      (left_type->name() == cdk::TYPE_DOUBLE && right_type->name() == cdk::TYPE_DOUBLE) ||
      (left_type->name() == cdk::TYPE_STRING && right_type->name() == cdk::TYPE_STRING) ||
      (left_type->name() == cdk::TYPE_VOID && right_type->name() == cdk::TYPE_VOID)))
    throw std::string("pointer type mismatch (kernel)");
}

void mml::type_checker::processFunctionalType(const std::shared_ptr<cdk::functional_type>& left, const std::shared_ptr<cdk::functional_type>& right) {
  if (left->input_length() != right->input_length()) throw std::string("Input length mismatch");
  if (left->output_length() != right->output_length()) throw std::string("Output length mismatch (1 or 0 expected)");

  for (size_t i = 0; i < left->input_length(); i++) 
    if (left->input(i) != right->input(i)) {
      if (!((left->input(i)->name() == cdk::TYPE_INT && right->input(i)->name() == cdk::TYPE_DOUBLE) ||
           (left->input(i)->name() == cdk::TYPE_DOUBLE && right->input(i)->name() == cdk::TYPE_INT)))
        throw std::string("Functional types differ at input n. " + std::to_string(i));
  } 
  
  if (left->output(0) != right->output(0) && left->output(0)->name() != cdk::TYPE_DOUBLE && right->output(0)->name() != cdk::TYPE_INT) 
    throw std::string("Functional types differ at output");
}