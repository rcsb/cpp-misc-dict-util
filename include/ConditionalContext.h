
// Nothing at the moment

/*!
** \file ConditionalContext.h
**
** \brief Header file for ConditionalContext class.
*/

#ifndef CONDITIONAL_CONTEXT_H
#define CONDITIONAL_CONTEXT_H

#include "ISTable.h"
#include "TableFile.h"
#include "DicFile.h"
#include "CifDataInfo.h"

#include <string>

// Enumerations for a conditional Item context
enum ConditionalContextItemAction {
  eSuppressNone, // No action
  eSuppressRow,
  eSuppressValue,
  eSuppressItem,
  eSuppressUnknown // Unknown action name
};

class ConditionalContext
{
 public:
  // Constructor
  ConditionalContext(Block& inBlock, DicFile *dictFileP);

  // Returns true if category tableName has a conditional context
  bool HaveConditionalTableContext(const string& tableName);

  // Returns true is category tableName should be skipped
  bool SkipTable(const string& tableName);

  // Returns true if there is a conditional item context for itemName
  bool HaveConditionalItemContext(const string& itemName);

  // Returns the ConditionalContextItemAction enum for a particular row
  ConditionalContextItemAction GetConditionalItemContext(const string& itemName, unsigned int row);

  // Returns true if itemName column can be skipped in its entirety
  bool SkipItem(const string& itemName);

  // Returns if an item is mandator
  bool IsItemMandatory(const string& itemName);
  
 private:
  Block& _inBlock; // Input file
  ISTable* _catConditionalContext;
  ISTable* _itemConditionalContext;
  DicFile* _dictFileP;
  ISTable* pdbxCatConditionalContext;
  ISTable* pdbxItemConditionalContext;
  ISTable* pdbxConditionalContextList;  
  CifDataInfo cDataInfo;

  // Evaluate a list of conditions
  bool _evalConditionalList(const std::string &context_id,
			    bool catContext, const std::string &catName);
  bool _evalConditionalList(const std::string &context_id,
			    bool catContext, const std::string &catName, const std::string &item, int row=0);

  // Evaluate a single condition
  bool _evalConditional(const std::string &target_item_name, const std::string &target_item_value,
			const std::string &cmp_op, bool catContext, const std::string &catName, const std::string &item, int row=0);

  // Returns the row tableName is in pdbx_category_conditional_context.
  unsigned int _getConditionalTableRow(const string& tableName);

  // Returns the row itemName is in pdbx_item_conditional_context.
  unsigned int _getConditionalItemRow(const string& itemName);  
};

#endif
