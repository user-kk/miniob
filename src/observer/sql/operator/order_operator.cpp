#include "sql/operator/order_operator.h"
#include "sql/expr/tuple.h"
#include <algorithm>
#include <cassert>

RC OrderOperator::open()
{
  if (children_.size() != 1) {
    LOG_WARN("project operator must has 1 child");
    return RC::INTERNAL;
  }

  Operator *child = children_[0];
  RC rc = child->open();
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
};
RC OrderOperator::next()
{
  if (isReady_) {
    current_tuple_index_++;
    if (current_tuple_index_ >= tuples_.size()) {
      return RC::RECORD_EOF;
    }
    return RC::SUCCESS;
  }

  RC rc = RC::SUCCESS;
  Operator *oper = children_[0];

  while ((rc = oper->next()) == RC::SUCCESS) {
    // Tuple *tuple = oper->current_tuple();
    ProjectTuple *current_tuple = dynamic_cast<ProjectTuple *>(oper->current_tuple());

    if (nullptr == current_tuple) {
      rc = RC::INTERNAL;
      LOG_WARN("failed to get current record. rc=%s", strrc(rc));
      break;
    }
    Tuple *tuple = current_tuple->clone();
    tuples_.push_back(tuple);
  }
  if (rc == RC::RECORD_EOF) {
    if (!seqs_.empty()) {
      rc = sort();
      if (rc == RC::SUCCESS) {
        isReady_ = true;
        return RC::SUCCESS;
      }
      return rc;
    }
    isReady_ = true;
    return RC::SUCCESS;
  }
  return rc;
};
RC OrderOperator::close()
{
  children_[0]->close();
  tuples_.clear();
  current_tuple_index_ = 0;
  for (auto &tuple : tuples_) {
    auto p = dynamic_cast<ProjectTuple *>(tuple);
    assert(p != nullptr);  // tuple必须是ProjectTuple *类型
    ProjectTuple::destory_cloned_tuple(p);
  }

  return RC::SUCCESS;
};
Tuple *OrderOperator::current_tuple()
{
  assert(current_tuple_index_ < tuples_.size());
  return tuples_[current_tuple_index_];
};
RC OrderOperator::sort()
{
  auto f = [this](Tuple *a, Tuple *b) -> bool {
    for (size_t i = 0; i < seqs_.size(); ++i) {
      TupleCell cell_a;
      RC rc1 = a->cell_at(i, cell_a);
      TupleCell cell_b;
      RC rc2 = b->cell_at(i, cell_b);
      assert(rc1 == RC::SUCCESS && rc2 == RC::SUCCESS);
      int result = cell_a.compare(cell_b);
      if (result == 0) {
        continue;
      }
      return (seqs_[i] == Seq::ASC) ? (result < 0) : (result > 0);
    }
    return true;  // 只有所有项全都相等才会到达这个位置
  };
  std::sort(tuples_.begin(), tuples_.end(), f);
  return RC::SUCCESS;
};
void OrderOperator::add_seq(SelectStmt::Seq seq)
{
  seqs_.push_back(seq);
};