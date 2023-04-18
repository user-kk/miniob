#pragma once

#include "sql/expr/tuple.h"
#include "sql/operator/operator.h"
#include "rc.h"
#include "sql/stmt/select_stmt.h"
#include <vector>

class OrderOperator : public Operator {
public:
  OrderOperator()
  {}

  virtual ~OrderOperator() = default;

  virtual RC open() override;
  virtual RC next() override;
  virtual RC close() override;

  virtual Tuple *current_tuple() override;
  void add_seq(SelectStmt::Seq seq);

private:
  RC sort();
  using Seq = SelectStmt::Seq;

private:
  std::vector<Tuple *> tuples_;
  std::vector<SelectStmt::Seq> seqs_;
  size_t current_tuple_index_ = 0;
  bool isReady_ = false;
};