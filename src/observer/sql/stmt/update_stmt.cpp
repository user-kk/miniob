/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "rc.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/common/db.h"
#include "storage/common/field_meta.h"
#include <storage/common/table.h>
#include <string>
#include <unordered_map>

UpdateStmt::UpdateStmt(Table *table, char *attr_name, const Value *values, int value_amount, FilterStmt *filter_stmt)
    : table_(table), values_(values), attr_name_(attr_name), value_amount_(value_amount), filter_stmt_(filter_stmt)
{}

RC UpdateStmt::create(
    Db *db, const Updates &update, Stmt *&stmt)  // stmt中放入tabel和要update的field ,还要放入过滤条件(filter_)
{
  const char *table_name = update.relation_name;
  if (db == nullptr || table_name == nullptr) {
    LOG_WARN("invalid argument db=%p table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  Table *table = db->find_table(table_name);
  if (table == nullptr) {
    LOG_WARN(" no such table db=%s table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  char *field_name = update.attribute_name;
  const FieldMeta *field_meta = table->table_meta().field(field_name);
  if (nullptr == field_meta) {
    LOG_WARN("no such field db=%s table=%s field=%s", db->name(), table->name(), field_name);
    return RC::SCHEMA_FIELD_MISSING;
  }

  // 构造传入Filter::create的参数
  std::unordered_map<std::string, Table *> table_map{{std::string(table_name), table}};

  FilterStmt *filter_stmt = nullptr;

  RC rc = FilterStmt::create(db, table, &table_map, update.conditions, update.condition_num, filter_stmt);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed create filter statement rc=%d:%s", rc, strrc(rc));
    return rc;
  }
  stmt = new UpdateStmt(table, field_name, &update.value, 1, filter_stmt);
  return RC::SUCCESS;
}
