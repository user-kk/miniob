#pragma once
#include "sql/operator/operator.h"
#include "sql/stmt/update_stmt.h"

class UpdateOperator : public Operator {
public:
  UpdateOperator(UpdateStmt *update_stmt, Trx *trx) : update_stmt_(update_stmt), trx_(trx)
  {}
  virtual ~UpdateOperator() = default;
  virtual RC open() override;
  virtual RC next() override;
  virtual RC close() override;
  Tuple *current_tuple() override
  {
    return nullptr;
  }

private:
  UpdateStmt *update_stmt_ = nullptr;
  Trx *trx_ = nullptr;
};