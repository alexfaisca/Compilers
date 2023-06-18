#ifndef __MML_TARGETS_SYMBOL_H__
#define __MML_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <cdk/types/basic_type.h>

namespace mml {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::shared_ptr<cdk::basic_type> _expressionType;
    std::string _name;
    std::string _lbl;
    std::string _segment;
    //long _value; // hack!
    int _qualifier;
    bool _set;
    bool _function;
    size_t _offset;
    bool _global;
    bool _forward;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, int qualifier, bool set, bool function) :
       _type(type), _expressionType(type), _name(name), _lbl(name), _segment(name), _qualifier(qualifier), _set(set), _function(function), _offset(0), _global(false), _forward(false) {
    }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    void set_expression_type(std::shared_ptr<cdk::basic_type> type) {
      _expressionType = type;
    }
    std::shared_ptr<cdk::basic_type> get_expression_type() const {
      return _expressionType;
    }
    const std::string &name() const {
      return _name;
    }
    /*long value() const {
      return _value;
    }
    long value(long v) {
      return _value = v;
    }*/
    int qualifier() const {
      return _qualifier;
    }
    bool set() const {
      return _set;
    }
    bool function() const {
      return _function;
    }
    size_t offset() const {
      return _offset;
    }
    void set_offset(size_t offset) {
      _offset = offset;
    }
    bool global() const {
      return _global;
    }
    void set_global() {
      _global = true;
    }
    void set_label(std::string &label) {
      _lbl = label;
    }
    std::string &get_label() {
      return _lbl;
    }
    void set_segment(std::string &segment) {
      _segment = segment;
    }
    std::string &get_segment() {
      return _segment;
    }
    void set_forward() {
      _forward = true;
    }
    bool is_forward() {
      return _forward;
    }
  };

  inline auto make_symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, int qualifier, bool set, bool function) {
    return std::make_shared<symbol>(type, name, qualifier, set, function);
  }

} // mml

#endif
